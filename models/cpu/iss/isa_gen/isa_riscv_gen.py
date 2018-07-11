#!/usr/bin/env python3

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
 

from isa_gen import *
import argparse
import os.path

fpuGroup      = IsaGroup('fpu', offload=['shfpu_offload', '--shared-fpu'], timingTable=TimingTable(name='fpu', option='--fpu-timing', default='timing_fpu_private.cfg'))
rv32mGroup     = IsaGroup('rv32m', timingTable=TimingTable(name='rv32m', option='--rv32m-timing', default='timing_rv32m_ri5cy.cfg'))
    
fpuGroupOther = InstrGroup(fpuGroup, 'INSTR_GROUP_OTHER')
fpuGroupAdd   = InstrGroup(fpuGroup, 'INSTR_GROUP_FPU_ADD')
fpuGroupMul   = InstrGroup(fpuGroup, 'INSTR_GROUP_FPU_MUL')
fpuGroupDiv   = InstrGroup(fpuGroup, 'INSTR_GROUP_FPU_DIV')
fpuGroupFmadd = InstrGroup(fpuGroup, 'INSTR_GROUP_FPU_FMADD')
fpuGroupConv  = InstrGroup(fpuGroup, 'INSTR_GROUP_FPU_CONV')



rv32mGroupOther   = InstrGroup(rv32mGroup, 'INSTR_GROUP_RV32M_OTHER')
rv32mGroupMul     = InstrGroup(rv32mGroup, 'INSTR_GROUP_RV32M_MUL')
rv32mGroupMulh    = InstrGroup(rv32mGroup, 'INSTR_GROUP_RV32M_MULH')
rv32mGroupDiv     = InstrGroup(rv32mGroup, 'INSTR_GROUP_RV32M_DIV')

