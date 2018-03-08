#
# Copyright (C) 2018 ETH Zurich and University of Bologna
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

# Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 
import vp_core as vp
import re
import ctypes

class component(vp.component):

    implementation = 'vp.trace_domain_impl'

    max_path_len = 32

    def reg_path(self, path):
        length = len(path)

        if length > self.max_path_len:
            self.max_path_len = length

    def dump_msg(self, path, str):
        if path == '':
            path = '/'
        print ('0: 0: [' + vp.bcolors.OKBLUE + path.ljust(self.max_path_len, ' ') + vp.bcolors.ENDC + '] ' + str)

    def build(self):
        self.new_service('trace')
        self.new_port('out')
        self.traces = []

        config = self.get_config()
        if config is not None:
          traces = config.get('trace')

          for trace in traces:
            self.traces.append(re.compile(trace))

          implem_traces = (ctypes.c_char_p * len(traces))()
          for i in range(0, len(traces)):
            implem_traces[i] = traces[i].encode('utf-8')
          self.impl.module.vp_trace_add_paths(self.impl.instance, len(traces), implem_traces)

    def pre_start(self):
        length = self.impl.module.vp_trace_exchange_max_path_len(self.impl.instance, self.max_path_len)
        if length > self.max_path_len:
            self.max_path_len = length

    def is_active(self, path):
      for trace in self.traces:
        if trace.search(path):
          return True
      return False
