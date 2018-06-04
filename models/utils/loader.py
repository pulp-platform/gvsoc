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
from elftools.elf.elffile import ELFFile
import os

class component(vp.component):

    implementation = 'utils.loader_impl'

    def load(self):


        binaries = self.get_config().get('load-binary_eval')
        if binaries is not None:
            binaries = [eval(binaries)]
        else:
            binaries = self.get_config().get('binaries')


        if binaries is not None:
            for binary in binaries:
                with open(binary, 'rb') as file:
                    elffile = ELFFile(file)
                    for segment in elffile.iter_segments():
                        if segment['p_type'] == 'PT_LOAD':
                            self.get_impl().module.loader_io_req(
                                self.get_impl().instance,
                                segment['p_paddr'],
                                segment['p_filesz'],
                                True,
                                segment.data()
                            )
                            if segment['p_filesz'] < segment['p_memsz']:
                                self.get_impl().module.loader_memset(
                                    self.get_impl().instance,
                                    segment['p_paddr'] + segment['p_filesz'],
                                    segment['p_memsz'] - segment['p_filesz'],
                                    0
                                )




                            #self.segments.append(BinarySegment(segment['p_paddr'], segment.data()))