class R5(Instr):
    def __init__(self, label, format, encoding, decode=None, N=None, L=None, mapTo=None, group=None, fast_handler=False, tags=[]):

        # Encodings for non-compressed instruction sets
              #   3 3 2 2 2 2 2       2 2 2 2 2       1 1 1 1 1       1 1 1       1 1
              #   1 0 9 8 7 6 5       4 3 2 1 0       9 8 7 6 5       4 3 2       1 0 9 8 7       6 5 4 3 2 1 0
              #   X X X X X X X   |   X X X X X   |   X X X X X   |   X X X   |   X X X X X   |   X X X X X X X
        # R   #         f7        |      rs2      |      rs1      |     f3    |       rd      |      opcode
        # RF  #         f7        |      rs2      |      rs1      |  ui[2:0]  |       rd      |      opcode
        # R2F #                  f7               |      rs1      |  ui[2:0]  |       rd      |      opcode
        # R3F #                  f7               |      rs1      |     f3    |       rd      |      opcode
        # R4U #    rs3     |  f2  |      rs2      |      rs1      |  ui[2:0]  |       rd      |      opcode
        # RRRR#         f7        |      rs2      |      rs1      |     f3    |     rd/rs3    |      opcode
        # RRRS#      f6      |     si[0|5:1]      |      rs2      |     f3    |     rd/rs1    |      opcode
        # RRRU#      f6      |     ui[0|5:1]      |      rs2      |     f3    |     rd/rs1    |      opcode
        #RRRU2#   f2 |   ui[4:0]  |      rs2      |      rs1      |     f3    |       rd      |      opcode
        #RRRRU#   f2 |   ui[4:0]  |      rs2      |      rs1      |     f3    |     rd/rs3    |      opcode
        # R1  #         f7        |       -       |      rs1      |     f3    |       rd      |      opcode
        # RRU #      f6      |     ui[0|5:1]      |      rs1      |     f3    |       rd      |      opcode
        # RRS #      f6      |     si[0|5:1]      |      rs1      |     f3    |       rd      |      opcode
        # RRU2#     f3 |ui[7:6]|f1|ui[0|5:1]        |      rs1      |     f3    |       rd      |      opcode
        # LR  #         f7        |      rs2      |      rs1      |     f3    |       rd      |      opcode        # Indirect addressing mode
        # RR  #   f2 |     rs3    |      rs2      |      rs1      |     f3    |       rd      |      opcode
        # SR  #   f2 |     rs3    |      rs2      |      rs1      |     f3    |       rd      |      opcode        # Indirect addressing mode
        # I   #              si[11:0]             |      rs1      |     f3    |       rd      |      opcode
        # L   #              si[11:0]             |      rs1      |     f3    |       rd      |      opcode        # Indirect addressing mode
        # IU  #              ui[11:0]             |      rs1      |     f3    |       rd      |      opcode
        # I1U #         f7        |    ui[4:0]    |      rs1      |     f3    |       rd      |      opcode
        # I2U #             ui0[11:0]             |   ui1[4:0]    |     f3    |       rd      |      opcode
        # I3U #    f4    |          ui[7:0]       |                    f13                    |      opcode
        # I4U #   f2 | ui0[4:0]   |   ui1[4:0]    |      rs1      |     f3    |       rd      |      opcode
        # I5U #   f2 | ui0[4:0]   |   ui1[4:0]    |      rs1      |     f3    |     rs2/rd    |      opcode
        # IOU #                  f12              |   ui0[4:0]    |     f3    |       f5      |      opcode
        # S   #       si[11:5]    |      rs2      |      rs1      |     f3    |     si[4:0]   |      opcode        # Indirect addressing mode
        # S1  #   f2 |     rs3    |      rs2      |      rs1      |     f3    |       f5      |      opcode 
        # SB  #     si[12|10:5]   |      rs2      |      rs1      |     f3    |   si[4:1|11]  |      opcode
        # SB2 #     si[12|10:5]   |    si[4:0]    |      rs1      |     f3    |   si[4:1|11]  |      opcode
        # U   #                           ui[31:12]                           |       rd      |      opcode
        # UJ  #                    si[20|10:1|11|19:15|14:12]                 |       rd      |      opcode
        # HL0 #              ui1[11:0]            |      rs1      |     f3    |  opcode  |ui0[0]|      opcode
        # HL1 #              ui1[11:0]            |    ui2[4:0]   |     f3    |  opcode  |ui0[0]|      opcode
        # Z   #                 -                 |       -       |     f3    |        -      |      opcode
        
        self.args = []

        if format == 'R': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            InReg (1, Range(20, 5)),
                            ]
        elif format == 'F': 
            self.args = [   InReg (0, Range(15, 5)),
                            ]
        elif format == 'RF': 
            self.args = [   OutFReg(0, Range(7,  5)),
                            InFReg (0, Range(15, 5)),
                            InFReg (1, Range(20, 5)),
                            UnsignedImm(0, Range(12, 3)),
                            ]
        elif format == 'RF2': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InFReg (0, Range(15, 5)),
                            InFReg (1, Range(20, 5)),
                            UnsignedImm(0, Range(12, 3)),
                            ]
        elif format == 'R2F1': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InFReg (0, Range(15, 5)),
                            UnsignedImm(0, Range(12, 3)),
                            ]
        elif format == 'R2F2': 
            self.args = [   OutFReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            UnsignedImm(0, Range(12, 3)),
                            ]
        elif format == 'R2F3': 
            self.args = [   OutFReg(0, Range(7,  5)),
                            InFReg (0, Range(15, 5)),
                            UnsignedImm(0, Range(12, 3)),
                            ]
        elif format == 'R3F': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InFReg (0, Range(15, 5)),
                            ]
        elif format == 'R3F2': 
            self.args = [   OutFReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            ]
        elif format == 'R4U': 
            self.args = [   OutFReg(0, Range(7,  5)),
                            InFReg (0, Range(15, 5)),
                            InFReg (1, Range(20, 5)),
                            InFReg (2, Range(27, 5)),
                            UnsignedImm(0, Range(12, 3)),
                            ]
        elif format == 'RRRR': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (2, Range(7,  5), dumpName=False),
                            InReg (0, Range(15, 5)),
                            InReg (1, Range(20, 5)),
                            ]
        elif format == 'RRRR2': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(7,  5), dumpName=False),
                            InReg (1, Range(15, 5)),
                            InReg (2, Range(20, 5)),
                            ]
        elif format == 'RRRS': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(7,  5)),
                            InReg (1, Range(15, 5)),
                            SignedImm(0, Ranges([[25, 1, 0], [20, 5, 1]])),
                            ]
        elif format == 'RRRU': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(7,  5)),
                            InReg (1, Range(15, 5)),
                            UnsignedImm(0, Ranges([[25, 1, 0], [20, 5, 1]])),
                            ]
        elif format == 'RRRU2': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            InReg (1, Range(20, 5)),
                            UnsignedImm(0, Range(25, 5)),
                            ]
        elif format == 'RRRRU': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (2, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            InReg (1, Range(20, 5)),
                            UnsignedImm(0, Range(25, 5)),
                            ]
        elif format == 'R1': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            ]
        elif format == 'RRU': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            UnsignedImm(0, Ranges([[25, 1, 0], [20, 5, 1]])),
                            ]
        elif format == 'RRS': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            SignedImm(0, Ranges([[25, 1, 0], [20, 5, 1]])),
                            ]
        elif format == 'RRU2': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            UnsignedImm(0, Ranges([[25, 1, 0], [20, 5, 1]])),
                            ]
        elif format == 'LR': 
            self.args = [   OutReg(0, Range(7,  5)),
                            Indirect(InReg (0, Range(15, 5)), InReg (1, Range(20, 5))),
                            ]
        elif format == 'LRPOST': 
            self.args = [   OutReg(0, Range(7,  5)),
                            Indirect(InReg (0, Range(15, 5)), InReg (1, Range(20, 5)), postInc=True),
                            ]
        elif format == 'RR': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            InReg (1, Range(20, 5)),
                            InReg (2, Range(25, 5)),
                        ]
        elif format == 'SR': 
            self.args = [   InReg (1, Range(20, 5)),
                            Indirect(InReg (0, Range(15, 5)), InReg (2, Range(7, 5))),
                        ]
        elif format == 'SR_OLD': 
            self.args = [   InReg (1, Range(20, 5)),
                            Indirect(InReg (0, Range(15, 5)), InReg (2, Range(25, 5))),
                        ]
        elif format == 'I': 
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            SignedImm(0, Range(20, 12)),
                        ]
        elif format == 'Z': 
            self.args = [
                        ]
        elif format == 'L': 
            self.args = [   OutReg(0, Range(7,  5)),
                            Indirect(InReg (0, Range(15, 5)), SignedImm(0, Range(20, 12))),
                        ]
        elif format == 'LRES': 
            self.args = [   OutReg(0, Range(7,  5)),
                            Indirect(InReg (0, Range(15, 5))),
                            UnsignedImm(0, Range(25, 1)),
                            UnsignedImm(1, Range(26, 1)),
                        ]
        elif format == 'FL': 
            self.args = [   OutFReg(0, Range(7,  5)),
                            Indirect(InReg (0, Range(15, 5)), SignedImm(0, Range(20, 12))),
                        ]
        elif format == 'LPOST': 
            self.args = [   OutReg(0, Range(7,  5)),
                            Indirect(InReg (0, Range(15, 5)), SignedImm(0, Range(20, 12)), postInc=True),
                        ]
        elif format == 'IU':
            self.args = [   OutReg(0, Range(7,  5)),
                            InReg (0, Range(15, 5)),
                            UnsignedImm(0, Range(20, 12)),
                        ]
        elif format == 'IUR':
            self.args = [   OutReg(0, Range(7,  5)),
                            UnsignedImm(1, Range(15, 5)),
                            UnsignedImm(0, Range(20, 12)),
                        ]
        elif format == 'I1U':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Range(15, 5)),
                            UnsignedImm(0, Range(20, 5)),
                        ]
        elif format == 'I2U':
            self.args = [   OutReg(0, Range(7, 5)),
                            UnsignedImm(0, Range(20, 12)),
                            UnsignedImm(1, Range(15, 5)),
                        ]
        elif format == 'I3U':
            self.args = [   UnsignedImm(0, Range(20, 8)),
			]
        elif format == 'I4U':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Range(15, 5)),
                            UnsignedImm(0, Range(25, 5)),
                            UnsignedImm(1, Range(20, 5)),
                        ]
        elif format == 'I5U':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(1, Range(7, 5), dumpName=False),
                            InReg(0, Range(15, 5)),
                            UnsignedImm(0, Range(25, 5)),
                            UnsignedImm(1, Range(20, 5)),
                        ]
        elif format == 'I5U2':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(1, Range(7, 5), dumpName=False),
                            InReg(0, Range(15, 5)),
                            InReg(2, Range(20, 5)),
                        ]
        elif format == 'IOU':
            self.args = [   UnsignedImm(0, Range(15, 5)),
                        ]
        elif format == 'S':
            self.args = [   InReg(1, Range(20, 5)),
                            Indirect(InReg(0, Range(15, 5)), SignedImm(0, Ranges([[7, 5, 0], [25, 7, 5]]))),
                        ]
        elif format == 'SCOND':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(1, Range(20, 5)),
                            Indirect(InReg(0, Range(15, 5))),
                            UnsignedImm(0, Range(25, 1)),
                            UnsignedImm(1, Range(26, 1)),
                        ]
        elif format == 'FS':
            self.args = [   InFReg(1, Range(20, 5)),
                            Indirect(InReg(0, Range(15, 5)), SignedImm(0, Ranges([[7, 5, 0], [25, 7, 5]]))),
                        ]
        elif format == 'SPOST':
            self.args = [   InReg(1, Range(20, 5)),
                            Indirect(InReg(0, Range(15, 5)), SignedImm(0, Ranges([[7, 5, 0], [25, 7, 5]])), postInc=True),                            
                        ]
        elif format == 'S1':
            self.args = [   InReg(0, Range(15, 5)),
                            InReg(1, Range(20, 5)),
                            InReg(2, Range(25,5)),
                        ]
        elif format == 'SRPOST':
            self.args = [   InReg(1, Range(20, 5)),
                            Indirect(InReg(0, Range(15, 5)), InReg(2, Range(7,5)), postInc=True),
                        ]
        elif format == 'SB':
            self.args = [   InReg(0, Range(15, 5)),
                            InReg(1, Range(20, 5)),
                            SignedImm(0, Ranges([[7, 1, 11], [8, 4, 1], [25, 6, 5], [31, 1, 12]])),
                        ]
        elif format == 'SB2':
            self.args = [   InReg(0, Range(15, 5)),
                            SignedImm(0, Ranges([[7, 1, 11], [8, 4, 1], [25, 6, 5], [31, 1, 12]])),
                            SignedImm(1, Range(20, 5)),
                        ]
        elif format == 'U':
            self.args = [   OutReg(0, Range(7, 5)),
                            UnsignedImm(0, Range(12, 20, 12)),
                        ]
        elif format == 'UJ':
            self.args = [   OutReg(0, Range(7, 5)),
                            SignedImm(0, Ranges([[12, 8, 12], [20, 1, 11], [21, 10, 1], [31, 1, 20]])),
                        ]
        elif format == 'HL0':
            self.args = [   UnsignedImm(0, Range(7, 1)),
                            InReg (0, Range(15, 5)),                            
                            UnsignedImm(1, Range(20, 12)),
                        ]
        elif format == 'HL1':
            self.args = [   UnsignedImm(0, Range(7, 1)),
                            UnsignedImm(1, Range(20, 12)),
                            UnsignedImm(2, Range(15, 5)),
                        ]

        # Encodings for compressed instruction set

               # X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X
        # CR   #      func4    |      rd/rs1       |         rs2       |  op      #
        # CR1  #      func4    |        rs1        |          f5       |  op      # rs2=0, rd=0, si=0
        # CR2  #      func4    |        rd         |         rs2       |  op      # rs1=0
        # CR3  #      func4    |        rs1        |         rs2       |  op      # rd=1, si=0
        # CI1  #   func3 | si[5]|     rd/rs1       |      si[4:0]      |  op      # 
        # CI1U #   func3 | ui[5]|     rd/rs1       |      ui[4:0]      |  op      # 
        # CI2  #   func3 | si[7]|     rd/rs1       |      si[6:2]      |  op      #
        # CI3  #   func3 | ui[5]|       rd         |     ui[4:2|7:6]   |  op      # rs1=2, ui->si
        # CI4  #   func3 | si[9]|      func5       |    si[4|6|8:7|5]  |  op      # rs1=2, rd=2
        # CI5  #   func3 |si[17]|     rd/rs1       |      si[16:12]    |  op      # si->ui
        # CI6  #   func3 | si[5]|       rd         |      si[4:0]      |  op      # rs1=0
        # CSS  #   func3   |      ui[5:2|7:6]      |         rs2       |  op      # rs1=2, ui->si
        # CIW  #   func3   |        ui[5:4|9:6|2|3]        |    rd'    |  op      # rs1=2, ui->si
        # CL   #   func3   |      ui   |   rs1'    |   ui  |    rd'    |  op      # ui->si
        # CL1  #                                   op                             # rd=0
        # CS   #   func3   |  ui[5:3]  |   rs1'    |ui[6|2]|    rs2'   |  op      # ui->si
        # CS2  #      func6            |  rd/rs1   | func  |    rs2    |  op      #
        # CB1  #   func3   | si[8|4:3] |   rs1'    |   si[7:6|2:1|5]   |  op      # rs2=0
        # CB2  #   func3 |ui[5]| func2 |  rd/rs1'  |       ui[4:0]     |  op      # 
        # CB2S #   func3 |si[5]| func2 |  rd/rs1'  |       si[4:0]     |  op      # 
        # CJ   #   func3   |         si[11|4|9:8|6|7|3:1|5]            |  op      # rd=0
        # CJ1  #   func3   |         si[11|4|9:8|6|7|3:1|5]            |  op      # rd=1

        elif format == 'CR':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Range(7, 5)),
                            InReg(1, Range(2, 5)),
                        ]
        elif format == 'CR1':
            self.args = [   OutReg(0, Const(0)),
                            InReg(0, Range(7, 5)),
                            InReg(1, Const(0)),
                            SignedImm(0, Const(0)),
                        ]
        elif format == 'CR2':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Const(0)),
                            InReg(1, Range(2, 5)),
                        ]
        elif format == 'CR3':
            self.args = [   OutReg(0, Const(1)),
                            InReg(0, Range(7, 5)),
                            InReg(1, Range(2, 5)),
                            SignedImm(0, Const(0)),
                        ]
        elif format == 'CI1':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Range(7, 5)),
                            SignedImm(0, Ranges([[2, 5, 0], [12, 1, 5]])),
                        ]
        elif format == 'CI1U':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Range(7, 5)),
                            UnsignedImm(0, Ranges([[2, 5, 0], [12, 1, 5]])),
                        ]
        elif format == 'CI2':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Range(7, 5)),
                            SignedImm(0, Ranges([[2, 5, 2], [12, 1, 7]])),
                        ]
        elif format == 'CI3':
            self.args = [   OutReg(0, Range(7, 5)),
                            Indirect(InReg(0, Const(2)), SignedImm(0, Ranges([[4, 3, 2], [12, 1, 5], [2, 2, 6]]), isSigned=False)),
                        ]
        elif format == 'FCI3':
            self.args = [   OutFReg(0, Range(7, 5)),
                            Indirect(InReg(0, Const(2)), SignedImm(0, Ranges([[4, 3, 2], [12, 1, 5], [2, 2, 6]]), isSigned=False)),
                        ]
        elif format == 'CI4':
            self.args = [   OutReg(0, Const(2)),
                            InReg(0, Const(2)),
                            SignedImm(0, Ranges([[6, 1, 4], [2, 1, 5], [5, 1, 6], [3, 2, 7], [12, 1, 9]])),
                        ]
        elif format == 'CI5':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Range(7, 5)),
                            UnsignedImm(0, Ranges([[2, 5, 12], [12, 1, 17]]), isSigned=True),
                        ]
        elif format == 'CI6':
            self.args = [   OutReg(0, Range(7, 5)),
                            InReg(0, Const(0)),
                            SignedImm(0, Ranges([[2, 5, 0], [12, 1, 5]])),
                        ]
        elif format == 'CSS':
            self.args = [   InReg(1, Range(2, 5)),
                            Indirect(InReg(0, Const(2)), SignedImm(0, Ranges([[9, 4, 2], [7, 2, 6]]), isSigned=False)),
                        ]
        elif format == 'FCSS':
            self.args = [   InFReg(1, Range(2, 5)),
                            Indirect(InReg(0, Const(2)), SignedImm(0, Ranges([[9, 4, 2], [7, 2, 6]]), isSigned=False)),
                        ]
        elif format == 'CIW':
            self.args = [   OutRegComp(0, Range(2, 3)),
                            InReg(0, Const(2)),
                            SignedImm(0, Ranges([[6, 1, 2], [5, 1, 3], [11, 2, 4], [7, 4, 6]]), isSigned=False),
                        ]
        elif format == 'CL':
            self.args = [   OutRegComp(0, Range(2, 3)),
                            Indirect(InRegComp(0, Range(7, 3)), SignedImm(0, Ranges([[6, 1, 2], [10, 3, 3], [5, 1, 6]]), isSigned=False)),
                        ]
        elif format == 'FCL':
            self.args = [   OutFRegComp(0, Range(2, 3)),
                            Indirect(InRegComp(0, Range(7, 3)), SignedImm(0, Ranges([[6, 1, 2], [10, 3, 3], [5, 1, 6]]), isSigned=False)),
                        ]
        elif format == 'CS':
            self.args = [   InRegComp(1, Range(2, 3)),
                            Indirect(InRegComp(0, Range(7, 3)), SignedImm(0, Ranges([[6, 1, 2], [10, 3, 3], [5, 1, 6]]), isSigned=False)),
                        ]
        elif format == 'FCS':
            self.args = [   InFRegComp(1, Range(2, 3)),
                            Indirect(InRegComp(0, Range(7, 3)), SignedImm(0, Ranges([[6, 1, 2], [10, 3, 3], [5, 1, 6]]), isSigned=False)),
                        ]
        elif format == 'CS2':
            self.args = [   OutRegComp(0, Range(7, 3)),
                            InRegComp(0, Range(7, 3)),
                            InRegComp(1, Range(2, 3)),
                        ]
        elif format == 'CB1':
            self.args = [   InRegComp(0, Range(7, 3)),
                            InReg(1, Const(0)),
                            SignedImm(0, Ranges([[3, 2, 1], [10, 2, 3], [2, 1, 5], [5, 2, 6], [12, 1, 8]])),
                        ]
        elif format == 'CB2':
            self.args = [   OutRegComp(0, Range(7, 3)),
                            InRegComp(0, Range(7, 3)),
                            UnsignedImm(0, Ranges([[2, 5, 0], [12, 1, 5]])),
                        ]
        elif format == 'CB2S':
            self.args = [   OutRegComp(0, Range(7, 3)),
                            InRegComp(0, Range(7, 3)),
                            SignedImm(0, Ranges([[2, 5, 0], [12, 1, 5]])),
                        ]
        elif format == 'CJ':
            self.args = [   OutReg(0, Const(0)),
                            SignedImm(0, Ranges([[3, 3, 1], [11, 1, 4], [2, 1, 5], [7, 1, 6], [6, 1, 7], [9, 2, 8], [8, 1, 10], [12, 1, 11]])),
                        ]
        elif format == 'CJ1':
            self.args = [   OutReg(0, Const(1)),
                            SignedImm(0, Ranges([[3, 3, 1], [11, 1, 4], [2, 1, 5], [7, 1, 6], [6, 1, 7], [9, 2, 8], [8, 1, 10], [12, 1, 11]])),
                        ]
        else:
            raise Exception('Undefined format: %s' % format)

        super(R5, self).__init__(label, type, encoding, decode, N, L, mapTo, group=group, fast_handler=fast_handler, tags=tags)





