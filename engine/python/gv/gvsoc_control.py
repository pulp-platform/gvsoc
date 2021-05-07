#
# Copyright (C) 2021 GreenWaves Technologies
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from telnetlib import Telnet

class Telnet_proxy(object):
    """
    A class used to control GVSOC through the telnet proxy

    Attributes
    ----------
    host : str
        a string giving the hostname where the proxy is running
    port : int
        the port where to connect
    """

    def __init__(self, host: str = 'localhost', port: int = 42951):
        self.telnet = Telnet(host, port)

    def trace_add(self, trace: str):
        """Enable a trace.

        Parameters
        ----------
        trace : str
            A regular expression used to enable traces
        """

        self.telnet.write(('trace add %s\n' % trace).encode('ascii'))

    def trace_remove(self, trace: str):
        """Disable a trace.

        Parameters
        ----------
        trace : str
            A regular expression used to disable traces
        """

        self.telnet.write(('trace remove %s\n' % trace).encode('ascii'))

    def event_add(self, event: str):
        """Enable an event.

        Parameters
        ----------
        event : str
            A regular expression used to enable events
        """

        self.telnet.write(('event add %s\n' % event).encode('ascii'))

    def event_remove(self, event: str):
        """Disable a trace.

        Parameters
        ----------
        event : str
            A regular expression used to enable events
        """

        self.telnet.write(('event remove %s\n' % event).encode('ascii'))

    def run(self, duration: int = None):
        """Starts execution.

        Parameters
        ----------
        duration : int, optional
            Specify the duration of the execution in picoseconds (will execute forever by default)
        """

        if duration is not None:
            self.telnet.write(('step %d\n' % duration).encode('ascii'))
        else:
            self.telnet.write('run\n'.encode('ascii'))

        self.telnet.read_until(b"\n")

    def quit(self, status=0):
        """Exit simulation.

        Parameters
        ----------
        duration : int, optional
            Specify the status value.
        """

        self.telnet.write(('quit %d\n' % status).encode('ascii'))


    def _get_retval(self):
        result = self.telnet.read_until(b"\n").decode('ascii')
        error = 0
        error_str = None
        for arg in result.split(';'):
            name, value = arg.split('=')
            if name == 'err':
                error = int(value)
            elif name == 'msg':
                error_str = value

        if error != 0:
            if error_str is None:
                raise RuntimeError("Proxy command failed with status %s" % error)
            else:
                raise RuntimeError("Proxy command failed with message: %s" % error_str)


    def _get_component(self, path):
            self.telnet.write(('get_component %s\n' % path).encode('ascii'))
            result = self.telnet.read_until(b"\n")
            return result.decode('ascii').replace('\n', '')



class Testbench(object):
    """Testbench class.

    This class can be instantiated to get access to the testbench.

    Attributes
    ----------
    proxy : int, optional
        The proxy object. This class will use it to send command to GVSOC through the proxy connection.
    path : int, optional
        The path to the testbench in the architecture.
    """

    def __init__(self, proxy, path='/sys/board/testbench/testbench'):
        self.proxy = proxy
        self.component = proxy._get_component(path)


    def i2s_get(self, id=0):
        """Open an SAI.

        Opena an SAI and return and object which can be used to interact with it.

        Parameters
        ----------
        id : int, optional
            The SAI identifier.
        """
        return Testbench_i2s(self.proxy, self.component, id)



