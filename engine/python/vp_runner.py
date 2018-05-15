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
 
import vp.time_domain
import vp.trace_engine
import vp_core
import plptree

from plp_platform import *


class Runner(Platform):

    def __init__(self, config):

        self.config = config
        super(Runner, self).__init__(self.config)

        systemConfig = self.config.args.config
        if systemConfig is not None:
            self.system_tree = plptree.get_configs_from_env(
                self.config.args.configDef, systemConfig)
        else:
            self.system_tree = plptree.get_configs_from_file(
                self.config.args.configFile)[0]



    def prepare(self):
        return 0

    def run(self):


        system = self.system_tree
        if self.system_tree.get_config('system') is not None:
          system = self.system_tree.get_config('system_tree')

        os.environ['PULP_CONFIG_FILE'] = self.config.getOption('configFile')


        top = system.get('vp_class')

        if top is None:
            raise Exception("The specified configuration does not contain any"
                            " top component")

        config = self.system_tree.get_config('gvsoc')
        debug_mode = len(config.get('trace')) != 0 or len(config.get('event')) != 0

        trace_engine = vp.trace_engine.component(name=None, config=config, debug=debug_mode)

        time_engine = trace_engine.new(
            name=None,
            component='vp.time_domain',
            config=system
        )

        top_comp = time_engine.new(
            name='sys',
            component=top,
            config=system
        )

        trace_engine.get_port('out').bind_to(top_comp.get_port('trace'))

        trace_engine.bind()

        trace_engine.post_post_build_all()

        trace_engine.pre_start_all()

        trace_engine.start_all()

        trace_engine.post_start_all()

        trace_engine.reset_all()

        trace_engine.load_all()

        status = time_engine.run()

        trace_engine.stop_all()

        if status == 'killed':
          print ('The top engine was not responding and was killed')
          return -1
        elif status == 'error':
          return -1
        else:
          return time_engine.run_status()


        return 0