#
# RV32I
#
rv32i = [

    R5('lui',   'U',  '------- ----- ----- --- ----- 0110111'),
    R5('auipc', 'U',  '------- ----- ----- --- ----- 0010111', 'auipc_decode'),
    R5('jal',   'UJ', '------- ----- ----- --- ----- 1101111', 'jal_decode', fast_handler=True),
    R5('jalr',  'I',  '------- ----- ----- 000 ----- 1100111', fast_handler=True),
    R5('beq',   'SB', '------- ----- ----- 000 ----- 1100011', 'bxx_decode', fast_handler=True),
    R5('bne',   'SB', '------- ----- ----- 001 ----- 1100011', 'bxx_decode', fast_handler=True),
    R5('blt',   'SB', '------- ----- ----- 100 ----- 1100011', 'bxx_decode', fast_handler=True),
    R5('bge',   'SB', '------- ----- ----- 101 ----- 1100011', 'bxx_decode', fast_handler=True),
    R5('bltu',  'SB', '------- ----- ----- 110 ----- 1100011', 'bxx_decode', fast_handler=True),
    R5('bgeu',  'SB', '------- ----- ----- 111 ----- 1100011', 'bxx_decode', fast_handler=True),
    R5('lb',    'L',  '------- ----- ----- 000 ----- 0000011', fast_handler=True, tags=["load"]),
    R5('lh',    'L',  '------- ----- ----- 001 ----- 0000011', fast_handler=True, tags=["load"]),
    R5('lw',    'L',  '------- ----- ----- 010 ----- 0000011', fast_handler=True, tags=["load"]),
    R5('lbu',   'L',  '------- ----- ----- 100 ----- 0000011', fast_handler=True, tags=["load"]),
    R5('lhu',   'L',  '------- ----- ----- 101 ----- 0000011', fast_handler=True, tags=["load"]),
    R5('sb',    'S',  '------- ----- ----- 000 ----- 0100011', fast_handler=True, tags=["store"]),
    R5('sh',    'S',  '------- ----- ----- 001 ----- 0100011', fast_handler=True, tags=["store"]),
    R5('sw',    'S',  '------- ----- ----- 010 ----- 0100011', fast_handler=True, tags=["store"]),
    R5('addi',  'I',  '------- ----- ----- 000 ----- 0010011'),
    R5('addi',  'Z',  '0000000 00000 00000 000 00000 0010011', mapTo="nop", L='nop'),
    R5('slti',  'I',  '------- ----- ----- 010 ----- 0010011'),
    R5('sltiu', 'I',  '------- ----- ----- 011 ----- 0010011'),
    R5('xori',  'I',  '------- ----- ----- 100 ----- 0010011'),
    R5('ori',   'I',  '------- ----- ----- 110 ----- 0010011'),
    R5('andi',  'I',  '------- ----- ----- 111 ----- 0010011'),
    R5('slli',  'I1U','0000000 ----- ----- 001 ----- 0010011'),
    R5('srli',  'I1U','0000000 ----- ----- 101 ----- 0010011'),
    R5('srai',  'I1U','0100000 ----- ----- 101 ----- 0010011'),
    R5('add',   'R',  '0000000 ----- ----- 000 ----- 0110011'),
    R5('sub',   'R',  '0100000 ----- ----- 000 ----- 0110011'),
    R5('sll',   'R',  '0000000 ----- ----- 001 ----- 0110011'),
    R5('slt',   'R',  '0000000 ----- ----- 010 ----- 0110011'),
    R5('sltu',  'R',  '0000000 ----- ----- 011 ----- 0110011'),
    R5('xor',   'R',  '0000000 ----- ----- 100 ----- 0110011'),
    R5('srl',   'R',  '0000000 ----- ----- 101 ----- 0110011'),
    R5('sra',   'R',  '0100000 ----- ----- 101 ----- 0110011'),
    R5('or',    'R',  '0000000 ----- ----- 110 ----- 0110011'),
    R5('and',   'R',  '0000000 ----- ----- 111 ----- 0110011'),
    R5('fence', 'I3U','0000--- ----- 00000 000 00000 0001111'),
    R5('fence.i','I', '0000000 00000 00000 001 00000 0001111'),
    #R5('ecall',   'I',  '0000000 00000 00000 000 00000 1110011'), # , mapTo="SCALL_exec"),
    R5('sbreak',  'I',  '0000000 00001 00000 000 00000 1110011')
#
]





#
# RV32M
#
rv32m = [

    R5('mul', 'R', '0000001 ----- ----- 000 ----- 0110011', group=rv32mGroupMul),
    R5('mulh',  'R', '0000001 ----- ----- 001 ----- 0110011', group=rv32mGroupMulh),
    R5('mulhsu','R', '0000001 ----- ----- 010 ----- 0110011', group=rv32mGroupMulh),
    R5('mulhu', 'R', '0000001 ----- ----- 011 ----- 0110011', group=rv32mGroupMulh),
    R5('div',   'R', '0000001 ----- ----- 100 ----- 0110011', group=rv32mGroupDiv),
    R5('divu',  'R', '0000001 ----- ----- 101 ----- 0110011', group=rv32mGroupDiv),
    R5('rem',   'R', '0000001 ----- ----- 110 ----- 0110011', group=rv32mGroupDiv),
    R5('remu',  'R', '0000001 ----- ----- 111 ----- 0110011', group=rv32mGroupDiv),

]

rv32a = [

    R5('lr.w', 'LRES',  '00010 -- 00000 ----- 010 ----- 0101111'),
    R5('sc.w', 'SCOND', '00011 -- ----- ----- 010 ----- 0101111')

]

rv32d = [
    R5('fld',       'FL', '------- ----- ----- 011 ----- 0000111', mapTo="lib_FLW"),
    R5('fsd',       'FS', '------- ----- ----- 011 ----- 0100111', mapTo="lib_FSW"),
    
    R5('c.fsd',      'FCS',  '101 --- --- -- --- 00'),
    R5('c.fsdsp',    'FCSS', '101 --- --- -- --- 10'),
    R5('c.fld',      'FCL',  '001 --- --- -- --- 00'),
    R5('c.flwsp',    'FCI3', '001 --- --- -- --- 10'),
]

rv32f = [

    R5('flw',       'FL', '------- ----- ----- 010 ----- 0000111', tags=["load"]),
    R5('fsw',       'FS', '------- ----- ----- 010 ----- 0100111'),
    R5('fmadd.s',   'R4U','-----00 ----- ----- --- ----- 1000011', group=fpuGroupFmadd),
    R5('fmsub.s',   'R4U','-----00 ----- ----- --- ----- 1000111', group=fpuGroupFmadd),
    R5('fnmsub.s',  'R4U','-----00 ----- ----- --- ----- 1001011', group=fpuGroupFmadd),
    R5('fnmadd.s',  'R4U','-----00 ----- ----- --- ----- 1001111', group=fpuGroupFmadd),

    R5('fadd.s',    'RF', '0000000 ----- ----- --- ----- 1010011', group=fpuGroupAdd),
    R5('fsub.s',    'RF', '0000100 ----- ----- --- ----- 1010011', group=fpuGroupAdd),
    R5('fmul.s',    'RF', '0001000 ----- ----- --- ----- 1010011', group=fpuGroupMul),
    R5('fdiv.s',    'RF', '0001100 ----- ----- --- ----- 1010011', group=fpuGroupDiv),
    R5('fsqrt.s',  'R2F3','0101100 00000 ----- --- ----- 1010011', group=fpuGroupDiv),

    R5('fsgnj.s',   'RF', '0010000 ----- ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fsgnjn.s',  'RF', '0010000 ----- ----- 001 ----- 1010011', group=fpuGroupConv),
    R5('fsgnjx.s',  'RF', '0010000 ----- ----- 010 ----- 1010011', group=fpuGroupConv),

    R5('fmin.s',    'RF', '0010100 ----- ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fmax.s',    'RF', '0010100 ----- ----- 001 ----- 1010011', group=fpuGroupConv),

    R5('fcvt.w.s', 'R2F1','1100000 00000 ----- --- ----- 1010011', group=fpuGroupConv),
    R5('fcvt.wu.s','R2F1','1100000 00001 ----- --- ----- 1010011', group=fpuGroupConv),

    R5('fmv.x.s',   'R3F','1110000 00000 ----- 000 ----- 1010011'),

    R5('feq.s',    'RF2', '1010000 ----- ----- 010 ----- 1010011'),
    R5('flt.s',    'RF2', '1010000 ----- ----- 001 ----- 1010011'),
    R5('fle.s',    'RF2', '1010000 ----- ----- 000 ----- 1010011'),

    R5('fclass.s',  'R3F','1110000 00000 ----- 001 ----- 1010011'),

    R5('fcvt.s.w', 'R2F2','1101000 00000 ----- --- ----- 1010011', group=fpuGroupConv),
    R5('fcvt.s.wu','R2F2','1101000 00001 ----- --- ----- 1010011', group=fpuGroupConv),

    R5('fmv.s.x',  'R3F2','1111000 00000 ----- 000 ----- 1010011'),
    
    R5('c.fsw',      'FCS',  '111 --- --- -- --- 00'),
    R5('c.fswsp',    'FCSS', '111 --- --- -- --- 10'),
    R5('c.flw',      'FCL',  '011 --- --- -- --- 00', tags=["load"]),
    R5('c.flwsp',    'FCI3', '011 --- --- -- --- 10', tags=["load"]),
]

