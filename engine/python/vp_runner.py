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
from os import listdir
from os.path import isfile, join, isdir
import os.path

from plp_platform import *
import plp_flash_stimuli
import shlex
import json_tools as js
import pulp_config
from prettytable import PrettyTable

import vcd.gtkw

def gen_gtkw_core_traces(gtkw, path):
    gtkw.trace(path + '.pc[31:0]', 'pc')
    gtkw.trace(path + '.asm', 'asm')
    gtkw.trace(path + '.func', 'func')
    gtkw.trace(path + '.inline_func', 'inline_func')
    gtkw.trace(path + '.file', 'file')
    gtkw.trace(path + '.line[31:0]', 'line', datafmt='dec')
    with gtkw.group('events', closed=True):
        gtkw.trace(path + '.pcer_cycles', 'cycles')
        gtkw.trace(path + '.pcer_instr', 'instr')
        gtkw.trace(path + '.pcer_ld_stall', 'ld_stall')
        gtkw.trace(path + '.pcer_jmp_stall', 'jmp_stall')
        gtkw.trace(path + '.pcer_imiss', 'imiss')
        gtkw.trace(path + '.pcer_ld', 'ld')
        gtkw.trace(path + '.pcer_st', 'st')
        gtkw.trace(path + '.pcer_jump', 'jump')
        gtkw.trace(path + '.pcer_branch', 'branch')
        gtkw.trace(path + '.pcer_taken_branch', 'taken_branch')
        gtkw.trace(path + '.pcer_rvc', 'rvc')
        gtkw.trace(path + '.pcer_ld_ext', 'ld_ext')
        gtkw.trace(path + '.pcer_st_ext', 'st_ext')
        gtkw.trace(path + '.pcer_ld_ext_cycles', 'ld_ext_cycles')
        gtkw.trace(path + '.pcer_st_ext_cycles', 'st_ext_cycles')
        gtkw.trace(path + '.pcer_tcdm_cont', 'tcdm_cont')
        gtkw.trace(path + '.misaligned', 'misaligned')

def check_user_traces(gtkw, path, user_traces):
    traces = user_traces.get_items()
    if traces is not None:
        for name, trace in user_traces.get_items().items():
            view_path = trace.get_str('view_path')

            if view_path.find('.') == -1:
                parent = None
                name = view_path
            else:
                parent, name = view_path.rsplit('.', 1)

            if parent == path:
                vcd_path = trace.get_str('path')
                gtkw.trace(vcd_path, view_path)


def gen_gtkw_files(config, gv_config):
    nb_pe = config.get_int('**/cluster/nb_pe')

    user_traces = gv_config.get('**/vcd/traces')

    # Remove trace file so that we can switch between regular file and fifo
    if os.path.exists('all.vcd'):
        os.remove('all.vcd')

    if len(gv_config.get('event').get()) != 0:
        path = os.path.join(os.getcwd(), 'view.gtkw')
        with open(path, 'w') as file:
            gtkw = vcd.gtkw.GTKWSave(file)

            gtkw.dumpfile('all.vcd')

            with gtkw.group('overview'):
                check_user_traces(gtkw, 'overview', user_traces)
                with gtkw.group('fc'):
                    check_user_traces(gtkw, 'overview.fc', user_traces)
                    gtkw.trace('sys.board.chip.soc.fc.pc[31:0]', 'fc:pc')

                with gtkw.group('cluster'):
                    check_user_traces(gtkw, 'overview.cluster', user_traces)
                    for i in range(0, nb_pe):
                        gtkw.trace('sys.board.chip.cluster.pe%d.pc[31:0]' % i, 'pe_%d:pc' % i)

            with gtkw.group('chip', closed=True):
                check_user_traces(gtkw, 'chip', user_traces)
                with gtkw.group('fc', closed=True):
                    check_user_traces(gtkw, 'chip.fc', user_traces)
                    gen_gtkw_core_traces(gtkw, 'sys.board.chip.soc.fc')

                with gtkw.group('cluster', closed=True):
                    gtkw.trace('sys.board.chip.cluster.power_trace', datafmt='real', extraflags=['analog_step'])
                    check_user_traces(gtkw, 'chip.cluster', user_traces)
                    for i in range(0, nb_pe):
                        with gtkw.group('pe_%d' % i, closed=True):
                            gen_gtkw_core_traces(gtkw, 'sys.board.chip.cluster.pe%d' % i)


        print ()
        print ('A Gtkwave script has been generated and can be opened with the following command:')
        print ('gtkwave ' + path)
        print ()


    if gv_config.get_bool('**/vcd/gtkw'):
        gv_config.set('vcd/format', 'vcd')
        os.mkfifo('all.vcd')
        vcd.gtkw.spawn_gtkwave_interactive('all.vcd', 'view.gtkw', quiet=False)


