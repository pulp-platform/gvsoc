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

    ...

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

    def quit(self):
        self.telnet.write('quit\n'.encode('ascii'))


    def get_retval(self):
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


    def get_component(self, path):
            self.telnet.write(('get_component %s\n' % path).encode('ascii'))
            result = self.telnet.read_until(b"\n")
            return result.decode('ascii').replace('\n', '')



class Testbench_i2s(object):

    def __init__(self, proxy, testbench, id=0):
        self.id = id
        self.proxy = proxy
        self.testbench = testbench

    def open(self, word_size=16, sampling_freq=-1, nb_slots=1, is_pdm=False,
            is_full_duplex=False, is_ext_clk=False,is_ext_ws=False, is_sai0_clk=False,
            is_sai0_ws=False, clk_polarity=0, ws_polarity=0):
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
        return self.proxy.get_retval()

    def close(self):
        options = ''
        options += ' itf=%d' % self.id
        options += ' enabled=0'
        cmd = 'component %s i2s setup %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy.get_retval()

    def clk_start(self):
        cmd = 'component %s i2s clk_start %d\n' % (self.testbench, self.id)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy.get_retval()

    def clk_stop(self):
        cmd = 'component %s i2s clk_stop %d\n' % (self.testbench, self.id)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy.get_retval()

    def slot_open(self, slot=0, is_rx=True, word_size=16, is_msb=True, sign_extend=False, left_align=False):
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' is_rx=%d' % is_rx
        options += ' enabled=1'
        options += ' word_size=%d' % word_size
        options += ' format=%d' % (is_msb | (left_align << 1) | (sign_extend << 1))
        cmd = 'component %s i2s slot_setup %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy.get_retval()

    def slot_close(self, slot=0):
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' enabled=0'
        cmd = 'component %s i2s slot_setup %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy.get_retval()

    def slot_rx_file_reader(self, slot=0, filetype="wav", filepath=None):
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' filetype=%s' % filetype
        options += ' filepath=%s' % filepath
        cmd = 'component %s i2s slot_rx_file_reader %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy.get_retval()

    def slot_tx_file_dumper(self, slot=0, filetype="wav", filepath=None):
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' filetype=%s' % filetype
        options += ' filepath=%s' % filepath
        cmd = 'component %s i2s slot_tx_file_dumper %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy.get_retval()

    def slot_stop(self, slot=0, stop_rx=True, stop_tx=True):
        options = ''
        options += ' itf=%d' % self.id
        options += ' slot=%d' % slot
        options += ' stop_rx=%d' % stop_rx
        options += ' stop_tx=%d' % stop_tx
        cmd = 'component %s i2s slot_stop %s\n' % (self.testbench, options)
        self.proxy.telnet.write(cmd.encode('ascii'))
        return self.proxy.get_retval()





class Testbench(object):

    def __init__(self, proxy):
        self.proxy = proxy
        self.component = proxy.get_component('/sys/board/testbench/testbench')


    def i2s_get(self, id=0):
        return Testbench_i2s(self.proxy, self.component, id)

    