rv32Xf16alt = [

    R5('fmadd.ah',   'R4U','-----10 ----- ----- 101 ----- 1000011', group=fpuGroupFmadd),
    R5('fmsub.ah',   'R4U','-----10 ----- ----- 101 ----- 1000111', group=fpuGroupFmadd),
    R5('fnmsub.ah',  'R4U','-----10 ----- ----- 101 ----- 1001011', group=fpuGroupFmadd),
    R5('fnmadd.ah',  'R4U','-----10 ----- ----- 101 ----- 1001111', group=fpuGroupFmadd),

    R5('fadd.ah',    'RF', '0000010 ----- ----- 101 ----- 1010011', group=fpuGroupAdd),
    R5('fsub.ah',    'RF', '0000110 ----- ----- 101 ----- 1010011', group=fpuGroupAdd),
    R5('fmul.ah',    'RF', '0001010 ----- ----- 101 ----- 1010011', group=fpuGroupMul),
    R5('fdiv.ah',    'RF', '0001110 ----- ----- 101 ----- 1010011', group=fpuGroupDiv),
    R5('fsqrt.ah',  'R2F3','0101110 00000 ----- 101 ----- 1010011', group=fpuGroupDiv),

    R5('fsgnj.ah',   'RF', '0010010 ----- ----- 100 ----- 1010011', group=fpuGroupConv),
    R5('fsgnjn.ah',  'RF', '0010010 ----- ----- 101 ----- 1010011', group=fpuGroupConv),
    R5('fsgnjx.ah',  'RF', '0010010 ----- ----- 110 ----- 1010011', group=fpuGroupConv),

    R5('fmin.ah',    'RF', '0010110 ----- ----- 100 ----- 1010011', group=fpuGroupConv),
    R5('fmax.ah',    'RF', '0010110 ----- ----- 101 ----- 1010011', group=fpuGroupConv),

    R5('fcvt.w.ah', 'R2F1','1100010 00000 ----- 101 ----- 1010011', group=fpuGroupConv),
    R5('fcvt.wu.ah','R2F1','1100010 00001 ----- 101 ----- 1010011', group=fpuGroupConv),

    R5('fmv.x.ah',   'R3F','1110010 00000 ----- 100 ----- 1010011', group=fpuGroupOther),

    R5('feq.ah',    'RF2', '1010010 ----- ----- 110 ----- 1010011', group=fpuGroupOther),
    R5('flt.ah',    'RF2', '1010010 ----- ----- 101 ----- 1010011', group=fpuGroupOther),
    R5('fle.ah',    'RF2', '1010010 ----- ----- 100 ----- 1010011', group=fpuGroupOther),

    R5('fclass.ah',  'R3F','1110010 00000 ----- 101 ----- 1010011', group=fpuGroupOther),

    R5('fcvt.ah.w', 'R2F2','1101010 00000 ----- 101 ----- 1010011', group=fpuGroupConv),
    R5('fcvt.ah.wu','R2F2','1101010 00001 ----- 101 ----- 1010011', group=fpuGroupConv),

    R5('fmv.ah.x',  'R3F2','1111010 00000 ----- 100 ----- 1010011', group=fpuGroupOther),
        
    R5('fcvt.s.ah', 'R2F2','0100000 00110 ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fcvt.ah.s', 'R2F2','0100010 00000 ----- 101 ----- 1010011', group=fpuGroupConv),
    
    R5('fcvt.h.ah', 'R2F2','0100010 00110 ----- --- ----- 1010011', group=fpuGroupConv),
    R5('fcvt.ah.h', 'R2F2','0100010 00010 ----- 101 ----- 1010011', group=fpuGroupConv),
]

rv32Xf16 = [

    R5('flh',       'FL', '------- ----- ----- 001 ----- 0000111', tags=["load"]),
    R5('fsh',       'FS', '------- ----- ----- 001 ----- 0100111'),
    R5('fmadd.h',   'R4U','-----10 ----- ----- --- ----- 1000011', group=fpuGroupFmadd),
    R5('fmsub.h',   'R4U','-----10 ----- ----- --- ----- 1000111', group=fpuGroupFmadd),
    R5('fnmsub.h',  'R4U','-----10 ----- ----- --- ----- 1001011', group=fpuGroupFmadd),
    R5('fnmadd.h',  'R4U','-----10 ----- ----- --- ----- 1001111', group=fpuGroupFmadd),

    R5('fadd.h',    'RF', '0000010 ----- ----- --- ----- 1010011', group=fpuGroupAdd),
    R5('fsub.h',    'RF', '0000110 ----- ----- --- ----- 1010011', group=fpuGroupAdd),
    R5('fmul.h',    'RF', '0001010 ----- ----- --- ----- 1010011', group=fpuGroupMul),
    R5('fdiv.h',    'RF', '0001110 ----- ----- --- ----- 1010011', group=fpuGroupDiv),
    R5('fsqrt.h',  'R2F3','0101110 00000 ----- --- ----- 1010011', group=fpuGroupDiv),

    R5('fsgnj.h',   'RF', '0010010 ----- ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fsgnjn.h',  'RF', '0010010 ----- ----- 001 ----- 1010011', group=fpuGroupConv),
    R5('fsgnjx.h',  'RF', '0010010 ----- ----- 010 ----- 1010011', group=fpuGroupConv),

    R5('fmin.h',    'RF', '0010110 ----- ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fmax.h',    'RF', '0010110 ----- ----- 001 ----- 1010011', group=fpuGroupConv),

    R5('fcvt.w.h', 'R2F1','1100010 00000 ----- --- ----- 1010011', group=fpuGroupConv),
    R5('fcvt.wu.h','R2F1','1100010 00001 ----- --- ----- 1010011', group=fpuGroupConv),

    R5('fmv.x.h',   'R3F','1110010 00000 ----- 000 ----- 1010011', group=fpuGroupOther),

    R5('feq.h',    'RF2', '1010010 ----- ----- 010 ----- 1010011', group=fpuGroupOther),
    R5('flt.h',    'RF2', '1010010 ----- ----- 001 ----- 1010011', group=fpuGroupOther),
    R5('fle.h',    'RF2', '1010010 ----- ----- 000 ----- 1010011', group=fpuGroupOther),

    R5('fclass.h',  'R3F','1110010 00000 ----- 001 ----- 1010011', group=fpuGroupOther),

    R5('fcvt.h.w', 'R2F2','1101010 00000 ----- --- ----- 1010011', group=fpuGroupConv),
    R5('fcvt.h.wu','R2F2','1101010 00001 ----- --- ----- 1010011', group=fpuGroupConv),

    R5('fmv.h.x',  'R3F2','1111010 00000 ----- 000 ----- 1010011', group=fpuGroupOther),

    R5('fcvt.s.h', 'R2F2','0100000 00010 ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fcvt.h.s', 'R2F2','0100010 00000 ----- --- ----- 1010011', group=fpuGroupConv),
]

rv32Xf8 = [

    R5('flb',       'FL', '------- ----- ----- 000 ----- 0000111', tags=["load"]),
    R5('fsb',       'FS', '------- ----- ----- 000 ----- 0100111'),
    R5('fmadd.b',   'R4U','-----11 ----- ----- --- ----- 1000011', group=fpuGroupFmadd),
    R5('fmsub.b',   'R4U','-----11 ----- ----- --- ----- 1000111', group=fpuGroupFmadd),
    R5('fnmsub.b',  'R4U','-----11 ----- ----- --- ----- 1001011', group=fpuGroupFmadd),
    R5('fnmadd.b',  'R4U','-----11 ----- ----- --- ----- 1001111', group=fpuGroupFmadd),

    R5('fadd.b',    'RF', '0000011 ----- ----- --- ----- 1010011', group=fpuGroupAdd),
    R5('fsub.b',    'RF', '0000111 ----- ----- --- ----- 1010011', group=fpuGroupAdd),
    R5('fmul.b',    'RF', '0001011 ----- ----- --- ----- 1010011', group=fpuGroupMul),
    R5('fdiv.b',    'RF', '0001111 ----- ----- --- ----- 1010011', group=fpuGroupDiv),
    R5('fsqrt.b',  'R2F3','0101111 00000 ----- --- ----- 1010011', group=fpuGroupDiv),

    R5('fsgnj.b',   'RF', '0010011 ----- ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fsgnjn.b',  'RF', '0010011 ----- ----- 001 ----- 1010011', group=fpuGroupConv),
    R5('fsgnjx.b',  'RF', '0010011 ----- ----- 010 ----- 1010011', group=fpuGroupConv),

    R5('fmin.b',    'RF', '0010111 ----- ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fmax.b',    'RF', '0010111 ----- ----- 001 ----- 1010011', group=fpuGroupConv),

    R5('fcvt.w.b', 'R2F1','1100011 00000 ----- --- ----- 1010011', group=fpuGroupConv),
    R5('fcvt.wu.b','R2F1','1100011 00001 ----- --- ----- 1010011', group=fpuGroupConv),

    R5('fmv.x.b',   'R3F','1110011 00000 ----- 000 ----- 1010011', group=fpuGroupOther),

    R5('feq.b',    'RF2', '1010011 ----- ----- 010 ----- 1010011', group=fpuGroupOther),
    R5('flt.b',    'RF2', '1010011 ----- ----- 001 ----- 1010011', group=fpuGroupOther),
    R5('fle.b',    'RF2', '1010011 ----- ----- 000 ----- 1010011', group=fpuGroupOther),

    R5('fclass.b',  'R3F','1110011 00000 ----- 001 ----- 1010011', group=fpuGroupOther),

    R5('fcvt.b.w', 'R2F2','1101011 00000 ----- --- ----- 1010011', group=fpuGroupConv),
    R5('fcvt.b.wu','R2F2','1101011 00001 ----- --- ----- 1010011', group=fpuGroupConv),

    R5('fmv.b.x',  'R3F2','1111011 00000 ----- 000 ----- 1010011', group=fpuGroupOther),

    R5('fcvt.s.b', 'R2F2','0100000 00011 ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fcvt.b.s', 'R2F2','0100011 00000 ----- --- ----- 1010011', group=fpuGroupConv),

    R5('fcvt.h.b', 'R2F2','0100010 00011 ----- 000 ----- 1010011', group=fpuGroupConv),
    R5('fcvt.b.h', 'R2F2','0100011 00010 ----- --- ----- 1010011', group=fpuGroupConv),

    R5('fcvt.ah.b','R2F2','0100010 00011 ----- 101 ----- 1010011', group=fpuGroupConv),
    R5('fcvt.b.ah','R2F2','0100011 00110 ----- --- ----- 1010011', group=fpuGroupConv),
]




#
# Privileged ISA
#
priv = [

    R5('csrrw', 'IU',  '------- ----- ----- 001 ----- 1110011'),
    R5('csrrs', 'IU',  '------- ----- ----- 010 ----- 1110011'),
    R5('csrrc', 'IU',  '------- ----- ----- 011 ----- 1110011'),
    R5('csrrwi','IUR', '------- ----- ----- 101 ----- 1110011'),
    R5('csrrsi','IUR', '------- ----- ----- 110 ----- 1110011'),
    R5('csrrci','IUR', '------- ----- ----- 111 ----- 1110011'),

]


priv_pulp_v2 = [

    #R5('uret',      'Z',   '0000000 00010 00000 000 00000 1110011'),
    #R5('sret',      'Z',   '0001000 00010 00000 000 00000 1110011'),
    #R5('hret',      'Z',   '0010000 00010 00000 000 00000 1110011'),
    R5('mret',      'Z',   '0011000 00010 00000 000 00000 1110011'),
    R5('wfi',       'Z',   '0001000 00101 00000 000 00000 1110011'),

]

priv_pulp = [
 
    R5('eret',  'Z',   '0001000 00000 00000 000 00000 1110011'),
    R5('wfi',   'Z',   '0001000 00010 00000 000 00000 1110011'),


]


priv_1_9 = [

    #R5('uret',      'Z',   '0000000 00010 00000 000 00000 1110011'),
    #R5('sret',      'Z',   '0001000 00010 00000 000 00000 1110011'),
    #R5('hret',      'Z',   '0010000 00010 00000 000 00000 1110011'),
    #R5('mret',      'Z',   '0011000 00010 00000 000 00000 1110011'),
    #R5('sfence.vm', 'F',   '0001000 00100 ----- 000 00000 1110011'),
    R5('wfi',       'Z',   '0001000 00101 00000 000 00000 1110011'),

]