class Testbench_i2s(object):
    """Class instantiated for each manipulated SAI.

    It can used to interact with the SAI, like injecting streams.

    Attributes
    ----------
        proxy : int, optional
            The proxy object. This class will use it to send command to GVSOC through the proxy connection.
        testbench : int, optional
            The testbench object.
        id : int, optional
            The identifier of the SAI interface.
    """

    def __init__(self, proxy, testbench, id=0):
        self.id = id
        self.proxy = proxy
        self.testbench = testbench

    def open(self, word_size=16, sampling_freq=-1, nb_slots=1, is_pdm=False,
            is_full_duplex=False, is_ext_clk=False, is_ext_ws=False, is_sai0_clk=False,
            is_sai0_ws=False, clk_polarity=0, ws_polarity=0):
        """Open and configure SAI.

        Parameters
        ----------
        word_size : int, optional
            Specify the frame word size in bits.
        sampling_freq : int, optional
            Specify the sampling frequency. This is used either to generate the clock when
            it is external or to check that internally generated one is correct.
        nb_slots : int, optional
            Number of slots in the frame.
        is_pdm : bool, optional
            True if the stream is a PDM stream.
        is_full_duplex : bool, optional
            True if the SAI is used in full duplex mode.
        is_ext_clk: bool, optional
            True is the clock is generated by the testbench.
        is_ext_ws: bool, optional
            True is the word strobe is generated by the testbench.
        is_sai0_clk: bool, optional
            True is the the clock should be taken from SAI0.
        is_sai0_ws: bool, optional
            True is the the word strobe should be taken from SAI0.
        clk_polarity : int, optional
            Clock polarity, definition is the same as SAI0 specifications.
        ws_polarity : int, optional
            Word strobe polarity, definition is the same as SAI0 specifications.
        """

        options = ''
        options += ' itf=%d' % self.id
        options += ' enabled=1'
        options += ' sampling_freq=%d' % sampling_freq
        options += ' word_size=%d' % word_size
        options += ' nb_slots=%d' % nb_slots
        options += ' is_pdm=%d' % is_pdm
        options += ' is_full_duplex=%d' % is_full_duplex
        options += ' is_ext_clk=%d' % is_ext_clk
        options += ' is_ext_ws=%d' % is_ext_ws
        options += ' is_sai0_clk=%d' % is_sai0_clk
        options += ' is_sai0_ws=%d' % is_sai0_ws
        options += ' clk_polarity=%d' % clk_polarity
        options += ' ws_polarity=%d' % ws_polarity
        cmd = 'component %s i2s setup %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()

    def close(self):
        """Close SAI.
        """
        options = ''
        options += ' itf=%d' % self.id
        options += ' enabled=0'
        cmd = 'component %s i2s setup %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()

    def clk_start(self):
        """Start clock.

        This can be used when the clock is generated by the testbench to start the generation.
        """
        cmd = 'component %s i2s clk_start %d\n' % (self.testbench, self.id)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()

    def clk_stop(self):
        """Stop clock.

        This can be used when the clock is generated by the testbench to stop the generation.
        """
        cmd = 'component %s i2s clk_stop %d\n' % (self.testbench, self.id)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()

    def slot_open(self, slot=0, is_rx=True, word_size=16, is_msb=True, sign_extend=False, left_align=False):
        """Open and configure a slot.

        Parameters
        ----------
        slot : int, optional
            Slot identifier
        is_rx : bool, optional
            True if gap receives the samples.
        word_size : int, optional
            Slot width in number of bits.
        is_msb : bool, optional
            True if the samples are received or sent with MSB first.
        sign_extend : bool, optional
            True if the samples are sign-extended.
        left_align : bool, optional
            True if the samples are left aligned.
        """
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' is_rx=%d' % is_rx
        options += ' enabled=1'
        options += ' word_size=%d' % word_size
        options += ' format=%d' % (is_msb | (left_align << 1) | (sign_extend << 1))
        cmd = 'component %s i2s slot_setup %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()

    def slot_close(self, slot=0):
        """Close a slot.

        Parameters
        ----------
        slot : int, optional
            Slot identifier
        """
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' enabled=0'
        cmd = 'component %s i2s slot_setup %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()

    def slot_rx_file_reader(self, slot=0, filetype="wav", filepath=None):
        """Read a stream of samples from a file.

        This will open a file and stream it to the SAI so that gap receives the samples

        Parameters
        ----------
        slot : int, optional
            Slot identifier
        filetype : string, optional
            Describes the type of the file, can be "wav" or "au".
        filepath : string, optional
            Path to the file.
        """
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' filetype=%s' % filetype
        options += ' filepath=%s' % filepath
        cmd = 'component %s i2s slot_rx_file_reader %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()

    def slot_tx_file_dumper(self, slot=0, filetype="wav", filepath=None):
        """Write a stream of samples to a file.

        This will open a file and write to it all the samples received from gap

        Parameters
        ----------
        slot : int, optional
            Slot identifier
        filetype : string, optional
            Describes the type of the file, can be "wav" or "au".
        filepath : string, optional
            Path to the file.
        """
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' filetype=%s' % filetype
        options += ' filepath=%s' % filepath
        cmd = 'component %s i2s slot_tx_file_dumper %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()

    def slot_stop(self, slot=0, stop_rx=True, stop_tx=True):
        """Stop a slot.

        This will stop the streamings (file reader or dumper) configured on the specified slot.

        Parameters
        ----------
        slot : int, optional
            Slot identifier
        stop_rx : bool, optional
            Stop the stream sent to gap.
        stop_tx : bool, optional
            Stop the stream received from gap.
        """
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' stop_rx=%d' % stop_rx
        options += ' stop_tx=%d' % stop_tx
        cmd = 'component %s i2s slot_stop %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy._get_retval()
