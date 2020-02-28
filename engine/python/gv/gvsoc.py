#
# Copyright (C) 2019 GreenWaves Technologies
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

import runner.default_runner
import os
import argparse
import json_tools as js


def appendArgs(parser: argparse.ArgumentParser, runnerConfig: js.config) -> None:
    """
    Append specific runner arguments.
    """
    
    parser.add_argument("--trace",
                        dest="traces",
                        default=[],
                        action="append",
                        help="Specify gvsoc trace")




class Runner(runner.default_runner.Runner):

    def __init__(self, args, config):
        super(Runner, self).__init__(args, config)
        self.__process_args()


    def exec(self):

        full_config =  js.import_config(self.config.get_dict(), interpret=True, gen=True)

        gvsoc_config = full_config.get('gvsoc')

        verbose = gvsoc_config.get_bool('verbose')

        debug_mode = gvsoc_config.get_bool('debug-mode') or \
            gvsoc_config.get_bool('traces/enabled') or \
            gvsoc_config.get_bool('events/enabled') or \
            len(gvsoc_config.get('traces/include_regex')) != 0 or \
            len(gvsoc_config.get('events/include_regex')) != 0

        gvsoc_config.set("debug-mode", debug_mode)

        if debug_mode:
            launcher = gvsoc_config.get_str('launchers/debug')
        else:
            launcher = gvsoc_config.get_str('launchers/default')

        gvsoc_config_path = os.path.join(os.getcwd(), 'gvsoc_config.json')

        with open(gvsoc_config_path, 'w') as file:
            file.write(full_config.dump_to_string())

        os.environ['PULP_CONFIG_FILE'] = gvsoc_config_path

        command = [launcher, '--config=' + gvsoc_config_path]

        if verbose:
            print ('Launching GVSOC with command: ')
            print (' '.join(command))

        return os.execvp(launcher, command)


    def __process_args(self):
        for trace in self.args.traces:
            self.config.set('gvsoc/traces/include_regex', trace)