#
# RV32C
#
rv32c = [

    # Compressed ISA
    R5('c.addi4spn', 'CIW', '000 --- --- -- --- 00', fast_handler=True),
    R5('c.lw',       'CL',  '010 --- --- -- --- 00', fast_handler=True, tags=["load"]),
    R5('c.sw',       'CS',  '110 --- --- -- --- 00', fast_handler=True),
    R5('c.nop',      'CI1', '000 000 000 00 000 01', fast_handler=True),
    R5('c.addi',     'CI1', '000 --- --- -- --- 01', fast_handler=True),
    R5('c.jal',      'CJ1', '001 --- --- -- --- 01', fast_handler=True, decode='jal_decode'),
    R5('c.li',       'CI6', '010 --- --- -- --- 01', fast_handler=True),
    R5('c.addi16sp', 'CI4', '011 -00 010 -- --- 01', fast_handler=True),
    R5('c.lui',      'CI5', '011 --- --- -- --- 01', fast_handler=True),
    R5('c.srli',     'CB2', '100 -00 --- -- --- 01', fast_handler=True),
    R5('c.srai',     'CB2', '100 -01 --- -- --- 01', fast_handler=True),
    R5('c.andi',     'CB2S','100 -10 --- -- --- 01', fast_handler=True),
    R5('c.sub',      'CS2', '100 011 --- 00 --- 01', fast_handler=True),
    R5('c.xor',      'CS2', '100 011 --- 01 --- 01', fast_handler=True),
    R5('c.or',       'CS2', '100 011 --- 10 --- 01', fast_handler=True),
    R5('c.and',      'CS2', '100 011 --- 11 --- 01', fast_handler=True),
    R5('c.j',        'CJ',  '101 --- --- -- --- 01', fast_handler=True, decode='jal_decode'),
    R5('c.beqz',     'CB1', '110 --- --- -- --- 01', fast_handler=True, decode='bxx_decode'),
    R5('c.bnez',     'CB1', '111 --- --- -- --- 01', fast_handler=True, decode='bxx_decode'),
    R5('c.slli',     'CI1U','000 --- --- -- --- 10', fast_handler=True),
    R5('c.lwsp',     'CI3', '010 --- --- -- --- 10', fast_handler=True, tags=["load"]),
    R5('c.jr',       'CR1', '100 0-- --- 00 000 10', fast_handler=True),
    R5('c.mv',       'CR2', '100 0-- --- -- --- 10', fast_handler=True),
    R5('c.ebreak',   'CR',  '100 100 000 00 000 10'),
    R5('c.jalr',     'CR3', '100 1-- --- 00 000 10', fast_handler=True),
    R5('c.add',      'CR',  '100 1-- --- -- --- 10', fast_handler=True),
    R5('c.swsp',     'CSS', '110 --- --- -- --- 10', fast_handler=True),
    R5('c.sbreak',   'CI1', '100 000 000 00 000 10'),
]



#
# PULP extensions
#
pulp = [

    # Reg-reg LD/ST
    R5('LB_RR',    'LR', '0000000 ----- ----- 111 ----- 0000011', L='p.lb' , fast_handler=True, tags=["load"]),
    R5('LH_RR',    'LR', '0001000 ----- ----- 111 ----- 0000011', L='p.lh' , fast_handler=True, tags=["load"]),
    R5('LW_RR',    'LR', '0010000 ----- ----- 111 ----- 0000011', L='p.lw' , fast_handler=True, tags=["load"]),
    R5('LBU_RR',   'LR', '0100000 ----- ----- 111 ----- 0000011', L='p.lbu', fast_handler=True, tags=["load"]),
    R5('LHU_RR',   'LR', '0101000 ----- ----- 111 ----- 0000011', L='p.lhu', fast_handler=True, tags=["load"]),    

    # Regular post-inc LD/ST
    R5('LB_POSTINC',    'LPOST', '------- ----- ----- 000 ----- 0001011', L='p.lb' , fast_handler=True, tags=["load"]),
    R5('LH_POSTINC',    'LPOST', '------- ----- ----- 001 ----- 0001011', L='p.lh' , fast_handler=True, tags=["load"]),
    R5('LW_POSTINC',    'LPOST', '------- ----- ----- 010 ----- 0001011', L='p.lw' , fast_handler=True, tags=["load"]),
    R5('LBU_POSTINC',   'LPOST', '------- ----- ----- 100 ----- 0001011', L='p.lbu', fast_handler=True, tags=["load"]),
    R5('LHU_POSTINC',   'LPOST', '------- ----- ----- 101 ----- 0001011', L='p.lhu', fast_handler=True, tags=["load"]), 
    R5('SB_POSTINC',    'SPOST', '------- ----- ----- 000 ----- 0101011', L='p.sb' , fast_handler=True),
    R5('SH_POSTINC',    'SPOST', '------- ----- ----- 001 ----- 0101011', L='p.sh' , fast_handler=True),
    R5('SW_POSTINC',    'SPOST', '------- ----- ----- 010 ----- 0101011', L='p.sw' , fast_handler=True),

    # Reg-reg post-inc LD/ST
    R5('LB_RR_POSTINC',   'LRPOST',  '0000000 ----- ----- 111 ----- 0001011', L='p.lb' , fast_handler=True, tags=["load"]),
    R5('LH_RR_POSTINC',   'LRPOST',  '0001000 ----- ----- 111 ----- 0001011', L='p.lh' , fast_handler=True, tags=["load"]),
    R5('LW_RR_POSTINC',   'LRPOST',  '0010000 ----- ----- 111 ----- 0001011', L='p.lw' , fast_handler=True, tags=["load"]),
    R5('LBU_RR_POSTINC',  'LRPOST',  '0100000 ----- ----- 111 ----- 0001011', L='p.lbu', fast_handler=True, tags=["load"]),
    R5('LHU_RR_POSTINC',  'LRPOST',  '0101000 ----- ----- 111 ----- 0001011', L='p.lhu', fast_handler=True, tags=["load"]),
    R5('SB_RR_POSTINC',   'SRPOST', '0000000 ----- ----- 100 ----- 0101011',  L='p.sb' , fast_handler=True),
    R5('SH_RR_POSTINC',   'SRPOST', '0000000 ----- ----- 101 ----- 0101011',  L='p.sh' , fast_handler=True),
    R5('SW_RR_POSTINC',   'SRPOST', '0000000 ----- ----- 110 ----- 0101011',  L='p.sw' , fast_handler=True),    

    # Additional ALU operations
    R5('p.avgu','R',  '0000010 ----- ----- 001 ----- 0110011'),
    R5('p.slet','R',  '0000010 ----- ----- 010 ----- 0110011'),
    R5('p.sletu','R', '0000010 ----- ----- 011 ----- 0110011'),
    R5('p.min', 'R',  '0000010 ----- ----- 100 ----- 0110011'),
    R5('p.minu','R',  '0000010 ----- ----- 101 ----- 0110011'),
    R5('p.max', 'R',  '0000010 ----- ----- 110 ----- 0110011'),
    R5('p.maxu','R',  '0000010 ----- ----- 111 ----- 0110011'),
    R5('p.ror', 'R',  '0000100 ----- ----- 101 ----- 0110011'),
    R5('p.ff1', 'R1',  '0001000 00000 ----- 000 ----- 0110011'),
    R5('p.fl1','R1',   '0001000 00000 ----- 001 ----- 0110011'),
    R5('p.clb', 'R1',  '0001000 00000 ----- 010 ----- 0110011'),
    R5('p.cnt', 'R1',  '0001000 00000 ----- 011 ----- 0110011'),
    R5('p.exths','R1', '0001000 00000 ----- 100 ----- 0110011'),
    R5('p.exthz','R1', '0001000 00000 ----- 101 ----- 0110011'),
    R5('p.extbs','R1', '0001000 00000 ----- 110 ----- 0110011'),
    R5('p.extbz','R1', '0001000 00000 ----- 111 ----- 0110011'),

    # HW loops
    R5('lp.starti','HL0','------- ----- ----- 000 0000- 1111011'),
    R5('lp.endi',  'HL0','------- ----- ----- 001 0000- 1111011'),
    R5('lp.count', 'HL0','------- ----- ----- 010 0000- 1111011'),
    R5('lp.counti','HL0','------- ----- ----- 011 0000- 1111011'),
    R5('lp.setup', 'HL0','------- ----- ----- 100 0000- 1111011'),
    R5('lp.setupi','HL1','------- ----- ----- 101 0000- 1111011'),
    
]

pulp_v1 = [
    R5('p.abs',  'R1', '0001010 00000 ----- 000 ----- 0110011', mapTo="lib_ABS"),

    R5('SB_RR',  'SR_OLD', '------- ----- ----- 100 ----- 0100011', L='p.sb',  mapTo="lib_SB_RR"),
    R5('SH_RR',  'SR_OLD', '------- ----- ----- 101 ----- 0100011', L='p.sh',  mapTo="lib_SH_RR"),
    R5('SW_RR',  'SR_OLD', '------- ----- ----- 110 ----- 0100011', L='p.sw',  mapTo="lib_SW_RR"),

    # MAC
    R5('p.mac',        'RR','00----- ----- ----- 000 ----- 1011011'),

    R5('p.mac.sl.sl',  'RR','11----- ----- ----- 100 ----- 1011011', mapTo="lib_MAC_SL_SL"),
    R5('p.mac.sl.sh',  'RR','11----- ----- ----- 101 ----- 1011011', mapTo="lib_MAC_SL_SH"),
    R5('p.mac.sl.zl',  'RR','10----- ----- ----- 100 ----- 1011011', mapTo="lib_MAC_SL_ZL"),
    R5('p.mac.sl.zh',  'RR','10----- ----- ----- 101 ----- 1011011', mapTo="lib_MAC_SL_ZH"),
    R5('p.mac.sh.sl',  'RR','11----- ----- ----- 110 ----- 1011011', mapTo="lib_MAC_SH_SL"),
    R5('p.mac.sh.sh',  'RR','11----- ----- ----- 111 ----- 1011011', mapTo="lib_MAC_SH_SH"),
    R5('p.mac.sh.zl',  'RR','10----- ----- ----- 110 ----- 1011011', mapTo="lib_MAC_SH_ZL"),
    R5('p.mac.sh.zh',  'RR','10----- ----- ----- 111 ----- 1011011', mapTo="lib_MAC_SH_ZH"),
    R5('p.mac.zl.sl',  'RR','01----- ----- ----- 100 ----- 1011011', mapTo="lib_MAC_ZL_SL"),
    R5('p.mac.zl.sh',  'RR','01----- ----- ----- 101 ----- 1011011', mapTo="lib_MAC_ZL_SH"),
    R5('p.mac.zl.zl',  'RR','00----- ----- ----- 100 ----- 1011011', mapTo="lib_MAC_ZL_ZL"),
    R5('p.mac.zl.zh',  'RR','00----- ----- ----- 101 ----- 1011011', mapTo="lib_MAC_ZL_ZH"),
    R5('p.mac.zh.sl',  'RR','01----- ----- ----- 110 ----- 1011011', mapTo="lib_MAC_ZH_SL"),
    R5('p.mac.zh.sh',  'RR','01----- ----- ----- 111 ----- 1011011', mapTo="lib_MAC_ZH_SH"),
    R5('p.mac.zh.zl',  'RR','00----- ----- ----- 110 ----- 1011011', mapTo="lib_MAC_ZH_ZL"),
    R5('p.mac.zh.zh',  'RR','00----- ----- ----- 111 ----- 1011011', mapTo="lib_MAC_ZH_ZH"),
]