class Runner(Platform):

    def __init__(self, config, js_config):

        super(Runner, self).__init__(config, js_config)

        parser = config.getParser()

        parser.add_argument("--no-debug-syms", dest="debug_syms", action="store_false", help="Deactivate debug symbol parsing, which can then be used for traces")

        parser.add_argument("--trace", dest="traces", default=[], action="append", help="Specify gvsoc trace")

        parser.add_argument("--event", dest="events", default=[], action="append", help="Specify gvsoc event (for VCD traces)")

        parser.add_argument("--event-format", dest="format", default=None, help="Specify events format (vcd or fst)")

        parser.add_argument("--gtkw", dest="gtkw", action="store_true", help="Dump events to pipe and open gtkwave in interactive mode")

        [args, otherArgs] = parser.parse_known_args()

        if 'devices' in args.command:

            parser.add_argument("--device", dest="devices", default=[], action="append", help="Gives more information about specified devices when using command 'devices'")

            [args, otherArgs] = parser.parse_known_args()

        self.args = args

        self.addCommand('run', 'Run execution on GVSOC')
        self.addCommand('prepare', 'Prepare binary for GVOSC')
        self.addCommand('devices', 'Show available devices')

        self.__prepare_env()

        for trace in args.traces:
            self.get_json().set('gvsoc/trace', trace)

        for event in args.events:
            self.get_json().set('gvsoc/event', event)

        if args.format is not None:
            self.get_json().set('gvsoc/vcd/format', args.format)

        if args.gtkw:
            self.get_json().set('gvsoc/vcd/gtkw', True)


    def devices(self):
        devices = []
        for path in js.get_paths():
            for file in listdir(os.path.join(path, 'devices')):
                if file.find('.json') != -1:
                    local_path = os.path.join('devices', file)
                    full_path = os.path.join(path, local_path)
                    if isfile(full_path):
                        config = pulp_config.get_config(file=full_path)

                        doc = config.get_str('doc_rst')
                        if doc is not None:
                            doc_path = os.path.join(path, doc)
                            devices.append([config, local_path, doc_path])

        if len(self.args.devices) == 0:
            x = PrettyTable(['Name', 'Path', 'Description', 'Supported platforms'])
            x.align = 'l'

            for device in devices:
                config  = device[0]
                x.add_row([config.get_str('name'), device[1], config.get_str('description'), ', '.join(config.get('platforms').get_dict())])

            print (x)

        else:
            for device in devices:
                if device[0].get_str('name') in self.args.devices:
                    config  = device[0]
                    doc_path = device[2]
                    print ()
                    with open(doc_path) as doc:
                        print (doc.read())




    def __prepare_env(self):

        self.gen_flash_stimuli = False
        self.gen_rom_stimuli = False

        if self.args.debug_syms:
            for binary in self.get_json().get('**/runner/binaries').get_dict():
                debug_binary = binary + '.debugInfo'
                self.get_json().set('**/debug_binaries', debug_binary)

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

        if self.args.debug_syms:
            for binary in self.get_json().get('**/runner/binaries').get_dict():
                if os.system('pulp-pc-info --file %s --all-file %s' % (binary, binary + '.debugInfo')) != 0:
                    raise Exception('Error while generating debug symbols information, make sure the toolchain and the binaries are accessible ')

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

            binary = self.get_json().get('**/runner/binaries').get_dict()
            if len(binary) > 0:
                binary = binary[0]
            else:
                binary = None

            if plp_flash_stimuli.genFlashImage(
                raw_stim=self.get_flash_preload_file(),
                bootBinary=binary,
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

        gen_gtkw_files(self.get_json(), gvsoc_config)

        power_engine = vp.power_engine.component(name=None, config=gvsoc_config, debug=debug_mode)

        time_engine = power_engine.new(
            name=None,
            component='vp.time_domain',
            config=self.get_json()
        )

        trace_engine = time_engine.new(
            name=None,
            component='vp.trace_engine',
            config=gvsoc_config
        )

        top_comp = time_engine.new(
            name='sys',
            component=top,
            config=self.get_json().get('system_tree')
        )

        trace_engine.get_port('out').bind_to(top_comp.get_port('trace'))

        power_engine.bind()

        power_engine.post_post_build_all()

        power_engine.pre_start_all()

        power_engine.start_all()

        power_engine.post_start_all()

        power_engine.final_bind()

        power_engine.reset_all(True)

        if not self.get_json().get_child_bool('**/gvsoc/use_external_bridge'):
            power_engine.reset_all(False)

        power_engine.load_all()

        status = time_engine.run()

        power_engine.stop_all()

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
