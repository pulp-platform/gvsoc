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


class component(vp.component):

    def build(self):

        comps = {}

        for comp_name in self.get_config().get('vp_comps'):            
            config = self.get_config().get_config(comp_name)
            comps[comp_name] = self.new(comp_name, component=config.get('vp_class'), config=config)

        bindings = self.get_config().get('vp_bindings')
        if bindings is not None:
            for binding in bindings:
                master_comp, master_port = binding[0].split('->')
                slave_comp, slave_port = binding[1].split('->')
                comps[master_comp].get_port(master_port).bind_to(comps[slave_comp].get_port(slave_port))