pulp_zeroriscy = [
    R5('p.elw',           'L',   '------- ----- ----- 110 ----- 0000011', mapTo="lib_LW", tags=["load"]),
]

pulp_v2 = [

    R5('p.abs',  'R1', '0000010 00000 ----- 000 ----- 0110011'),

    R5('SB_RR',    'SR', '0000000 ----- ----- 100 ----- 0100011', L='p.sb', fast_handler=True),
    R5('SH_RR',    'SR', '0000000 ----- ----- 101 ----- 0100011', L='p.sh', fast_handler=True),
    R5('SW_RR',    'SR', '0000000 ----- ----- 110 ----- 0100011', L='p.sw', fast_handler=True),

    R5('p.elw',           'L',   '------- ----- ----- 110 ----- 0000011', tags=["load"]),

    R5('pv.add.h',        'R',   '000000- ----- ----- 000 ----- 1010111'),
    R5('pv.add.sc.h',     'R',   '000000- ----- ----- 100 ----- 1010111'),
    R5('pv.add.sci.h',    'RRS', '000000- ----- ----- 110 ----- 1010111'),
    R5('pv.add.b',        'R',   '000000- ----- ----- 001 ----- 1010111'),
    R5('pv.add.sc.b',     'R',   '000000- ----- ----- 101 ----- 1010111'),
    R5('pv.add.sci.b',    'RRS', '000000- ----- ----- 111 ----- 1010111'),

    R5('pv.sub.h',        'R',   '000010- ----- ----- 000 ----- 1010111'),
    R5('pv.sub.sc.h',     'R',   '000010- ----- ----- 100 ----- 1010111'),
    R5('pv.sub.sci.h',    'RRS', '000010- ----- ----- 110 ----- 1010111'),
    R5('pv.sub.b',        'R',   '000010- ----- ----- 001 ----- 1010111'),
    R5('pv.sub.sc.b',     'R',   '000010- ----- ----- 101 ----- 1010111'),
    R5('pv.sub.sci.b',    'RRS', '000010- ----- ----- 111 ----- 1010111'),

    R5('pv.avg.h',        'R',   '000100- ----- ----- 000 ----- 1010111'),
    R5('pv.avg.sc.h',     'R',   '000100- ----- ----- 100 ----- 1010111'),
    R5('pv.avg.sci.h',    'RRS', '000100- ----- ----- 110 ----- 1010111'),
    R5('pv.avg.b',        'R',   '000100- ----- ----- 001 ----- 1010111'),
    R5('pv.avg.sc.b',     'R',   '000100- ----- ----- 101 ----- 1010111'),
    R5('pv.avg.sci.b',    'RRS', '000100- ----- ----- 111 ----- 1010111'),

    R5('pv.avgu.h',       'R',   '000110- ----- ----- 000 ----- 1010111'),
    R5('pv.avgu.sc.h',    'R',   '000110- ----- ----- 100 ----- 1010111'),
    R5('pv.avgu.sci.h',   'RRU', '000110- ----- ----- 110 ----- 1010111'),
    R5('pv.avgu.b',       'R',   '000110- ----- ----- 001 ----- 1010111'),
    R5('pv.avgu.sc.b',    'R',   '000110- ----- ----- 101 ----- 1010111'),
    R5('pv.avgu.sci.b',   'RRU', '000110- ----- ----- 111 ----- 1010111'),

    R5('pv.min.h',        'R',   '001000- ----- ----- 000 ----- 1010111'),
    R5('pv.min.sc.h',     'R',   '001000- ----- ----- 100 ----- 1010111'),
    R5('pv.min.sci.h',    'RRS', '001000- ----- ----- 110 ----- 1010111'),
    R5('pv.min.b',        'R',   '001000- ----- ----- 001 ----- 1010111'),
    R5('pv.min.sc.b',     'R',   '001000- ----- ----- 101 ----- 1010111'),
    R5('pv.min.sci.b',    'RRS', '001000- ----- ----- 111 ----- 1010111'),

    R5('pv.minu.h',       'R',   '001010- ----- ----- 000 ----- 1010111'),
    R5('pv.minu.sc.h',    'R',   '001010- ----- ----- 100 ----- 1010111'),
    R5('pv.minu.sci.h',   'RRU', '001010- ----- ----- 110 ----- 1010111'),
    R5('pv.minu.b',       'R',   '001010- ----- ----- 001 ----- 1010111'),
    R5('pv.minu.sc.b',    'R',   '001010- ----- ----- 101 ----- 1010111'),
    R5('pv.minu.sci.b',   'RRU', '001010- ----- ----- 111 ----- 1010111'),

    R5('pv.max.h',        'R',   '001100- ----- ----- 000 ----- 1010111'),
    R5('pv.max.sc.h',     'R',   '001100- ----- ----- 100 ----- 1010111'),
    R5('pv.max.sci.h',    'RRS', '001100- ----- ----- 110 ----- 1010111'),
    R5('pv.max.b',        'R',   '001100- ----- ----- 001 ----- 1010111'),
    R5('pv.max.sc.b',     'R',   '001100- ----- ----- 101 ----- 1010111'),
    R5('pv.max.sci.b',    'RRS', '001100- ----- ----- 111 ----- 1010111'),

    R5('pv.maxu.h',       'R',   '001110- ----- ----- 000 ----- 1010111'),
    R5('pv.maxu.sc.h',    'R',   '001110- ----- ----- 100 ----- 1010111'),
    R5('pv.maxu.sci.h',   'RRU', '001110- ----- ----- 110 ----- 1010111'),
    R5('pv.maxu.b',       'R',   '001110- ----- ----- 001 ----- 1010111'),
    R5('pv.maxu.sc.b',    'R',   '001110- ----- ----- 101 ----- 1010111'),
    R5('pv.maxu.sci.b',   'RRU', '001110- ----- ----- 111 ----- 1010111'),

    R5('pv.srl.h',        'R',   '010000- ----- ----- 000 ----- 1010111'),
    R5('pv.srl.sc.h',     'R',   '010000- ----- ----- 100 ----- 1010111'),
    R5('pv.srl.sci.h',    'RRU', '010000- ----- ----- 110 ----- 1010111'),
    R5('pv.srl.b',        'R',   '010000- ----- ----- 001 ----- 1010111'),
    R5('pv.srl.sc.b',     'R',   '010000- ----- ----- 101 ----- 1010111'),
    R5('pv.srl.sci.b',    'RRU', '010000- ----- ----- 111 ----- 1010111'),

    R5('pv.sra.h',        'R',   '010010- ----- ----- 000 ----- 1010111'),
    R5('pv.sra.sc.h',     'R',   '010010- ----- ----- 100 ----- 1010111'),
    R5('pv.sra.sci.h',    'RRS', '010010- ----- ----- 110 ----- 1010111'),
    R5('pv.sra.b',        'R',   '010010- ----- ----- 001 ----- 1010111'),
    R5('pv.sra.sc.b',     'R',   '010010- ----- ----- 101 ----- 1010111'),
    R5('pv.sra.sci.b',    'RRS', '010010- ----- ----- 111 ----- 1010111'),

    R5('pv.sll.h',        'R',   '010100- ----- ----- 000 ----- 1010111'),
    R5('pv.sll.sc.h',     'R',   '010100- ----- ----- 100 ----- 1010111'),
    R5('pv.sll.sci.h',    'RRU', '010100- ----- ----- 110 ----- 1010111'),
    R5('pv.sll.b',        'R',   '010100- ----- ----- 001 ----- 1010111'),
    R5('pv.sll.sc.b',     'R',   '010100- ----- ----- 101 ----- 1010111'),
    R5('pv.sll.sci.b',    'RRU', '010100- ----- ----- 111 ----- 1010111'),

    R5('pv.or.h',         'R',   '010110- ----- ----- 000 ----- 1010111'),
    R5('pv.or.sc.h',      'R',   '010110- ----- ----- 100 ----- 1010111'),
    R5('pv.or.sci.h',     'RRS', '010110- ----- ----- 110 ----- 1010111'),
    R5('pv.or.b',         'R',   '010110- ----- ----- 001 ----- 1010111'),
    R5('pv.or.sc.b',      'R',   '010110- ----- ----- 101 ----- 1010111'),
    R5('pv.or.sci.b',     'RRS', '010110- ----- ----- 111 ----- 1010111'),

    R5('pv.xor.h',        'R',   '011000- ----- ----- 000 ----- 1010111'),
    R5('pv.xor.sc.h',     'R',   '011000- ----- ----- 100 ----- 1010111'),
    R5('pv.xor.sci.h',    'RRS', '011000- ----- ----- 110 ----- 1010111'),
    R5('pv.xor.b',        'R',   '011000- ----- ----- 001 ----- 1010111'),
    R5('pv.xor.sc.b',     'R',   '011000- ----- ----- 101 ----- 1010111'),
    R5('pv.xor.sci.b',    'RRS', '011000- ----- ----- 111 ----- 1010111'),

    R5('pv.and.h',        'R',   '011010- ----- ----- 000 ----- 1010111'),
    R5('pv.and.sc.h',     'R',   '011010- ----- ----- 100 ----- 1010111'),
    R5('pv.and.sci.h',    'RRS', '011010- ----- ----- 110 ----- 1010111'),
    R5('pv.and.b',        'R',   '011010- ----- ----- 001 ----- 1010111'),
    R5('pv.and.sc.b',     'R',   '011010- ----- ----- 101 ----- 1010111'),
    R5('pv.and.sci.b',    'RRS', '011010- ----- ----- 111 ----- 1010111'),

    R5('pv.abs.h',        'R1',  '0111000 ----- ----- 000 ----- 1010111'),  
    R5('pv.abs.b',        'R1',  '0111000 ----- ----- 001 ----- 1010111'), 

    R5('pv.extract.h',    'RRU', '011110- ----- ----- 110 ----- 1010111'),  
    R5('pv.extract.b',    'RRU', '011110- ----- ----- 111 ----- 1010111'),  
    R5('pv.extractu.h',   'RRU', '100100- ----- ----- 110 ----- 1010111'),
    R5('pv.extractu.b',   'RRU', '100100- ----- ----- 111 ----- 1010111'),  

    R5('pv.insert.h',     'RRRU','101100- ----- ----- 110 ----- 1010111'),
    R5('pv.insert.b',     'RRRU','101100- ----- ----- 111 ----- 1010111'),

    R5('pv.dotsp.h',      'R',   '100110- ----- ----- 000 ----- 1010111'),
    R5('pv.dotsp.h.sc',   'R',   '100110- ----- ----- 100 ----- 1010111'),
    R5('pv.dotsp.h.sci',  'RRS', '100110- ----- ----- 110 ----- 1010111'),

    R5('pv.dotsp.b',      'R',   '100110- ----- ----- 001 ----- 1010111'),
    R5('pv.dotsp.b.sc',   'R',   '100110- ----- ----- 101 ----- 1010111'),
    R5('pv.dotsp.b.sci',  'RRS', '100110- ----- ----- 111 ----- 1010111'),

    R5('pv.dotup.h',      'R',   '100000- ----- ----- 000 ----- 1010111'),
    R5('pv.dotup.h.sc',   'R',   '100000- ----- ----- 100 ----- 1010111'),
    R5('pv.dotup.h.sci',  'RRU', '100000- ----- ----- 110 ----- 1010111'),

    R5('pv.dotup.b',      'R',   '100000- ----- ----- 001 ----- 1010111'),
    R5('pv.dotup.b.sc',   'R',   '100000- ----- ----- 101 ----- 1010111'),
    R5('pv.dotup.b.sci',  'RRU', '100000- ----- ----- 111 ----- 1010111'),

    R5('pv.dotusp.h',     'R',   '100010- ----- ----- 000 ----- 1010111'),
    R5('pv.dotusp.h.sc',  'R',   '100010- ----- ----- 100 ----- 1010111'),
    R5('pv.dotusp.h.sci', 'RRS', '100010- ----- ----- 110 ----- 1010111'),

    R5('pv.dotusp.b',     'R',   '100010- ----- ----- 001 ----- 1010111'),
    R5('pv.dotusp.b.sc',  'R',   '100010- ----- ----- 101 ----- 1010111'),
    R5('pv.dotusp.b.sci', 'RRS', '100010- ----- ----- 111 ----- 1010111'),


    R5('pv.sdotsp.h',     'RRRR','101110- ----- ----- 000 ----- 1010111'),
    R5('pv.sdotsp.h.sc',  'RRRR','101110- ----- ----- 100 ----- 1010111'),
    R5('pv.sdotsp.h.sci', 'RRRS','101110- ----- ----- 110 ----- 1010111'),

    R5('pv.sdotsp.b',     'RRRR','101110- ----- ----- 001 ----- 1010111'),
    R5('pv.sdotsp.b.sc',  'RRRR','101110- ----- ----- 101 ----- 1010111'),
    R5('pv.sdotsp.b.sci', 'RRRS','101110- ----- ----- 111 ----- 1010111'),

    R5('pv.sdotup.h',     'RRRR','101000- ----- ----- 000 ----- 1010111'),
    R5('pv.sdotup.h.sc',  'RRRR','101000- ----- ----- 100 ----- 1010111'),
    R5('pv.sdotup.h.sci', 'RRRU','101000- ----- ----- 110 ----- 1010111'),

    R5('pv.sdotup.b',     'RRRR','101000- ----- ----- 001 ----- 1010111'),
    R5('pv.sdotup.b.sc',  'RRRR','101000- ----- ----- 101 ----- 1010111'),
    R5('pv.sdotup.b.sci', 'RRRU','101000- ----- ----- 111 ----- 1010111'),

    R5('pv.sdotusp.h',    'RRRR','101010- ----- ----- 000 ----- 1010111'),
    R5('pv.sdotusp.h.sc', 'RRRR','101010- ----- ----- 100 ----- 1010111'),
    R5('pv.sdotusp.h.sci','RRRS','101010- ----- ----- 110 ----- 1010111'),

    R5('pv.sdotusp.b',    'RRRR','101010- ----- ----- 001 ----- 1010111'),
    R5('pv.sdotusp.b.sc', 'RRRR','101010- ----- ----- 101 ----- 1010111'),
    R5('pv.sdotusp.b.sci','RRRS','101010- ----- ----- 111 ----- 1010111'),

    R5('pv.shuffle.h',    'R',   '110000- ----- ----- 000 ----- 1010111'),
    R5('pv.shuffle.h.sci','RRU', '110000- ----- ----- 110 ----- 1010111'),

    R5('pv.shuffle.b',    'R',   '110000- ----- ----- 001 ----- 1010111'),
    R5('pv.shufflei0.b.sci','RRU2','110000- ----- ----- 111 ----- 1010111'),
    R5('pv.shufflei1.b.sci','RRU2','111010- ----- ----- 111 ----- 1010111'),
    R5('pv.shufflei2.b.sci','RRU2','111100- ----- ----- 111 ----- 1010111'),
    R5('pv.shufflei3.b.sci','RRU2','111110- ----- ----- 111 ----- 1010111'),

    R5('pv.shuffle2.h',   'RRRR','110010- ----- ----- 000 ----- 1010111'),
    R5('pv.shuffle2.b',   'RRRR','110010- ----- ----- 001 ----- 1010111'),

    R5('pv.pack.h',       'RRRR','110100- ----- ----- 000 ----- 1010111'),
    R5('pv.packhi.b',     'RRRR','110110- ----- ----- 001 ----- 1010111'),
    R5('pv.packlo.b',     'RRRR','111000- ----- ----- 001 ----- 1010111'),

    R5('pv.cmpeq.h',      'R',   '000001- ----- ----- 000 ----- 1010111'),
    R5('pv.cmpeq.sc.h',   'R',   '000001- ----- ----- 100 ----- 1010111'),
    R5('pv.cmpeq.sci.h',  'RRS', '000001- ----- ----- 110 ----- 1010111'),
    R5('pv.cmpeq.b',      'R',   '000001- ----- ----- 001 ----- 1010111'),
    R5('pv.cmpeq.sc.b',   'R',   '000001- ----- ----- 101 ----- 1010111'),
    R5('pv.cmpeq.sci.b',  'RRS', '000001- ----- ----- 111 ----- 1010111'),

    R5('pv.cmpne.h',      'R',   '000011- ----- ----- 000 ----- 1010111'),
    R5('pv.cmpne.sc.h',   'R',   '000011- ----- ----- 100 ----- 1010111'),
    R5('pv.cmpne.sci.h',  'RRS', '000011- ----- ----- 110 ----- 1010111'),
    R5('pv.cmpne.b',      'R',   '000011- ----- ----- 001 ----- 1010111'),
    R5('pv.cmpne.sc.b',   'R',   '000011- ----- ----- 101 ----- 1010111'),
    R5('pv.cmpne.sci.b',  'RRS', '000011- ----- ----- 111 ----- 1010111'),

    R5('pv.cmpgt.h',      'R',   '000101- ----- ----- 000 ----- 1010111'),
    R5('pv.cmpgt.sc.h',   'R',   '000101- ----- ----- 100 ----- 1010111'),
    R5('pv.cmpgt.sci.h',  'RRS', '000101- ----- ----- 110 ----- 1010111'),
    R5('pv.cmpgt.b',      'R',   '000101- ----- ----- 001 ----- 1010111'),
    R5('pv.cmpgt.sc.b',   'R',   '000101- ----- ----- 101 ----- 1010111'),
    R5('pv.cmpgt.sci.b',  'RRS', '000101- ----- ----- 111 ----- 1010111'),

    R5('pv.cmpge.h',      'R',   '000111- ----- ----- 000 ----- 1010111'),
    R5('pv.cmpge.sc.h',   'R',   '000111- ----- ----- 100 ----- 1010111'),
    R5('pv.cmpge.sci.h',  'RRS', '000111- ----- ----- 110 ----- 1010111'),
    R5('pv.cmpge.b',      'R',   '000111- ----- ----- 001 ----- 1010111'),
    R5('pv.cmpge.sc.b',   'R',   '000111- ----- ----- 101 ----- 1010111'),
    R5('pv.cmpge.sci.b',  'RRS', '000111- ----- ----- 111 ----- 1010111'),

    R5('pv.cmplt.h',      'R',   '001001- ----- ----- 000 ----- 1010111'),
    R5('pv.cmplt.sc.h',   'R',   '001001- ----- ----- 100 ----- 1010111'),
    R5('pv.cmplt.sci.h',  'RRS', '001001- ----- ----- 110 ----- 1010111'),
    R5('pv.cmplt.b',      'R',   '001001- ----- ----- 001 ----- 1010111'),
    R5('pv.cmplt.sc.b',   'R',   '001001- ----- ----- 101 ----- 1010111'),
    R5('pv.cmplt.sci.b',  'RRS', '001001- ----- ----- 111 ----- 1010111'),

    R5('pv.cmple.h',      'R',   '001011- ----- ----- 000 ----- 1010111'),
    R5('pv.cmple.sc.h',   'R',   '001011- ----- ----- 100 ----- 1010111'),
    R5('pv.cmple.sci.h',  'RRS', '001011- ----- ----- 110 ----- 1010111'),
    R5('pv.cmple.b',      'R',   '001011- ----- ----- 001 ----- 1010111'),
    R5('pv.cmple.sc.b',   'R',   '001011- ----- ----- 101 ----- 1010111'),
    R5('pv.cmple.sci.b',  'RRS', '001011- ----- ----- 111 ----- 1010111'),

    R5('pv.cmpgtu.h',     'R',   '001101- ----- ----- 000 ----- 1010111'),
    R5('pv.cmpgtu.sc.h',  'R',   '001101- ----- ----- 100 ----- 1010111'),
    R5('pv.cmpgtu.sci.h', 'RRU', '001101- ----- ----- 110 ----- 1010111'),
    R5('pv.cmpgtu.b',     'R',   '001101- ----- ----- 001 ----- 1010111'),
    R5('pv.cmpgtu.sc.b',  'R',   '001101- ----- ----- 101 ----- 1010111'),
    R5('pv.cmpgtu.sci.b', 'RRU', '001101- ----- ----- 111 ----- 1010111'),

    R5('pv.cmpgeu.h',     'R',   '001111- ----- ----- 000 ----- 1010111'),
    R5('pv.cmpgeu.sc.h',  'R',   '001111- ----- ----- 100 ----- 1010111'),
    R5('pv.cmpgeu.sci.h', 'RRU', '001111- ----- ----- 110 ----- 1010111'),
    R5('pv.cmpgeu.b',     'R',   '001111- ----- ----- 001 ----- 1010111'),
    R5('pv.cmpgeu.sc.b',  'R',   '001111- ----- ----- 101 ----- 1010111'),
    R5('pv.cmpgeu.sci.b', 'RRU', '001111- ----- ----- 111 ----- 1010111'),

    R5('pv.cmpltu.h',     'R',   '010001- ----- ----- 000 ----- 1010111'),
    R5('pv.cmpltu.sc.h',  'R',   '010001- ----- ----- 100 ----- 1010111'),
    R5('pv.cmpltu.sci.h', 'RRU', '010001- ----- ----- 110 ----- 1010111'),
    R5('pv.cmpltu.b',     'R',   '010001- ----- ----- 001 ----- 1010111'),
    R5('pv.cmpltu.sc.b',  'R',   '010001- ----- ----- 101 ----- 1010111'),
    R5('pv.cmpltu.sci.b', 'RRU', '010001- ----- ----- 111 ----- 1010111'),

    R5('pv.cmpleu.h',     'R',   '010011- ----- ----- 000 ----- 1010111'),
    R5('pv.cmpleu.sc.h',  'R',   '010011- ----- ----- 100 ----- 1010111'),
    R5('pv.cmpleu.sci.h', 'RRU', '010011- ----- ----- 110 ----- 1010111'),
    R5('pv.cmpleu.b',     'R',   '010011- ----- ----- 001 ----- 1010111'),
    R5('pv.cmpleu.sc.b',  'R',   '010011- ----- ----- 101 ----- 1010111'),
    R5('pv.cmpleu.sci.b', 'RRU', '010011- ----- ----- 111 ----- 1010111'),

    R5('p.beqimm',        'SB2', '------- ----- ----- 010 ----- 1100011', fast_handler=True, decode='bxx_decode'),
    R5('p.bneimm',        'SB2', '------- ----- ----- 011 ----- 1100011', fast_handler=True, decode='bxx_decode'),

    R5('p.mac',           'RRRR', '0100001 ----- ----- 000 ----- 0110011'),
    R5('p.msu',           'RRRR', '0100001 ----- ----- 001 ----- 0110011'),
    R5('p.mul',           'R',    '0000001 ----- ----- 000 ----- 0110011'),

    R5('p.muls',          'R',    '1000000 ----- ----- 000 ----- 1011011'),
    R5('p.mulhhs',        'R',    '1100000 ----- ----- 000 ----- 1011011'),
    R5('p.mulsN',         'RRRU2','10----- ----- ----- 000 ----- 1011011'),
    R5('p.mulhhsN',       'RRRU2','11----- ----- ----- 000 ----- 1011011'),
    R5('p.mulsNR',        'RRRU2','10----- ----- ----- 100 ----- 1011011'),
    R5('p.mulhhsNR',      'RRRU2','11----- ----- ----- 100 ----- 1011011'),

    R5('p.mulu',          'R',    '0000000 ----- ----- 000 ----- 1011011'),
    R5('p.mulhhu',        'R',    '0100000 ----- ----- 000 ----- 1011011'),
    R5('p.muluN',         'RRRU2','00----- ----- ----- 000 ----- 1011011'),
    R5('p.mulhhuN',       'RRRU2','01----- ----- ----- 000 ----- 1011011'),
    R5('p.muluNR',        'RRRU2','00----- ----- ----- 100 ----- 1011011'),
    R5('p.mulhhuNR',      'RRRU2','01----- ----- ----- 100 ----- 1011011'),

    R5('p.macs',          'RRRR', '1000000 ----- ----- 001 ----- 1011011'),
    R5('p.machhs',        'RRRR', '1100000 ----- ----- 001 ----- 1011011'),
    R5('p.macsN',         'RRRRU','10----- ----- ----- 001 ----- 1011011'),
    R5('p.machhsN',       'RRRRU','11----- ----- ----- 001 ----- 1011011'),
    R5('p.macsNR',        'RRRRU','10----- ----- ----- 101 ----- 1011011'),
    R5('p.machhsNR',      'RRRRU','11----- ----- ----- 101 ----- 1011011'),

    R5('p.macu',          'RRRR', '0000000 ----- ----- 001 ----- 1011011'),
    R5('p.machhu',        'RRRR', '0100000 ----- ----- 001 ----- 1011011'),
    R5('p.macuN',         'RRRRU','00----- ----- ----- 001 ----- 1011011'),
    R5('p.machhuN',       'RRRRU','01----- ----- ----- 001 ----- 1011011'),
    R5('p.macuNR',        'RRRRU','00----- ----- ----- 101 ----- 1011011'),
    R5('p.machhuNR',      'RRRRU','01----- ----- ----- 101 ----- 1011011'),

    R5('p.addNi',         'RRRU2','00----- ----- ----- 010 ----- 1011011'),
    R5('p.adduNi',        'RRRU2','10----- ----- ----- 010 ----- 1011011'),
    R5('p.addRNi',        'RRRU2','00----- ----- ----- 110 ----- 1011011'),
    R5('p.adduRNi',       'RRRU2','10----- ----- ----- 110 ----- 1011011'),

    R5('p.subNi',         'RRRU2','00----- ----- ----- 011 ----- 1011011'),
    R5('p.subuNi',        'RRRU2','10----- ----- ----- 011 ----- 1011011'),
    R5('p.subRNi',        'RRRU2','00----- ----- ----- 111 ----- 1011011'),
    R5('p.subuRNi',       'RRRU2','10----- ----- ----- 111 ----- 1011011'),

    R5('p.addN',          'RRRR2',    '0100000 ----- ----- 010 ----- 1011011'),
    R5('p.adduN',         'RRRR2',    '1100000 ----- ----- 010 ----- 1011011'),
    R5('p.addRN',         'RRRR2',    '0100000 ----- ----- 110 ----- 1011011'),
    R5('p.adduRN',        'RRRR2',    '1100000 ----- ----- 110 ----- 1011011'),

    R5('p.subN',          'RRRR2',    '0100000 ----- ----- 011 ----- 1011011'),
    R5('p.subuN',         'RRRR2',    '1100000 ----- ----- 011 ----- 1011011'),
    R5('p.subRN',         'RRRR2',    '0100000 ----- ----- 111 ----- 1011011'),
    R5('p.subuRN',        'RRRR2',    '1100000 ----- ----- 111 ----- 1011011'),

    R5('p.clipi',         'I1U',  '0001010 ----- ----- 001 ----- 0110011'),
    R5('p.clipui',        'I1U',  '0001010 ----- ----- 010 ----- 0110011'),
    R5('p.clip',          'R',    '0001010 ----- ----- 101 ----- 0110011'),
    R5('p.clipu',         'R',    '0001010 ----- ----- 110 ----- 0110011'),

    R5('p.extracti',      'I4U',  '11----- ----- ----- 000 ----- 0110011'),
    R5('p.extractui',     'I4U',  '11----- ----- ----- 001 ----- 0110011'),
    R5('p.extract',       'R',    '10----- ----- ----- 000 ----- 0110011'),
    R5('p.extractu',      'R',    '10----- ----- ----- 001 ----- 0110011'),
    R5('p.inserti',       'I5U',  '11----- ----- ----- 010 ----- 0110011'),
    R5('p.insert',        'I5U2', '10----- ----- ----- 010 ----- 0110011'),
    R5('p.bseti',         'I4U',  '11----- ----- ----- 100 ----- 0110011'),
    R5('p.bclri',         'I4U',  '11----- ----- ----- 011 ----- 0110011'),
    R5('p.bset',          'R',    '10----- ----- ----- 100 ----- 0110011'),
    R5('p.bclr',          'R',    '10----- ----- ----- 011 ----- 0110011'),

]


