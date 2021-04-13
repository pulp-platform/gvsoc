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
