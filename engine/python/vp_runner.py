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
import vp.power_engine
import vp_core
import runner.stim_utils

from plp_platform import *
import plp_flash_stimuli
import shlex


class Runner(Platform):

    def __init__(self, config, js_config):

        super(Runner, self).__init__(config, js_config)

        parser = config.getParser()

        [args, otherArgs] = parser.parse_known_args()

        self.addCommand('run', 'Run execution on GVSOC')
        self.addCommand('prepare', 'Prepare binary for GVOSC')

        self.__prepare_env()


    def __prepare_env(self):

        self.gen_flash_stimuli = False
        self.gen_rom_stimuli = False

        comps_conf = self.get_json().get('**/fs/files')

        if comps_conf is not None or self.get_json().get_child_bool('**/runner/boot_from_flash'):

            self.gen_flash_stimuli = True

        if self.get_json().get('**/rom') != None:

            if self.get_json().get('**/soc/rom/stim_file') is not None:
                try:
                    rom_file = eval(self.get_json().get_str('**/soc/rom/stim_file'))
                except:
                    rom_file = self.get_json().get_str('**/soc/rom/stim_file')

                self.get_json().get('**/soc/rom').set('stim_file', rom_file)

            else:

                self.boot_binary = os.path.join(os.environ.get('PULP_SDK_INSTALL'), 'bin', 'boot-%s' % self.get_json().get('**/chip/name').get())

                if os.path.exists(self.boot_binary):
                    self.gen_rom_stimuli = True




    def get_flash_preload_file(self):
        return os.path.join(os.getcwd(), 'stimuli/flash.bin')


    def prepare(self):
        comps = []
        comps_conf = self.get_json().get('**/flash/fs/files')
        if comps_conf is not None:
            comps = comps_conf.get_dict()

        if self.gen_rom_stimuli:
            stim = runner.stim_utils.stim(verbose=self.get_json().get_child_bool('**/runner/verbose'))
            stim.add_binary(self.boot_binary)
            stim.add_area(self.get_json().get_child_int('**/rom/base'), self.get_json().get_child_int('**/rom/size'))
            stim.gen_stim_bin('stimuli/rom.bin')


        if self.gen_flash_stimuli:

            encrypted = self.get_json().get_child_str('**/efuse/encrypted')
            aes_key = self.get_json().get_child_str('**/efuse/aes_key')
            aes_iv = self.get_json().get_child_str('**/efuse/aes_iv')

            if plp_flash_stimuli.genFlashImage(
                raw_stim=self.get_flash_preload_file(),
                bootBinary=self.get_json().get('**/runner/binaries').get_elem(0).get(),
                comps=comps,
                verbose=self.get_json().get('**/runner/verbose').get(),
                archi=self.get_json().get('**/pulp_chip_family').get(),
                flashType=self.get_json().get('**/runner/flash_type').get(),
                encrypt=encrypted, aesKey=aes_key, aesIv=aes_iv):
                return -1

        if self.get_json().get('**/efuse') is not None:
            efuse = runner.stim_utils.Efuse(self.get_json(), verbose=self.get_json().get('**/runner/verbose').get())
            efuse.gen_stim_txt('efuse_preload.data')

        return 0

    def run(self):

        autorun_conf = self.get_json().get('**/debug_bridge/autorun')
        if autorun_conf is not None and autorun_conf.get_bool() and not self.config.getOption('reentrant'):

            with open('autorun_config.json', 'w') as file:
                file.write(self.get_json().dump_to_string())

            options = self.get_json().get_child_str('**/debug_bridge/options')
            if options is None:
                options  = ''

            cmd_options = ['pulp-run-bridge', '--dir=%s' % self.config.getOption('dir'), '--config-file=%s/autorun_config.json' % self.config.getOption('dir'), '--options=%s' % options]
            if self.get_json().get_child_bool('**/runner/wait_pulp_run'):
                cmd_options.append('--wait-pulp-run')

            os.execlp(*cmd_options)


        autorun = self.get_json().get('**/debug_bridge/autorun')
        bridge_active = self.get_json().get('**/debug_bridge/active')

        bridge = autorun is not None and autorun.get_bool() or \
          self.get_json().get('**/gdb/active').get_bool() or \
          bridge_active is not None and bridge_active.get_bool()

        if bridge:
            self.get_json().get('**/jtag_proxy').set('active', True)
            self.get_json().get('gvsoc').set('use_external_bridge', True)

            if not self.get_json().get_child_bool('**/runner/wait_pulp_run'):
                self.get_json().get('gvsoc').set('no_exit', True)

        if not bridge and self.get_json().get_child_str('**/runner/boot-mode') != 'bridge' and self.get_json().get_child_str('**/runner/boot-mode') != 'rom' and self.get_json().get_child_str('**/runner/boot-mode') != 'jtag':
            binaries = self.get_json().get('**/runner/binaries').get_dict()
            for binary in binaries:
                self.get_json().get('**/plt_loader').set('binaries', binary)

        if self.gen_rom_stimuli:
            self.get_json().get('**/soc/rom').set('stim_file', 'stimuli/rom.bin')

        if self.get_json().get('**/efuse') is not None:
            self.get_json().get('**/soc/efuse').set('stim_file', 'efuse_preload.data')

        if self.gen_flash_stimuli:
            if self.get_json().get('**/spiflash') is not None:
                self.get_json().get('**/spiflash').set('stim_file', self.get_flash_preload_file())


        if self.get_json().get_child_str('**/runner/boot-mode') != 'rom' and not bridge:
            set_pc_addr = self.get_json().get_child_int('**/loader/set_pc_addr')
            if set_pc_addr != None:
                self.get_json().get('**/plt_loader').set('set_pc_addr', '0x%x' % set_pc_addr)
            set_pc_offset = self.get_json().get_child_str('**/loader/set_pc_offset')
            if set_pc_offset != None:
                self.get_json().get('**/plt_loader').set('set_pc_offset', set_pc_offset)
            start_addr = self.get_json().get_child_int('**/loader/start_addr')
            if start_addr != None:
                self.get_json().get('**/plt_loader').set('start_addr', '0x%x' % start_addr)
            start_value = self.get_json().get_child_int('**/loader/start_value')
            if start_value != None:
                self.get_json().get('**/plt_loader').set('start_value', '0x%x' % start_value)

        if self.get_json().get('**/fs/files') is not None or self.get_json().get_child_bool('**/runner/boot_from_flash'):
            if self.get_json().get('**/flash') is not None:
                self.get_json().get('**/flash').set('preload_file', self.get_flash_preload_file())


        with open('plt_config.json', 'w') as file:
            file.write(self.get_json().dump_to_string())


        os.environ['PULP_CONFIG_FILE'] = os.path.join(os.getcwd(), 'plt_config.json')

        top = self.get_json().get_child_str('system_tree/vp_class')

        if top is None:
            raise Exception("The specified configuration does not contain any"
                            " top component")

        gvsoc_config = self.get_json().get('gvsoc')
        debug_mode = len(gvsoc_config.get('trace').get()) != 0 or len(gvsoc_config.get('event').get()) != 0

        trace_engine = vp.trace_engine.component(name=None, config=gvsoc_config, debug=debug_mode)

        power_engine = trace_engine.new(
            name=None,
            component='vp.power_engine',
            config=gvsoc_config
        )

        time_engine = power_engine.new(
            name=None,
            component='vp.time_domain',
            config=self.get_json()
        )

        top_comp = time_engine.new(
            name='sys',
            component=top,
            config=self.get_json().get('system_tree')
        )

        trace_engine.get_port('out').bind_to(top_comp.get_port('trace'))

        trace_engine.bind()

        trace_engine.post_post_build_all()

        trace_engine.pre_start_all()

        trace_engine.start_all()

        trace_engine.post_start_all()

        trace_engine.final_bind()

        trace_engine.reset_all(True)

        if not self.get_json().get_child_bool('**/gvsoc/use_external_bridge'):
            trace_engine.reset_all(False)

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

    def power(self):
        if os.system('power_report_extract --report=power_report.csv --dump --config=plt_config.json --output=power_synthesis.txt') != 0:
            return -1

        return 0