gap8 = [
    R5('pv.cplxmul.s',      'R',   '010101- ----- ----- 000 ----- 1010111', mapTo="lib_CPLXMULS"),
    R5('pv.cplxmul.s.div2', 'R',   '0101010 ----- ----- 010 ----- 1010111', mapTo="lib_CPLXMULS_DIV2"),
    R5('pv.cplxmul.s.div4', 'R',   '0101011 ----- ----- 010 ----- 1010111', mapTo="lib_CPLXMULS_DIV4"),
    R5('pv.cplxmul.s.sc',   'R',   '010101- ----- ----- 100 ----- 1010111', mapTo="lib_CPLXMULS_SC"),
    R5('pv.cplxmul.s.sci',  'RRS', '010101- ----- ----- 110 ----- 1010111', mapTo="lib_CPLXMULS_SCI"),

    R5('pv.cplxconj.h',     'R1',  '0101110 00000 ----- 000 ----- 1010111', mapTo="lib_CPLX_CONJ_16"),
    R5('pv.subrotmj.h',     'R',   '011011- ----- ----- 000 ----- 1010111', mapTo="lib_VEC_ADD_16_ROTMJ"),
    R5('pv.subrotmj.h.div2','R',   '0110110 ----- ----- 010 ----- 1010111', mapTo="lib_VEC_ADD_16_ROTMJ_DIV2"),
    R5('pv.subrotmj.h.div4','R',   '0110111 ----- ----- 010 ----- 1010111', mapTo="lib_VEC_ADD_16_ROTMJ_DIV4"),

    R5('pv.add.h.div2',     'R',   '0000000 ----- ----- 010 ----- 1010111', mapTo="lib_VEC_ADD_16_DIV2"),
    R5('pv.add.h.div4',     'R',   '0000001 ----- ----- 010 ----- 1010111', mapTo="lib_VEC_ADD_16_DIV4"),

    R5('pv.sub.h.div2',     'R',   '0000100 ----- ----- 010 ----- 1010111', mapTo="lib_VEC_SUB_16_DIV2"),
    R5('pv.sub.h.div4',     'R',   '0000101 ----- ----- 010 ----- 1010111', mapTo="lib_VEC_SUB_16_DIV4"),

    R5('pv.add.b.div2',     'R',   '0000000 ----- ----- 011 ----- 1010111', mapTo="lib_VEC_ADD_8_DIV2"),
    R5('pv.add.b.div4',     'R',   '0000001 ----- ----- 011 ----- 1010111', mapTo="lib_VEC_ADD_8_DIV4"),

    R5('pv.sub.b.div2',     'R',   '0000100 ----- ----- 011 ----- 1010111', mapTo="lib_VEC_SUB_8_DIV2"),
    R5('pv.sub.b.div4',     'R',   '0000101 ----- ----- 011 ----- 1010111', mapTo="lib_VEC_SUB_8_DIV4"),

    R5('pv.vitop.max',      'R',   '011001- ----- ----- 001 ----- 1010111', mapTo="lib_VITOP_MAX"),
    R5('pv.vitop.sel',      'R',   '011001- ----- ----- 000 ----- 1010111', mapTo="lib_VITOP_SEL"),

    R5('pv.pack.h.h',       'R',   '110100- ----- ----- 110 ----- 1010111', mapTo="lib_VEC_PACK_SC_H_16"),
    R5('pv.pack.h.l',       'R',   '110100- ----- ----- 100 ----- 1010111', mapTo="lib_VEC_PACK_SC_HL_16"),
]

parser = argparse.ArgumentParser(description='Generate ISA for RISCV')

parser.add_argument("--version", dest="version", default=1, type=int, metavar="VALUE", help="Specify ISA version")
parser.add_argument("--header-file", dest="header_file", default=None, metavar="PATH", help="Specify header output file")
parser.add_argument("--source-file", dest="source_file", default=None, metavar="PATH", help="Specify source output file")

args = parser.parse_args()
        
commonOptions = ["--pulp-perf-counters", "--pulp-hw-loop", "--itc-internal", "--itc-external-req", "--itc-external-wire", "--is-secured"]

#if args.version == 1:
#    isa = Isa('riscv', [IsaSubset('rv32i', rv32i),
#                        IsaSubset('rv32m', rv32m),
#                        IsaSubset('rv32c', rv32c),
#                        IsaSubset('priv', priv),
#                        IsaSubset('priv_pulp', priv_pulp, file=None, active='--priv_pulp'),
#                        IsaSubset('priv_1_9', priv_1_9, file=None, active='--priv_1_9'),
#                        IsaSubset('pulp', pulp),
#                        IsaSubset('fpu', rv32f, active='--fpu', file=None),
#                        IsaSubset('fpud', rv32d, active='--fpud', file=None),
#                        IsaSubset('pulp_v1', pulp_v1, file=None)
#                        ], options=commonOptions, power='pe.instr')
#    isaName = 'riscv'


isa = Isa(
    'riscv',
    [
        IsaSubset('priv_pulp_v2', priv_pulp_v2),
        IsaSubset('i', rv32i),
        IsaSubset('rv32m', rv32m),
        #IsaSubset('rv32a', rv32a),
        IsaSubset('c', rv32c),
        IsaSubset('priv', priv),
        IsaSubset('pulp', pulp),
        IsaSubset('pulp_v2', pulp_v2),
        #IsaSubset('pulp_zeroriscy', pulp_zeroriscy),
        IsaSubset('fpu', rv32f),
        IsaSubset('f16', rv32Xf16),
        IsaSubset('f16alt', rv32Xf16alt),
        IsaSubset('f8', rv32Xf8),
        #IsaSubset('fpud', rv32d),
        #IsaSubset('gap8', gap8),
        #IsaSubset('priv_pulp_v2', priv_pulp_v2),
        #IsaSubset('priv_1_9', priv_1_9)
    ]
)

try:
    os.makedirs(os.path.dirname(args.header_file))
except Exception:
    pass

try:
    os.makedirs(os.path.dirname(args.source_file))
except Exception:
    pass

with open(args.header_file, 'w') as isaFileHeader:
    with open(args.source_file, 'w') as isaFile:

        for insn in isa.get_insns():
            if "load" in insn.tags:
                insn.get_out_reg(0).set_latency(2)

        isa.gen(isaFile, isaFileHeader)
