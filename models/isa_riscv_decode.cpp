/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 */
#ifndef __SWIG__
#include "riscv/riscv.hpp"
#endif
#include "isa_riscv_decode.hpp"

#include "riscv/rv32i.hpp"
#include "riscv/rv32m.hpp"
#include "riscv/rv32c.hpp"
#include "riscv/priv.hpp"
#include "riscv/pulp.hpp"

#ifndef __SWIG__
#include "lib_exec.hpp"
#endif

char *instrNames[] = {
  (char *)"lui",
  (char *)"auipc",
  (char *)"jal",
  (char *)"jalr",
  (char *)"beq",
  (char *)"bne",
  (char *)"blt",
  (char *)"bge",
  (char *)"bltu",
  (char *)"bgeu",
  (char *)"lb",
  (char *)"lh",
  (char *)"lw",
  (char *)"lbu",
  (char *)"lhu",
  (char *)"sb",
  (char *)"sh",
  (char *)"sw",
  (char *)"addi",
  (char *)"nop",
  (char *)"slti",
  (char *)"sltiu",
  (char *)"xori",
  (char *)"ori",
  (char *)"andi",
  (char *)"slli",
  (char *)"srli",
  (char *)"srai",
  (char *)"add",
  (char *)"sub",
  (char *)"sll",
  (char *)"slt",
  (char *)"sltu",
  (char *)"xor",
  (char *)"srl",
  (char *)"sra",
  (char *)"or",
  (char *)"and",
  (char *)"fence",
  (char *)"fence.i",
  (char *)"ecall",
  (char *)"scallimm",
  (char *)"sbreak",
  (char *)"mul",
  (char *)"mulh",
  (char *)"mulhsu",
  (char *)"mulhu",
  (char *)"div",
  (char *)"divu",
  (char *)"rem",
  (char *)"remu",
  (char *)"lr.w",
  (char *)"sc.w",
  (char *)"fld",
  (char *)"fsd",
  (char *)"c.fsd",
  (char *)"c.fsdsp",
  (char *)"c.fld",
  (char *)"c.flwsp",
  (char *)"flw",
  (char *)"fsw",
  (char *)"fmadd.s",
  (char *)"fmsub.s",
  (char *)"fadd.s",
  (char *)"fsub.s",
  (char *)"fmul.s",
  (char *)"fdiv.s",
  (char *)"fsqrt.s",
  (char *)"fsgnj.s",
  (char *)"fsgnjn.s",
  (char *)"fsgnjx.s",
  (char *)"fmin.s",
  (char *)"fmax.s",
  (char *)"fcvt.w.s",
  (char *)"fcvt.wu.s",
  (char *)"fmv.x.s",
  (char *)"feq.s",
  (char *)"flt.s",
  (char *)"fle.s",
  (char *)"fclass.s",
  (char *)"fcvt.s.w",
  (char *)"fcvt.s.wu",
  (char *)"fmv.s.x",
  (char *)"fcvt.s.d",
  (char *)"fcvt.d.s",
  (char *)"fsgnj.d",
  (char *)"c.fsw",
  (char *)"c.fswsp",
  (char *)"c.flw",
  (char *)"csrrw",
  (char *)"csrrs",
  (char *)"csrrc",
  (char *)"csrrwi",
  (char *)"csrrsi",
  (char *)"csrrci",
  (char *)"uret",
  (char *)"sret",
  (char *)"hret",
  (char *)"mret",
  (char *)"wfi",
  (char *)"eret",
  (char *)"sfence.vm",
  (char *)"c.addi4spn",
  (char *)"c.lw",
  (char *)"c.sw",
  (char *)"c.nop",
  (char *)"c.addi",
  (char *)"c.jal",
  (char *)"c.li",
  (char *)"c.addi16sp",
  (char *)"c.lui",
  (char *)"c.srli",
  (char *)"c.srai",
  (char *)"c.andi",
  (char *)"c.sub",
  (char *)"c.xor",
  (char *)"c.or",
  (char *)"c.and",
  (char *)"c.j",
  (char *)"c.beqz",
  (char *)"c.bnez",
  (char *)"c.slli",
  (char *)"c.lwsp",
  (char *)"c.jr",
  (char *)"c.mv",
  (char *)"c.ebreak",
  (char *)"c.jalr",
  (char *)"c.add",
  (char *)"c.swsp",
  (char *)"c.sbreak",
  (char *)"p.lb",
  (char *)"p.lh",
  (char *)"p.lw",
  (char *)"p.lbu",
  (char *)"p.lhu",
  (char *)"p.sb",
  (char *)"p.sh",
  (char *)"p.sw",
  (char *)"p.avgu",
  (char *)"p.slet",
  (char *)"p.sletu",
  (char *)"p.min",
  (char *)"p.minu",
  (char *)"p.max",
  (char *)"p.maxu",
  (char *)"p.ror",
  (char *)"p.ff1",
  (char *)"p.fl1",
  (char *)"p.clb",
  (char *)"p.cnt",
  (char *)"p.exths",
  (char *)"p.exthz",
  (char *)"p.extbs",
  (char *)"p.extbz",
  (char *)"lp.starti",
  (char *)"lp.endi",
  (char *)"lp.count",
  (char *)"lp.counti",
  (char *)"lp.setup",
  (char *)"lp.setupi",
  (char *)"p.abs",
  (char *)"p.mac",
  (char *)"p.mac.sl.sl",
  (char *)"p.mac.sl.sh",
  (char *)"p.mac.sl.zl",
  (char *)"p.mac.sl.zh",
  (char *)"p.mac.sh.sl",
  (char *)"p.mac.sh.sh",
  (char *)"p.mac.sh.zl",
  (char *)"p.mac.sh.zh",
  (char *)"p.mac.zl.sl",
  (char *)"p.mac.zl.sh",
  (char *)"p.mac.zl.zl",
  (char *)"p.mac.zl.zh",
  (char *)"p.mac.zh.sl",
  (char *)"p.mac.zh.sh",
  (char *)"p.mac.zh.zl",
  (char *)"p.mac.zh.zh",
  (char *)"p.elw",
  (char *)"pv.add.h",
  (char *)"pv.add.sc.h",
  (char *)"pv.add.sci.h",
  (char *)"pv.add.b",
  (char *)"pv.add.sc.b",
  (char *)"pv.add.sci.b",
  (char *)"pv.sub.h",
  (char *)"pv.sub.sc.h",
  (char *)"pv.sub.sci.h",
  (char *)"pv.sub.b",
  (char *)"pv.sub.sc.b",
  (char *)"pv.sub.sci.b",
  (char *)"pv.avg.h",
  (char *)"pv.avg.sc.h",
  (char *)"pv.avg.sci.h",
  (char *)"pv.avg.b",
  (char *)"pv.avg.sc.b",
  (char *)"pv.avg.sci.b",
  (char *)"pv.avgu.h",
  (char *)"pv.avgu.sc.h",
  (char *)"pv.avgu.sci.h",
  (char *)"pv.avgu.b",
  (char *)"pv.avgu.sc.b",
  (char *)"pv.avgu.sci.b",
  (char *)"pv.min.h",
  (char *)"pv.min.sc.h",
  (char *)"pv.min.sci.h",
  (char *)"pv.min.b",
  (char *)"pv.min.sc.b",
  (char *)"pv.min.sci.b",
  (char *)"pv.minu.h",
  (char *)"pv.minu.sc.h",
  (char *)"pv.minu.sci.h",
  (char *)"pv.minu.b",
  (char *)"pv.minu.sc.b",
  (char *)"pv.minu.sci.b",
  (char *)"pv.max.h",
  (char *)"pv.max.sc.h",
  (char *)"pv.max.sci.h",
  (char *)"pv.max.b",
  (char *)"pv.max.sc.b",
  (char *)"pv.max.sci.b",
  (char *)"pv.maxu.h",
  (char *)"pv.maxu.sc.h",
  (char *)"pv.maxu.sci.h",
  (char *)"pv.maxu.b",
  (char *)"pv.maxu.sc.b",
  (char *)"pv.maxu.sci.b",
  (char *)"pv.srl.h",
  (char *)"pv.srl.sc.h",
  (char *)"pv.srl.sci.h",
  (char *)"pv.srl.b",
  (char *)"pv.srl.sc.b",
  (char *)"pv.srl.sci.b",
  (char *)"pv.sra.h",
  (char *)"pv.sra.sc.h",
  (char *)"pv.sra.sci.h",
  (char *)"pv.sra.b",
  (char *)"pv.sra.sc.b",
  (char *)"pv.sra.sci.b",
  (char *)"pv.sll.h",
  (char *)"pv.sll.sc.h",
  (char *)"pv.sll.sci.h",
  (char *)"pv.sll.b",
  (char *)"pv.sll.sc.b",
  (char *)"pv.sll.sci.b",
  (char *)"pv.or.h",
  (char *)"pv.or.sc.h",
  (char *)"pv.or.sci.h",
  (char *)"pv.or.b",
  (char *)"pv.or.sc.b",
  (char *)"pv.or.sci.b",
  (char *)"pv.xor.h",
  (char *)"pv.xor.sc.h",
  (char *)"pv.xor.sci.h",
  (char *)"pv.xor.b",
  (char *)"pv.xor.sc.b",
  (char *)"pv.xor.sci.b",
  (char *)"pv.and.h",
  (char *)"pv.and.sc.h",
  (char *)"pv.and.sci.h",
  (char *)"pv.and.b",
  (char *)"pv.and.sc.b",
  (char *)"pv.and.sci.b",
  (char *)"pv.abs.h",
  (char *)"pv.abs.b",
  (char *)"pv.extract.h",
  (char *)"pv.extract.b",
  (char *)"pv.extractu.h",
  (char *)"pv.extractu.b",
  (char *)"pv.insert.h",
  (char *)"pv.insert.b",
  (char *)"pv.dotsp.h",
  (char *)"pv.dotsp.h.sc",
  (char *)"pv.dotsp.h.sci",
  (char *)"pv.dotsp.b",
  (char *)"pv.dotsp.b.sc",
  (char *)"pv.dotsp.b.sci",
  (char *)"pv.dotup.h",
  (char *)"pv.dotup.h.sc",
  (char *)"pv.dotup.h.sci",
  (char *)"pv.dotup.b",
  (char *)"pv.dotup.b.sc",
  (char *)"pv.dotup.b.sci",
  (char *)"pv.dotusp.h",
  (char *)"pv.dotusp.h.sc",
  (char *)"pv.dotusp.h.sci",
  (char *)"pv.dotusp.b",
  (char *)"pv.dotusp.b.sc",
  (char *)"pv.dotusp.b.sci",
  (char *)"pv.sdotsp.h",
  (char *)"pv.sdotsp.h.sc",
  (char *)"pv.sdotsp.h.sci",
  (char *)"pv.sdotsp.b",
  (char *)"pv.sdotsp.b.sc",
  (char *)"pv.sdotsp.b.sci",
  (char *)"pv.sdotup.h",
  (char *)"pv.sdotup.h.sc",
  (char *)"pv.sdotup.h.sci",
  (char *)"pv.sdotup.b",
  (char *)"pv.sdotup.b.sc",
  (char *)"pv.sdotup.b.sci",
  (char *)"pv.sdotusp.h",
  (char *)"pv.sdotusp.h.sc",
  (char *)"pv.sdotusp.h.sci",
  (char *)"pv.sdotusp.b",
  (char *)"pv.sdotusp.b.sc",
  (char *)"pv.sdotusp.b.sci",
  (char *)"pv.shuffle.h",
  (char *)"pv.shuffle.h.sci",
  (char *)"pv.shuffle.b",
  (char *)"pv.shufflei0.b.sci",
  (char *)"pv.shufflei1.b.sci",
  (char *)"pv.shufflei2.b.sci",
  (char *)"pv.shufflei3.b.sci",
  (char *)"pv.shuffle2.h",
  (char *)"pv.shuffle2.b",
  (char *)"pv.pack.h",
  (char *)"pv.packhi.b",
  (char *)"pv.packlo.b",
  (char *)"pv.cmpeq.h",
  (char *)"pv.cmpeq.sc.h",
  (char *)"pv.cmpeq.sci.h",
  (char *)"pv.cmpeq.b",
  (char *)"pv.cmpeq.sc.b",
  (char *)"pv.cmpeq.sci.b",
  (char *)"pv.cmpne.h",
  (char *)"pv.cmpne.sc.h",
  (char *)"pv.cmpne.sci.h",
  (char *)"pv.cmpne.b",
  (char *)"pv.cmpne.sc.b",
  (char *)"pv.cmpne.sci.b",
  (char *)"pv.cmpgt.h",
  (char *)"pv.cmpgt.sc.h",
  (char *)"pv.cmpgt.sci.h",
  (char *)"pv.cmpgt.b",
  (char *)"pv.cmpgt.sc.b",
  (char *)"pv.cmpgt.sci.b",
  (char *)"pv.cmpge.h",
  (char *)"pv.cmpge.sc.h",
  (char *)"pv.cmpge.sci.h",
  (char *)"pv.cmpge.b",
  (char *)"pv.cmpge.sc.b",
  (char *)"pv.cmpge.sci.b",
  (char *)"pv.cmplt.h",
  (char *)"pv.cmplt.sc.h",
  (char *)"pv.cmplt.sci.h",
  (char *)"pv.cmplt.b",
  (char *)"pv.cmplt.sc.b",
  (char *)"pv.cmplt.sci.b",
  (char *)"pv.cmple.h",
  (char *)"pv.cmple.sc.h",
  (char *)"pv.cmple.sci.h",
  (char *)"pv.cmple.b",
  (char *)"pv.cmple.sc.b",
  (char *)"pv.cmple.sci.b",
  (char *)"pv.cmpgtu.h",
  (char *)"pv.cmpgtu.sc.h",
  (char *)"pv.cmpgtu.sci.h",
  (char *)"pv.cmpgtu.b",
  (char *)"pv.cmpgtu.sc.b",
  (char *)"pv.cmpgtu.sci.b",
  (char *)"pv.cmpgeu.h",
  (char *)"pv.cmpgeu.sc.h",
  (char *)"pv.cmpgeu.sci.h",
  (char *)"pv.cmpgeu.b",
  (char *)"pv.cmpgeu.sc.b",
  (char *)"pv.cmpgeu.sci.b",
  (char *)"pv.cmpltu.h",
  (char *)"pv.cmpltu.sc.h",
  (char *)"pv.cmpltu.sci.h",
  (char *)"pv.cmpltu.b",
  (char *)"pv.cmpltu.sc.b",
  (char *)"pv.cmpltu.sci.b",
  (char *)"pv.cmpleu.h",
  (char *)"pv.cmpleu.sc.h",
  (char *)"pv.cmpleu.sci.h",
  (char *)"pv.cmpleu.b",
  (char *)"pv.cmpleu.sc.b",
  (char *)"pv.cmpleu.sci.b",
  (char *)"p.beqimm",
  (char *)"p.bneimm",
  (char *)"p.msu",
  (char *)"p.mul",
  (char *)"p.muls",
  (char *)"p.mulhhs",
  (char *)"p.mulsN",
  (char *)"p.mulhhsN",
  (char *)"p.mulsNR",
  (char *)"p.mulhhsNR",
  (char *)"p.mulu",
  (char *)"p.mulhhu",
  (char *)"p.muluN",
  (char *)"p.mulhhuN",
  (char *)"p.muluNR",
  (char *)"p.mulhhuNR",
  (char *)"p.macs",
  (char *)"p.machhs",
  (char *)"p.macsN",
  (char *)"p.machhsN",
  (char *)"p.macsNR",
  (char *)"p.machhsNR",
  (char *)"p.macu",
  (char *)"p.machhu",
  (char *)"p.macuN",
  (char *)"p.machhuN",
  (char *)"p.macuNR",
  (char *)"p.machhuNR",
  (char *)"p.addNi",
  (char *)"p.adduNi",
  (char *)"p.addRNi",
  (char *)"p.adduRNi",
  (char *)"p.subNi",
  (char *)"p.subuNi",
  (char *)"p.subRNi",
  (char *)"p.subuRNi",
  (char *)"p.addN",
  (char *)"p.adduN",
  (char *)"p.addRN",
  (char *)"p.adduRN",
  (char *)"p.subN",
  (char *)"p.subuN",
  (char *)"p.subRN",
  (char *)"p.subuRN",
  (char *)"p.clipi",
  (char *)"p.clipui",
  (char *)"p.clip",
  (char *)"p.clipu",
  (char *)"p.extracti",
  (char *)"p.extractui",
  (char *)"p.extract",
  (char *)"p.extractu",
  (char *)"p.inserti",
  (char *)"p.insert",
  (char *)"p.bseti",
  (char *)"p.bclri",
  (char *)"p.bset",
  (char *)"p.bclr",
  (char *)"pv.cplxmul.s",
  (char *)"pv.cplxmul.s.div2",
  (char *)"pv.cplxmul.s.div4",
  (char *)"pv.cplxmul.s.sc",
  (char *)"pv.cplxmul.s.sci",
  (char *)"pv.cplxconj.h",
  (char *)"pv.subrotmj.h",
  (char *)"pv.subrotmj.h.div2",
  (char *)"pv.subrotmj.h.div4",
  (char *)"pv.add.h.div2",
  (char *)"pv.add.h.div4",
  (char *)"pv.sub.h.div2",
  (char *)"pv.sub.h.div4",
  (char *)"pv.add.b.div2",
  (char *)"pv.add.b.div4",
  (char *)"pv.sub.b.div2",
  (char *)"pv.sub.b.div4",
  (char *)"pv.vitop.max",
  (char *)"pv.vitop.sel",
  (char *)"pv.pack.h.h",
  (char *)"pv.pack.h.l",
};

static char *timingTable_fpu_names[] = { (char *)"INSTR_GROUP_OTHER", (char *)"INSTR_GROUP_FPU_ADD", (char *)"INSTR_GROUP_FPU_MUL", (char *)"INSTR_GROUP_FPU_DIV", (char *)"INSTR_GROUP_FPU_FMADD", (char *)"INSTR_GROUP_FPU_CONV", };

static timingTable_t timingTable_fpu = { "fpu", "--fpu-timing", "timing_fpu_private.cfg", 0, 6, timingTable_fpu_names, NULL, NULL };
static char *timingTable_rv32m_names[] = { (char *)"INSTR_GROUP_RV32M_OTHER", (char *)"INSTR_GROUP_RV32M_MUL", (char *)"INSTR_GROUP_RV32M_MULH", (char *)"INSTR_GROUP_RV32M_DIV", };

static timingTable_t timingTable_rv32m = { "rv32m", "--rv32m-timing", "timing_rv32m_ri5cy.cfg", 0, 4, timingTable_rv32m_names, NULL, NULL };

timingTable_t *timeTables[] = {&timingTable_fpu, &timingTable_rv32m, };

isaOption_t isaOptions[] = {
  {(char *)"--pulp-perf-counters", __pulp_perf_counters}, 
  {(char *)"--pulp-hw-loop", __pulp_hw_loop}, 
  {(char *)"--itc-internal", __itc_internal}, 
  {(char *)"--itc-external-req", __itc_external_req}, 
  {(char *)"--itc-external-wire", __itc_external_wire}, 
  {(char *)"--is-secured", __is_secured}, 
  {(char *)"--priv_pulp", __priv_pulp}, 
  {(char *)"--priv_1_9", __priv_1_9}, 
  {(char *)"--fpu", __fpu}, 
  {(char *)"--fpud", __fpud}, 
  {(char *)"--shared-fpu", __shared_fpu}, 
  {NULL, 0}
};

static void lui_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->uim[0] = (getField(pc->value, 12, 20)<<12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 0;
  setHandler(pc, lui_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lui";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void auipc_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->uim[0] = (getField(pc->value, 12, 20)<<12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 1;
  setHandler(pc, auipc_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "auipc";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetUimm(pc, pc->uim[0]);

  AUIPC_decode(cpu, pc);
}

static void jal_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 12, 8)<<12) | (getField(pc->value, 20, 1)<<11) | (getField(pc->value, 21, 10)<<1) | (getField(pc->value, 31, 1)<<20), 21);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 2;
  setHandler(pc, jal_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "jal";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetSimm(pc, pc->sim[0]);

  JAL_decode(cpu, pc);
}

static void jalr_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 3;
  setHandler(pc, jalr_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "jalr";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void beq_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 1)<<11) | (getField(pc->value, 8, 4)<<1) | (getField(pc->value, 25, 6)<<5) | (getField(pc->value, 31, 1)<<12), 13);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 4;
  setHandler(pc, beq_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "beq";
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);

  BXX_decode(cpu, pc);
}

static void bne_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 1)<<11) | (getField(pc->value, 8, 4)<<1) | (getField(pc->value, 25, 6)<<5) | (getField(pc->value, 31, 1)<<12), 13);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 5;
  setHandler(pc, bne_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "bne";
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);

  BXX_decode(cpu, pc);
}

static void blt_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 1)<<11) | (getField(pc->value, 8, 4)<<1) | (getField(pc->value, 25, 6)<<5) | (getField(pc->value, 31, 1)<<12), 13);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 6;
  setHandler(pc, blt_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "blt";
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);

  BXX_decode(cpu, pc);
}

static void bge_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 1)<<11) | (getField(pc->value, 8, 4)<<1) | (getField(pc->value, 25, 6)<<5) | (getField(pc->value, 31, 1)<<12), 13);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 7;
  setHandler(pc, bge_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "bge";
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);

  BXX_decode(cpu, pc);
}

static void bltu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 1)<<11) | (getField(pc->value, 8, 4)<<1) | (getField(pc->value, 25, 6)<<5) | (getField(pc->value, 31, 1)<<12), 13);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 8;
  setHandler(pc, bltu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "bltu";
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);

  BXX_decode(cpu, pc);
}

static void bgeu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 1)<<11) | (getField(pc->value, 8, 4)<<1) | (getField(pc->value, 25, 6)<<5) | (getField(pc->value, 31, 1)<<12), 13);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 9;
  setHandler(pc, bgeu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "bgeu";
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);

  BXX_decode(cpu, pc);
}

static void lb_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 10;
  setHandler(pc, lib_LB_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lb";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void lh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 11;
  setHandler(pc, lib_LH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void lw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 12;
  setHandler(pc, lib_LW_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lw";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void lbu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 13;
  setHandler(pc, lib_LBU_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lbu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void lhu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 14;
  setHandler(pc, lib_LHU_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lhu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void sb_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 5)<<0) | (getField(pc->value, 25, 7)<<5), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 15;
  setHandler(pc, lib_SB_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sb";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void sh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 5)<<0) | (getField(pc->value, 25, 7)<<5), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 16;
  setHandler(pc, lib_SH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sh";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void sw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 5)<<0) | (getField(pc->value, 25, 7)<<5), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 17;
  setHandler(pc, lib_SW_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sw";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void addi_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 18;
  setHandler(pc, lib_ADD_SI_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "addi";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void addi_0_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 19;
  setHandler(pc, lib_ADD_SI_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "nop";
}

static void slti_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 20;
  setHandler(pc, slti_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "slti";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void sltiu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 21;
  setHandler(pc, sltiu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sltiu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void xori_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 22;
  setHandler(pc, lib_XOR_SI_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "xori";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void ori_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 23;
  setHandler(pc, lib_OR_SI_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "ori";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void andi_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 24;
  setHandler(pc, lib_AND_SI_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "andi";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void slli_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 25;
  setHandler(pc, lib_SLL_UI_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "slli";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void srli_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 26;
  setHandler(pc, lib_SRL_UI_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "srli";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void srai_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 27;
  setHandler(pc, lib_SRA_UI_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "srai";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void add_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 28;
  setHandler(pc, lib_ADD_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "add";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void sub_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 29;
  setHandler(pc, lib_SUB_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sub";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void sll_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 30;
  setHandler(pc, lib_SLL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sll";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void slt_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 31;
  setHandler(pc, slt_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "slt";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void sltu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 32;
  setHandler(pc, sltu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sltu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void xor_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 33;
  setHandler(pc, lib_XOR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "xor";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void srl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 34;
  setHandler(pc, lib_SRL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "srl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void sra_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 35;
  setHandler(pc, lib_SRA_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sra";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void or_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 36;
  setHandler(pc, lib_OR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "or";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void and_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 37;
  setHandler(pc, lib_AND_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "and";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void fence_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->uim[0] = (getField(pc->value, 20, 8)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 38;
  setHandler(pc, fence_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fence";
  traceSetUimm(pc, pc->uim[0]);
}

static void fence_i_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 39;
  setHandler(pc, fence_i_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fence.i";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void ecall_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 40;
  setHandler(pc, ecall_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "ecall";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void scallimm_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->uim[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 41;
  setHandler(pc, scallimm_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "scallimm";
  traceSetUimm(pc, pc->uim[0]);
}

static void sbreak_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 42;
  setHandler(pc, sbreak_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sbreak";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void mul_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_RV32M_MUL;
  pc->instrId = 43;
  pc->latency = getTiming_latency(cpu, &timingTable_rv32m, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_rv32m, pc->group);
  setHandler(pc, lib_MULU_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "mul";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void mulh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_RV32M_MULH;
  pc->instrId = 44;
  pc->latency = getTiming_latency(cpu, &timingTable_rv32m, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_rv32m, pc->group);
  setHandler(pc, mulh_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "mulh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void mulhsu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_RV32M_MULH;
  pc->instrId = 45;
  pc->latency = getTiming_latency(cpu, &timingTable_rv32m, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_rv32m, pc->group);
  setHandler(pc, mulhsu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "mulhsu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void mulhu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_RV32M_MULH;
  pc->instrId = 46;
  pc->latency = getTiming_latency(cpu, &timingTable_rv32m, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_rv32m, pc->group);
  setHandler(pc, mulhu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "mulhu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void div_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_RV32M_DIV;
  pc->instrId = 47;
  pc->latency = getTiming_latency(cpu, &timingTable_rv32m, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_rv32m, pc->group);
  setHandler(pc, div_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "div";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void divu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_RV32M_DIV;
  pc->instrId = 48;
  pc->latency = getTiming_latency(cpu, &timingTable_rv32m, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_rv32m, pc->group);
  setHandler(pc, divu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "divu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void rem_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_RV32M_DIV;
  pc->instrId = 49;
  pc->latency = getTiming_latency(cpu, &timingTable_rv32m, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_rv32m, pc->group);
  setHandler(pc, rem_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "rem";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void remu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_RV32M_DIV;
  pc->instrId = 50;
  pc->latency = getTiming_latency(cpu, &timingTable_rv32m, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_rv32m, pc->group);
  setHandler(pc, remu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "remu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void c_addi4spn_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 2, 3)<<0)) + 8;
  pc->inReg[0] = 2;
  pc->sim[0] = (getField(pc->value, 6, 1)<<2) | (getField(pc->value, 5, 1)<<3) | (getField(pc->value, 11, 2)<<4) | (getField(pc->value, 7, 4)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 102;
  setHandler(pc, c_addi4spn_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.addi4spn";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void c_lw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 2, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->sim[0] = (getField(pc->value, 6, 1)<<2) | (getField(pc->value, 10, 3)<<3) | (getField(pc->value, 5, 1)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 103;
  setHandler(pc, c_lw_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.lw";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_sw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = ((getField(pc->value, 2, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->sim[0] = (getField(pc->value, 6, 1)<<2) | (getField(pc->value, 10, 3)<<3) | (getField(pc->value, 5, 1)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 104;
  setHandler(pc, c_sw_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.sw";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_nop_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 2, 5)<<0) | (getField(pc->value, 12, 1)<<5), 6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 105;
  setHandler(pc, c_nop_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.nop";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void c_addi_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 2, 5)<<0) | (getField(pc->value, 12, 1)<<5), 6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 106;
  setHandler(pc, c_addi_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.addi";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void c_jal_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = 1;
  pc->sim[0] = getSignedValue((getField(pc->value, 3, 3)<<1) | (getField(pc->value, 11, 1)<<4) | (getField(pc->value, 2, 1)<<5) | (getField(pc->value, 7, 1)<<6) | (getField(pc->value, 6, 1)<<7) | (getField(pc->value, 9, 2)<<8) | (getField(pc->value, 8, 1)<<10) | (getField(pc->value, 12, 1)<<11), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 107;
  setHandler(pc, c_jal_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.jal";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetSimm(pc, pc->sim[0]);

  JAL_decode(cpu, pc);
}

static void c_li_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = 0;
  pc->sim[0] = getSignedValue((getField(pc->value, 2, 5)<<0) | (getField(pc->value, 12, 1)<<5), 6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 108;
  setHandler(pc, c_li_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.li";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void c_addi16sp_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = 2;
  pc->inReg[0] = 2;
  pc->sim[0] = getSignedValue((getField(pc->value, 6, 1)<<4) | (getField(pc->value, 2, 1)<<5) | (getField(pc->value, 5, 1)<<6) | (getField(pc->value, 3, 2)<<7) | (getField(pc->value, 12, 1)<<9), 10);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 109;
  setHandler(pc, c_addi16sp_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.addi16sp";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void c_lui_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->uim[0] = getSignedValue((getField(pc->value, 2, 5)<<12) | (getField(pc->value, 12, 1)<<17), 18);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 110;
  setHandler(pc, c_lui_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.lui";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void c_srli_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->uim[0] = (getField(pc->value, 2, 5)<<0) | (getField(pc->value, 12, 1)<<5);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 111;
  setHandler(pc, c_srli_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.srli";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void c_srai_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->uim[0] = (getField(pc->value, 2, 5)<<0) | (getField(pc->value, 12, 1)<<5);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 112;
  setHandler(pc, c_srai_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.srai";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void c_andi_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->sim[0] = getSignedValue((getField(pc->value, 2, 5)<<0) | (getField(pc->value, 12, 1)<<5), 6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 113;
  setHandler(pc, c_andi_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.andi";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void c_sub_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[1] = ((getField(pc->value, 2, 3)<<0)) + 8;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 114;
  setHandler(pc, c_sub_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.sub";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void c_xor_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[1] = ((getField(pc->value, 2, 3)<<0)) + 8;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 115;
  setHandler(pc, c_xor_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.xor";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void c_or_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[1] = ((getField(pc->value, 2, 3)<<0)) + 8;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 116;
  setHandler(pc, c_or_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.or";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void c_and_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[1] = ((getField(pc->value, 2, 3)<<0)) + 8;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 117;
  setHandler(pc, c_and_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.and";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void c_j_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = 0;
  pc->sim[0] = getSignedValue((getField(pc->value, 3, 3)<<1) | (getField(pc->value, 11, 1)<<4) | (getField(pc->value, 2, 1)<<5) | (getField(pc->value, 7, 1)<<6) | (getField(pc->value, 6, 1)<<7) | (getField(pc->value, 9, 2)<<8) | (getField(pc->value, 8, 1)<<10) | (getField(pc->value, 12, 1)<<11), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 118;
  setHandler(pc, c_j_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.j";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetSimm(pc, pc->sim[0]);

  JAL_decode(cpu, pc);
}

static void c_beqz_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[1] = 0;
  pc->sim[0] = getSignedValue((getField(pc->value, 3, 2)<<1) | (getField(pc->value, 10, 2)<<3) | (getField(pc->value, 2, 1)<<5) | (getField(pc->value, 5, 2)<<6) | (getField(pc->value, 12, 1)<<8), 9);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 119;
  setHandler(pc, c_beqz_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.beqz";
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);

  BXX_decode(cpu, pc);
}

static void c_bnez_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->inReg[1] = 0;
  pc->sim[0] = getSignedValue((getField(pc->value, 3, 2)<<1) | (getField(pc->value, 10, 2)<<3) | (getField(pc->value, 2, 1)<<5) | (getField(pc->value, 5, 2)<<6) | (getField(pc->value, 12, 1)<<8), 9);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 120;
  setHandler(pc, c_bnez_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.bnez";
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);

  BXX_decode(cpu, pc);
}

static void c_slli_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->uim[0] = (getField(pc->value, 2, 5)<<0) | (getField(pc->value, 12, 1)<<5);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 121;
  setHandler(pc, c_slli_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.slli";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void c_lwsp_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = 2;
  pc->sim[0] = (getField(pc->value, 4, 3)<<2) | (getField(pc->value, 12, 1)<<5) | (getField(pc->value, 2, 2)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 122;
  setHandler(pc, c_lwsp_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.lwsp";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_jr_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = 0;
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[1] = 0;
  pc->sim[0] = 0;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 123;
  setHandler(pc, c_jr_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.jr";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void c_mv_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = 0;
  pc->inReg[1] = (getField(pc->value, 2, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 124;
  setHandler(pc, c_mv_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.mv";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void c_ebreak_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 2, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 125;
  setHandler(pc, c_ebreak_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.ebreak";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void c_jalr_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = 1;
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 2, 5)<<0);
  pc->sim[0] = 0;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 126;
  setHandler(pc, c_jalr_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.jalr";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void c_add_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 2, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 127;
  setHandler(pc, c_add_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.add";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void c_swsp_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 2, 5)<<0);
  pc->inReg[0] = 2;
  pc->sim[0] = (getField(pc->value, 9, 4)<<2) | (getField(pc->value, 7, 2)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 128;
  setHandler(pc, c_swsp_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.swsp";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_sbreak_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 2, 5)<<0) | (getField(pc->value, 12, 1)<<5), 6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 129;
  setHandler(pc, sbreak_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.sbreak";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetSimm(pc, pc->sim[0]);
}

static void csrrw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 89;
  setHandler(pc, csrrw_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "csrrw";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void csrrs_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 90;
  setHandler(pc, csrrs_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "csrrs";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void csrrc_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 91;
  setHandler(pc, csrrc_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "csrrc";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void csrrwi_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->uim[1] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 92;
  setHandler(pc, csrrwi_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "csrrwi";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetUimm(pc, pc->uim[1]);
  traceSetUimm(pc, pc->uim[0]);
}

static void csrrsi_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->uim[1] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 93;
  setHandler(pc, csrrsi_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "csrrsi";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetUimm(pc, pc->uim[1]);
  traceSetUimm(pc, pc->uim[0]);
}

static void csrrci_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->uim[1] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 94;
  setHandler(pc, csrrci_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "csrrci";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetUimm(pc, pc->uim[1]);
  traceSetUimm(pc, pc->uim[0]);
}

static void eret_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 100;
  setHandler(pc, eret_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "eret";
}

static void wfi_0_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 99;
  setHandler(pc, wfi_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "wfi";
}

static void uret_0_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 95;
  setHandler(pc, uret_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "uret";
}

static void sret_0_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 96;
  setHandler(pc, sret_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sret";
}

static void hret_0_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 97;
  setHandler(pc, hret_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "hret";
}

static void mret_0_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 98;
  setHandler(pc, mret_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "mret";
}

static void sfence_vm_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 101;
  setHandler(pc, sfence_vm_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "sfence.vm";
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void wfi_1_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 99;
  setHandler(pc, wfi_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "wfi";
}

static void LB_RR_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 130;
  setHandler(pc, lib_LB_RR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lb";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectReg(pc, pc->inReg[0], pc->inReg[1]);
}

static void LH_RR_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 131;
  setHandler(pc, lib_LH_RR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectReg(pc, pc->inReg[0], pc->inReg[1]);
}

static void LW_RR_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 132;
  setHandler(pc, lib_LW_RR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lw";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectReg(pc, pc->inReg[0], pc->inReg[1]);
}

static void LBU_RR_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 133;
  setHandler(pc, lib_LBU_RR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lbu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectReg(pc, pc->inReg[0], pc->inReg[1]);
}

static void LHU_RR_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 134;
  setHandler(pc, lib_LHU_RR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lhu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectReg(pc, pc->inReg[0], pc->inReg[1]);
}

static void LB_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 130;
  setHandler(pc, lib_LB_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lb";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImmPostInc(pc, pc->inReg[0], pc->sim[0]);
}

static void LH_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 131;
  setHandler(pc, lib_LH_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImmPostInc(pc, pc->inReg[0], pc->sim[0]);
}

static void LW_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 132;
  setHandler(pc, lib_LW_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lw";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImmPostInc(pc, pc->inReg[0], pc->sim[0]);
}

static void LBU_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 133;
  setHandler(pc, lib_LBU_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lbu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImmPostInc(pc, pc->inReg[0], pc->sim[0]);
}

static void LHU_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 134;
  setHandler(pc, lib_LHU_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lhu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImmPostInc(pc, pc->inReg[0], pc->sim[0]);
}

static void SB_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 5)<<0) | (getField(pc->value, 25, 7)<<5), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 135;
  setHandler(pc, lib_SB_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sb";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImmPostInc(pc, pc->inReg[0], pc->sim[0]);
}

static void SH_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 5)<<0) | (getField(pc->value, 25, 7)<<5), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 136;
  setHandler(pc, lib_SH_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sh";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImmPostInc(pc, pc->inReg[0], pc->sim[0]);
}

static void SW_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 5)<<0) | (getField(pc->value, 25, 7)<<5), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 137;
  setHandler(pc, lib_SW_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sw";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImmPostInc(pc, pc->inReg[0], pc->sim[0]);
}

static void LB_RR_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 130;
  setHandler(pc, lib_LB_RR_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lb";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectRegPostInc(pc, pc->inReg[0], pc->inReg[1]);
}

static void LH_RR_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 131;
  setHandler(pc, lib_LH_RR_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectRegPostInc(pc, pc->inReg[0], pc->inReg[1]);
}

static void LW_RR_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 132;
  setHandler(pc, lib_LW_RR_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lw";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectRegPostInc(pc, pc->inReg[0], pc->inReg[1]);
}

static void LBU_RR_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 133;
  setHandler(pc, lib_LBU_RR_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lbu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectRegPostInc(pc, pc->inReg[0], pc->inReg[1]);
}

static void LHU_RR_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 134;
  setHandler(pc, lib_LHU_RR_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.lhu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectRegPostInc(pc, pc->inReg[0], pc->inReg[1]);
}

static void SB_RR_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 7, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 135;
  setHandler(pc, lib_SB_RR_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sb";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectRegPostInc(pc, pc->inReg[0], pc->inReg[2]);
}

static void SH_RR_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 7, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 136;
  setHandler(pc, lib_SH_RR_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sh";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectRegPostInc(pc, pc->inReg[0], pc->inReg[2]);
}

static void SW_RR_POSTINC_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 7, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 137;
  setHandler(pc, lib_SW_RR_POSTINC_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sw";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectRegPostInc(pc, pc->inReg[0], pc->inReg[2]);
}

static void p_avgu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 138;
  setHandler(pc, lib_AVGU_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.avgu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void p_slet_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 139;
  setHandler(pc, p_slet_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.slet";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void p_sletu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 140;
  setHandler(pc, p_sletu_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sletu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void p_min_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 141;
  setHandler(pc, lib_MINS_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.min";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void p_minu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 142;
  setHandler(pc, lib_MINU_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.minu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void p_max_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 143;
  setHandler(pc, lib_MAXS_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.max";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void p_maxu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 144;
  setHandler(pc, lib_MAXU_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.maxu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void p_ror_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 145;
  setHandler(pc, lib_ROR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.ror";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
}

static void p_ff1_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 146;
  setHandler(pc, lib_FF1_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.ff1";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void p_fl1_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 147;
  setHandler(pc, lib_FL1_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.fl1";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void p_clb_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 148;
  setHandler(pc, lib_CLB_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.clb";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void p_cnt_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 149;
  setHandler(pc, lib_CNT_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.cnt";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void p_exths_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 150;
  setHandler(pc, p_exths_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.exths";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void p_exthz_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 151;
  setHandler(pc, p_exthz_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.exthz";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void p_extbs_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 152;
  setHandler(pc, p_extbs_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.extbs";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void p_extbz_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 153;
  setHandler(pc, p_extbz_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.extbz";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void lp_starti_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->uim[0] = (getField(pc->value, 7, 1)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[1] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 154;
  setHandler(pc, lp_starti_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lp.starti";
  traceSetUimm(pc, pc->uim[0]);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[1]);
}

static void lp_endi_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->uim[0] = (getField(pc->value, 7, 1)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[1] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 155;
  setHandler(pc, lp_endi_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lp.endi";
  traceSetUimm(pc, pc->uim[0]);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[1]);
}

static void lp_count_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->uim[0] = (getField(pc->value, 7, 1)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[1] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 156;
  setHandler(pc, lp_count_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lp.count";
  traceSetUimm(pc, pc->uim[0]);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[1]);
}

static void lp_counti_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->uim[0] = (getField(pc->value, 7, 1)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[1] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 157;
  setHandler(pc, lp_counti_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lp.counti";
  traceSetUimm(pc, pc->uim[0]);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[1]);
}

static void lp_setup_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->uim[0] = (getField(pc->value, 7, 1)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[1] = (getField(pc->value, 20, 12)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 158;
  setHandler(pc, lp_setup_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lp.setup";
  traceSetUimm(pc, pc->uim[0]);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[1]);
}

static void lp_setupi_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->uim[0] = (getField(pc->value, 7, 1)<<0);
  pc->uim[1] = (getField(pc->value, 20, 12)<<0);
  pc->uim[2] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 159;
  setHandler(pc, lp_setupi_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "lp.setupi";
  traceSetUimm(pc, pc->uim[0]);
  traceSetUimm(pc, pc->uim[1]);
  traceSetUimm(pc, pc->uim[2]);
}

static void flw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 59;
  setHandler(pc, lib_FLW_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "flw";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void fsw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 5)<<0) | (getField(pc->value, 25, 7)<<5), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 60;
  setHandler(pc, lib_FSW_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fsw";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void fmadd_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->inReg[2] = (getField(pc->value, 27, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_FMADD;
  pc->instrId = 61;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fmadd_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fmadd_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmadd.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fmsub_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->inReg[2] = (getField(pc->value, 27, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_FMADD;
  pc->instrId = 62;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fmsub_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fmsub_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmsub.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fmsub_s_0_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->inReg[2] = (getField(pc->value, 27, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_FMADD;
  pc->instrId = 62;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fnmsub_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fnmsub_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmsub.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fmsub_s_1_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->inReg[2] = (getField(pc->value, 27, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_FMADD;
  pc->instrId = 62;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fnmadd_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fnmadd_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmsub.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fadd_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_ADD;
  pc->instrId = 63;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fadd_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fadd_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fadd.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fsub_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_ADD;
  pc->instrId = 64;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fsub_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fsub_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fsub.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fmul_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_MUL;
  pc->instrId = 65;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fmul_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fmul_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmul.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fdiv_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_DIV;
  pc->instrId = 66;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fdiv_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fdiv_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fdiv.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fsqrt_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_DIV;
  pc->instrId = 67;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fsqrt_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fsqrt_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fsqrt.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fsgnj_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 68;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fsgnj_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fsgnj_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fsgnj.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fsgnjn_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 69;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fsgnjn_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fsgnjn_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fsgnjn.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fsgnjx_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 70;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fsgnjx_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fsgnjx_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fsgnjx.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fmin_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 71;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fmin_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fmin_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmin.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fmax_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 72;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fmax_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fmax_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmax.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fcvt_w_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 73;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fcvt_w_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fcvt_w_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fcvt.w.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fcvt_wu_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 74;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fcvt_wu_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fcvt_wu_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fcvt.wu.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fmv_x_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 75;
  setHandler(pc, lib_fmv_x_s_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmv.x.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void feq_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 76;
  setHandler(pc, lib_feq_s_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "feq.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void flt_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 77;
  setHandler(pc, lib_flt_s_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "flt.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fle_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 78;
  setHandler(pc, lib_fle_s_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fle.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fclass_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 79;
  setHandler(pc, lib_fclass_s_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fclass.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void fcvt_s_w_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 80;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fcvt_s_w_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fcvt_s_w_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fcvt.s.w";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fcvt_s_wu_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 81;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fcvt_s_wu_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fcvt_s_wu_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fcvt.s.wu";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fmv_s_x_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 82;
  setHandler(pc, lib_fmv_x_s_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fmv.s.x";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void fcvt_s_d_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 83;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fcvt_s_d_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fcvt_s_d_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fcvt.s.d";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fcvt_d_s_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 84;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fcvt_d_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fcvt_d_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fcvt.d.s";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void fsgnj_d_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0) + NB_REGS;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->uim[0] = (getField(pc->value, 12, 3)<<0);
  pc->group = INSTR_GROUP_FPU_CONV;
  pc->instrId = 85;
  pc->latency = getTiming_latency(cpu, &timingTable_fpu, pc->group);
  pc->bandwidth = getTiming_bandwidth(cpu, &timingTable_fpu, pc->group);
  if (__shared_fpu) {
    pc->execFunc = lib_fsgnj_s_exec;
    pc->offload = 1;
    setHandler(pc, shfpu_offload);
  } else {
    setHandler(pc, lib_fsgnj_s_exec);
    pc->offload = 0;
  }
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fsgnj.d";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetUimm(pc, pc->uim[0]);
}

static void c_fsw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = ((getField(pc->value, 2, 3)<<0)) + 8 + NB_REGS;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->sim[0] = (getField(pc->value, 6, 1)<<2) | (getField(pc->value, 10, 3)<<3) | (getField(pc->value, 5, 1)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 86;
  setHandler(pc, c_fsw_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.fsw";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_fswsp_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 2, 5)<<0) + NB_REGS;
  pc->inReg[0] = 2;
  pc->sim[0] = (getField(pc->value, 9, 4)<<2) | (getField(pc->value, 7, 2)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 87;
  setHandler(pc, c_fswsp_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.fswsp";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_flw_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 2, 3)<<0)) + 8 + NB_REGS;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->sim[0] = (getField(pc->value, 6, 1)<<2) | (getField(pc->value, 10, 3)<<3) | (getField(pc->value, 5, 1)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 88;
  setHandler(pc, c_flw_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.flw";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_flwsp_0_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = 2;
  pc->sim[0] = (getField(pc->value, 4, 3)<<2) | (getField(pc->value, 12, 1)<<5) | (getField(pc->value, 2, 2)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 58;
  setHandler(pc, c_flwsp_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.flwsp";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void fld_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 20, 12)<<0), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 53;
  setHandler(pc, lib_FLW_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fld";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void fsd_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0) + NB_REGS;
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->sim[0] = getSignedValue((getField(pc->value, 7, 5)<<0) | (getField(pc->value, 25, 7)<<5), 12);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 54;
  setHandler(pc, lib_FSW_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "fsd";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_fsd_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = ((getField(pc->value, 2, 3)<<0)) + 8 + NB_REGS;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->sim[0] = (getField(pc->value, 6, 1)<<2) | (getField(pc->value, 10, 3)<<3) | (getField(pc->value, 5, 1)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 55;
  setHandler(pc, c_fsd_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.fsd";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_fsdsp_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 2, 5)<<0) + NB_REGS;
  pc->inReg[0] = 2;
  pc->sim[0] = (getField(pc->value, 9, 4)<<2) | (getField(pc->value, 7, 2)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 56;
  setHandler(pc, c_fsdsp_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.fsdsp";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_fld_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = ((getField(pc->value, 2, 3)<<0)) + 8 + NB_REGS;
  pc->inReg[0] = ((getField(pc->value, 7, 3)<<0)) + 8;
  pc->sim[0] = (getField(pc->value, 6, 1)<<2) | (getField(pc->value, 10, 3)<<3) | (getField(pc->value, 5, 1)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 57;
  setHandler(pc, c_fld_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.fld";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void c_flwsp_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0) + NB_REGS;
  pc->inReg[0] = 2;
  pc->sim[0] = (getField(pc->value, 4, 3)<<2) | (getField(pc->value, 12, 1)<<5) | (getField(pc->value, 2, 2)<<6);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 58;
  setHandler(pc, c_flwsp_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 2;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 2);

  t->opcode = "c.flwsp";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetIndirectImm(pc, pc->inReg[0], pc->sim[0]);
}

static void p_abs_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 160;
  setHandler(pc, lib_ABS_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.abs";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
}

static void SB_RR_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 135;
  setHandler(pc, lib_SB_RR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sb";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectReg(pc, pc->inReg[0], pc->inReg[2]);
}

static void SH_RR_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 136;
  setHandler(pc, lib_SH_RR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sh";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectReg(pc, pc->inReg[0], pc->inReg[2]);
}

static void SW_RR_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 137;
  setHandler(pc, lib_SW_RR_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.sw";
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetIndirectReg(pc, pc->inReg[0], pc->inReg[2]);
}

static void p_mac_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 161;
  setHandler(pc, p_mac_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_sl_sl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 162;
  setHandler(pc, lib_MAC_SL_SL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.sl.sl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_sl_sh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 163;
  setHandler(pc, lib_MAC_SL_SH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.sl.sh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_sl_zl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 164;
  setHandler(pc, lib_MAC_SL_ZL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.sl.zl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_sl_zh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 165;
  setHandler(pc, lib_MAC_SL_ZH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.sl.zh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_sh_sl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 166;
  setHandler(pc, lib_MAC_SH_SL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.sh.sl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_sh_sh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 167;
  setHandler(pc, lib_MAC_SH_SH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.sh.sh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_sh_zl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 168;
  setHandler(pc, lib_MAC_SH_ZL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.sh.zl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_sh_zh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 169;
  setHandler(pc, lib_MAC_SH_ZH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.sh.zh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_zl_sl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 170;
  setHandler(pc, lib_MAC_ZL_SL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.zl.sl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_zl_sh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 171;
  setHandler(pc, lib_MAC_ZL_SH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.zl.sh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_zl_zl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 172;
  setHandler(pc, lib_MAC_ZL_ZL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.zl.zl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_zl_zh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 173;
  setHandler(pc, lib_MAC_ZL_ZH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.zl.zh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_zh_sl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 174;
  setHandler(pc, lib_MAC_ZH_SL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.zh.sl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_zh_sh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 175;
  setHandler(pc, lib_MAC_ZH_SH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.zh.sh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_zh_zl_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 176;
  setHandler(pc, lib_MAC_ZH_ZL_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.zh.zl";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

static void p_mac_zh_zh_decode_gen(cpu_t *cpu, pc_t *pc)
{
  trace_t *t = &pc->trace;
  pc->outReg[0] = (getField(pc->value, 7, 5)<<0);
  pc->inReg[0] = (getField(pc->value, 15, 5)<<0);
  pc->inReg[1] = (getField(pc->value, 20, 5)<<0);
  pc->inReg[2] = (getField(pc->value, 25, 5)<<0);
  pc->group = INSTR_GROUP_OTHER;
  pc->instrId = 177;
  setHandler(pc, lib_MAC_ZH_ZH_exec);
  if (cpu->cache->powerEn) setPowerTrace(cpu, pc, (char *)"pe.instr");
  pc->size = 4;
  if (pc->next == NULL) pc->next = getPc(cpu, pc->addr + 4);

  t->opcode = "p.mac.zh.zh";
  traceSetReg(pc, pc->outReg[0], 1, 1);
  traceSetReg(pc, pc->inReg[0], 0, 1);
  traceSetReg(pc, pc->inReg[1], 0, 1);
  traceSetReg(pc, pc->inReg[2], 0, 1);
}

int decodePc_rv32i(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x7f) >> 0;
  if (groupOpcode == 0b0110011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b100) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------100-----0110011: xor
          xor_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b101) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0100000) {
          // 0100000----------101-----0110011: sra
          sra_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0000000) {
          // 0000000----------101-----0110011: srl
          srl_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b110) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------110-----0110011: or
          or_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b111) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------111-----0110011: and
          and_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b011) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------011-----0110011: sltu
          sltu_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b001) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------001-----0110011: sll
          sll_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b010) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------010-----0110011: slt
          slt_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0100000) {
          // 0100000----------000-----0110011: sub
          sub_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0000000) {
          // 0000000----------000-----0110011: add
          add_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b0010111) {
      // -------------------------0010111: auipc
      auipc_decode_gen(cpu, pc);
  }
  else if (groupOpcode == 0b1101111) {
      // -------------------------1101111: jal
      jal_decode_gen(cpu, pc);
  }
  else if (groupOpcode == 0b0110111) {
      // -------------------------0110111: lui
      lui_decode_gen(cpu, pc);
  }
  else if (groupOpcode == 0b0001111) {
    unsigned int groupOpcode = (opcode & 0xfff80) >> 7;
    if (groupOpcode == 0b0000000100000) {
      unsigned int groupOpcode = (opcode & 0xfff00000) >> 20;
      if (groupOpcode == 0b000000000000) {
          // 00000000000000000001000000001111: fence.i
          fence_i_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b0000000000000) {
      unsigned int groupOpcode = (opcode & 0xf0000000) >> 28;
      if (groupOpcode == 0b0000) {
          // 0000--------00000000000000001111: fence
          fence_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b1110011) {
    unsigned int groupOpcode = (opcode & 0x7f80) >> 7;
    if (groupOpcode == 0b00000000) {
      unsigned int groupOpcode = (opcode & 0xfff00000) >> 20;
      if (groupOpcode == 0b000000000000) {
        unsigned int groupOpcode = (opcode & 0xf8000) >> 15;
        if (groupOpcode == 0b00000) {
            // 00000000000000000000000001110011: ecall
            ecall_decode_gen(cpu, pc);
        }
        else {
          // 000000000000-----000000001110011: scallimm
          scallimm_decode_gen(cpu, pc);
        }
      }
      else if (groupOpcode == 0b000000000001) {
        unsigned int groupOpcode = (opcode & 0xf8000) >> 15;
        if (groupOpcode == 0b00000) {
            // 00000000000100000000000001110011: sbreak
            sbreak_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b0010011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b100) {
        // -----------------100-----0010011: xori
        xori_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b101) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0100000) {
          // 0100000----------101-----0010011: srai
          srai_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0000000) {
          // 0000000----------101-----0010011: srli
          srli_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b110) {
        // -----------------110-----0010011: ori
        ori_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b111) {
        // -----------------111-----0010011: andi
        andi_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b011) {
        // -----------------011-----0010011: sltiu
        sltiu_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b001) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------001-----0010011: slli
          slli_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b010) {
        // -----------------010-----0010011: slti
        slti_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b000) {
      unsigned int groupOpcode = (opcode & 0xffff8f80) >> 7;
      if (groupOpcode == 0b0000000000000000000000) {
          // 00000000000000000000000000010011: addi
          addi_0_decode_gen(cpu, pc);
      }
      else {
        // -----------------000-----0010011: addi
        addi_decode_gen(cpu, pc);
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b1100111) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b000) {
        // -----------------000-----1100111: jalr
        jalr_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b1100011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b100) {
        // -----------------100-----1100011: blt
        blt_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b110) {
        // -----------------110-----1100011: bltu
        bltu_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b111) {
        // -----------------111-----1100011: bgeu
        bgeu_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b001) {
        // -----------------001-----1100011: bne
        bne_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b101) {
        // -----------------101-----1100011: bge
        bge_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b000) {
        // -----------------000-----1100011: beq
        beq_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b0000011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b100) {
        // -----------------100-----0000011: lbu
        lbu_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b101) {
        // -----------------101-----0000011: lhu
        lhu_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b001) {
        // -----------------001-----0000011: lh
        lh_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b010) {
        // -----------------010-----0000011: lw
        lw_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b000) {
        // -----------------000-----0000011: lb
        lb_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b0100011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b010) {
        // -----------------010-----0100011: sw
        sw_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b001) {
        // -----------------001-----0100011: sh
        sh_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b000) {
        // -----------------000-----0100011: sb
        sb_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_rv32m(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x7f) >> 0;
  if (groupOpcode == 0b0110011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b100) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000001) {
          // 0000001----------100-----0110011: div
          div_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b101) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000001) {
          // 0000001----------101-----0110011: divu
          divu_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b110) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000001) {
          // 0000001----------110-----0110011: rem
          rem_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b111) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000001) {
          // 0000001----------111-----0110011: remu
          remu_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b011) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000001) {
          // 0000001----------011-----0110011: mulhu
          mulhu_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b001) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000001) {
          // 0000001----------001-----0110011: mulh
          mulh_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b010) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000001) {
          // 0000001----------010-----0110011: mulhsu
          mulhsu_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000001) {
          // 0000001----------000-----0110011: mul
          mul_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_rv32c(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x3) >> 0;
  if (groupOpcode == 0b00) {
    unsigned int groupOpcode = (opcode & 0xe000) >> 13;
    if (groupOpcode == 0b010) {
        // 010-----------00: c.lw
        c_lw_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b110) {
        // 110-----------00: c.sw
        c_sw_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b000) {
        // 000-----------00: c.addi4spn
        c_addi4spn_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b01) {
    unsigned int groupOpcode = (opcode & 0xe000) >> 13;
    if (groupOpcode == 0b100) {
      unsigned int groupOpcode = (opcode & 0xc00) >> 10;
      if (groupOpcode == 0b11) {
        unsigned int groupOpcode = (opcode & 0x60) >> 5;
        if (groupOpcode == 0b11) {
          unsigned int groupOpcode = (opcode & 0x1000) >> 12;
          if (groupOpcode == 0b0) {
              // 100011---11---01: c.and
              c_and_decode_gen(cpu, pc);
          }
          else {
            goto error;
          }
        }
        else if (groupOpcode == 0b00) {
          unsigned int groupOpcode = (opcode & 0x1000) >> 12;
          if (groupOpcode == 0b0) {
              // 100011---00---01: c.sub
              c_sub_decode_gen(cpu, pc);
          }
          else {
            goto error;
          }
        }
        else if (groupOpcode == 0b01) {
          unsigned int groupOpcode = (opcode & 0x1000) >> 12;
          if (groupOpcode == 0b0) {
              // 100011---01---01: c.xor
              c_xor_decode_gen(cpu, pc);
          }
          else {
            goto error;
          }
        }
        else if (groupOpcode == 0b10) {
          unsigned int groupOpcode = (opcode & 0x1000) >> 12;
          if (groupOpcode == 0b0) {
              // 100011---10---01: c.or
              c_or_decode_gen(cpu, pc);
          }
          else {
            goto error;
          }
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b00) {
          // 100-00--------01: c.srli
          c_srli_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b01) {
          // 100-01--------01: c.srai
          c_srai_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b10) {
          // 100-10--------01: c.andi
          c_andi_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b101) {
        // 101-----------01: c.j
        c_j_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b110) {
        // 110-----------01: c.beqz
        c_beqz_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b111) {
        // 111-----------01: c.bnez
        c_bnez_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b011) {
      unsigned int groupOpcode = (opcode & 0xf80) >> 7;
      if (groupOpcode == 0b00010) {
          // 011-00010-----01: c.addi16sp
          c_addi16sp_decode_gen(cpu, pc);
      }
      else {
        // 011-----------01: c.lui
        c_lui_decode_gen(cpu, pc);
      }
    }
    else if (groupOpcode == 0b001) {
        // 001-----------01: c.jal
        c_jal_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b010) {
        // 010-----------01: c.li
        c_li_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b000) {
      unsigned int groupOpcode = (opcode & 0x1ffc) >> 2;
      if (groupOpcode == 0b00000000000) {
          // 0000000000000001: c.nop
          c_nop_decode_gen(cpu, pc);
      }
      else {
        // 000-----------01: c.addi
        c_addi_decode_gen(cpu, pc);
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b10) {
    unsigned int groupOpcode = (opcode & 0xe000) >> 13;
    if (groupOpcode == 0b010) {
        // 010-----------10: c.lwsp
        c_lwsp_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b110) {
        // 110-----------10: c.swsp
        c_swsp_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b100) {
      unsigned int groupOpcode = (opcode & 0x1000) >> 12;
      if (groupOpcode == 0b0) {
        unsigned int groupOpcode = (opcode & 0x7c) >> 2;
        if (groupOpcode == 0b00000) {
          unsigned int groupOpcode = (opcode & 0xf80) >> 7;
          if (groupOpcode == 0b00000) {
              // 1000000000000010: c.sbreak
              c_sbreak_decode_gen(cpu, pc);
          }
          else {
            // 1000-----0000010: c.jr
            c_jr_decode_gen(cpu, pc);
          }
        }
        else {
          // 1000----------10: c.mv
          c_mv_decode_gen(cpu, pc);
        }
      }
      else if (groupOpcode == 0b1) {
        unsigned int groupOpcode = (opcode & 0x7c) >> 2;
        if (groupOpcode == 0b00000) {
          unsigned int groupOpcode = (opcode & 0xf80) >> 7;
          if (groupOpcode == 0b00000) {
              // 1001000000000010: c.ebreak
              c_ebreak_decode_gen(cpu, pc);
          }
          else {
            // 1001-----0000010: c.jalr
            c_jalr_decode_gen(cpu, pc);
          }
        }
        else {
          // 1001----------10: c.add
          c_add_decode_gen(cpu, pc);
        }
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000) {
        // 000-----------10: c.slli
        c_slli_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_priv(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x7f) >> 0;
  if (groupOpcode == 0b1110011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b010) {
        // -----------------010-----1110011: csrrs
        csrrs_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b110) {
        // -----------------110-----1110011: csrrsi
        csrrsi_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b111) {
        // -----------------111-----1110011: csrrci
        csrrci_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b011) {
        // -----------------011-----1110011: csrrc
        csrrc_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b001) {
        // -----------------001-----1110011: csrrw
        csrrw_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b101) {
        // -----------------101-----1110011: csrrwi
        csrrwi_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_priv_pulp(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0xffffffff) >> 0;
  if (groupOpcode == 0b00010000001000000000000001110011) {
      // 00010000001000000000000001110011: wfi
      if (!__priv_pulp) goto error;
      wfi_0_decode_gen(cpu, pc);
  }
  else if (groupOpcode == 0b00010000000000000000000001110011) {
      // 00010000000000000000000001110011: eret
      if (!__priv_pulp) goto error;
      eret_decode_gen(cpu, pc);
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_priv_1_9(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x7fff) >> 0;
  if (groupOpcode == 0b000000001110011) {
    unsigned int groupOpcode = (opcode & 0xfff00000) >> 20;
    if (groupOpcode == 0b001000000010) {
      unsigned int groupOpcode = (opcode & 0xf8000) >> 15;
      if (groupOpcode == 0b00000) {
          // 00100000001000000000000001110011: hret
          if (!__priv_1_9) goto error;
          hret_0_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000100000010) {
      unsigned int groupOpcode = (opcode & 0xf8000) >> 15;
      if (groupOpcode == 0b00000) {
          // 00010000001000000000000001110011: sret
          if (!__priv_1_9) goto error;
          sret_0_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000000000010) {
      unsigned int groupOpcode = (opcode & 0xf8000) >> 15;
      if (groupOpcode == 0b00000) {
          // 00000000001000000000000001110011: uret
          if (!__priv_1_9) goto error;
          uret_0_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b001100000010) {
      unsigned int groupOpcode = (opcode & 0xf8000) >> 15;
      if (groupOpcode == 0b00000) {
          // 00110000001000000000000001110011: mret
          if (!__priv_1_9) goto error;
          mret_0_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000100000100) {
        // 000100000100-----000000001110011: sfence.vm
        if (!__priv_1_9) goto error;
        sfence_vm_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b000100000101) {
      unsigned int groupOpcode = (opcode & 0xf8000) >> 15;
      if (groupOpcode == 0b00000) {
          // 00010000010100000000000001110011: wfi
          if (!__priv_1_9) goto error;
          wfi_1_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_pulp(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x7f) >> 0;
  if (groupOpcode == 0b0110011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b100) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0001000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 000100000000-----100-----0110011: p.exths
            p_exths_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000010) {
          // 0000010----------100-----0110011: p.min
          p_min_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b110) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0001000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 000100000000-----110-----0110011: p.extbs
            p_extbs_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000010) {
          // 0000010----------110-----0110011: p.max
          p_max_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b010) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0001000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 000100000000-----010-----0110011: p.clb
            p_clb_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000010) {
          // 0000010----------010-----0110011: p.slet
          p_slet_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b111) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0001000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 000100000000-----111-----0110011: p.extbz
            p_extbz_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000010) {
          // 0000010----------111-----0110011: p.maxu
          p_maxu_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b011) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0001000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 000100000000-----011-----0110011: p.cnt
            p_cnt_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000010) {
          // 0000010----------011-----0110011: p.sletu
          p_sletu_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b001) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0001000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 000100000000-----001-----0110011: p.fl1
            p_fl1_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000010) {
          // 0000010----------001-----0110011: p.avgu
          p_avgu_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b101) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000010) {
          // 0000010----------101-----0110011: p.minu
          p_minu_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0001000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 000100000000-----101-----0110011: p.exthz
            p_exthz_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000100) {
          // 0000100----------101-----0110011: p.ror
          p_ror_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000) {
      unsigned int groupOpcode = (opcode & 0xfff00000) >> 20;
      if (groupOpcode == 0b000100000000) {
          // 000100000000-----000-----0110011: p.ff1
          p_ff1_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b0001011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b101) {
        // -----------------101-----0001011: LHU_POSTINC
        LHU_POSTINC_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b010) {
        // -----------------010-----0001011: LW_POSTINC
        LW_POSTINC_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b111) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0101000) {
          // 0101000----------111-----0001011: LHU_RR_POSTINC
          LHU_RR_POSTINC_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0010000) {
          // 0010000----------111-----0001011: LW_RR_POSTINC
          LW_RR_POSTINC_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0000000) {
          // 0000000----------111-----0001011: LB_RR_POSTINC
          LB_RR_POSTINC_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0001000) {
          // 0001000----------111-----0001011: LH_RR_POSTINC
          LH_RR_POSTINC_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0100000) {
          // 0100000----------111-----0001011: LBU_RR_POSTINC
          LBU_RR_POSTINC_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b001) {
        // -----------------001-----0001011: LH_POSTINC
        LH_POSTINC_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b100) {
        // -----------------100-----0001011: LBU_POSTINC
        LBU_POSTINC_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b000) {
        // -----------------000-----0001011: LB_POSTINC
        LB_POSTINC_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b0000011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b111) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0101000) {
          // 0101000----------111-----0000011: LHU_RR
          LHU_RR_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0010000) {
          // 0010000----------111-----0000011: LW_RR
          LW_RR_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0000000) {
          // 0000000----------111-----0000011: LB_RR
          LB_RR_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0001000) {
          // 0001000----------111-----0000011: LH_RR
          LH_RR_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0100000) {
          // 0100000----------111-----0000011: LBU_RR
          LBU_RR_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b1111011) {
    unsigned int groupOpcode = (opcode & 0x7f00) >> 8;
    if (groupOpcode == 0b0100000) {
        // -----------------0100000-1111011: lp.count
        lp_count_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b0000000) {
        // -----------------0000000-1111011: lp.starti
        lp_starti_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b0110000) {
        // -----------------0110000-1111011: lp.counti
        lp_counti_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b0010000) {
        // -----------------0010000-1111011: lp.endi
        lp_endi_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b1010000) {
        // -----------------1010000-1111011: lp.setupi
        lp_setupi_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b1000000) {
        // -----------------1000000-1111011: lp.setup
        lp_setup_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b0101011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b101) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------101-----0101011: SH_RR_POSTINC
          SH_RR_POSTINC_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b010) {
        // -----------------010-----0101011: SW_POSTINC
        SW_POSTINC_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b110) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------110-----0101011: SW_RR_POSTINC
          SW_RR_POSTINC_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b001) {
        // -----------------001-----0101011: SH_POSTINC
        SH_POSTINC_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b100) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0000000) {
          // 0000000----------100-----0101011: SB_RR_POSTINC
          SB_RR_POSTINC_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000) {
        // -----------------000-----0101011: SB_POSTINC
        SB_POSTINC_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_fpu(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x3) >> 0;
  if (groupOpcode == 0b11) {
    unsigned int groupOpcode = (opcode & 0x7c) >> 2;
    if (groupOpcode == 0b10010) {
      unsigned int groupOpcode = (opcode & 0x6000000) >> 25;
      if (groupOpcode == 0b00) {
          // -----00------------------1001011: fmsub.s
          if (!__fpu) goto error;
          fmsub_s_0_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b00001) {
      unsigned int groupOpcode = (opcode & 0x7000) >> 12;
      if (groupOpcode == 0b010) {
          // -----------------010-----0000111: flw
          if (!__fpu) goto error;
          flw_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b10100) {
      unsigned int groupOpcode = (opcode & 0xfe000000) >> 25;
      if (groupOpcode == 0b0101100) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 010110000000-------------1010011: fsqrt.s
            if (!__fpu) goto error;
            fsqrt_s_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0001100) {
          // 0001100------------------1010011: fdiv.s
          if (!__fpu) goto error;
          fdiv_s_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0010100) {
        unsigned int groupOpcode = (opcode & 0x7000) >> 12;
        if (groupOpcode == 0b001) {
            // 0010100----------001-----1010011: fmax.s
            if (!__fpu) goto error;
            fmax_s_decode_gen(cpu, pc);
        }
        else if (groupOpcode == 0b000) {
            // 0010100----------000-----1010011: fmin.s
            if (!__fpu) goto error;
            fmin_s_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000100) {
          // 0000100------------------1010011: fsub.s
          if (!__fpu) goto error;
          fsub_s_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b1110000) {
        unsigned int groupOpcode = (opcode & 0x7000) >> 12;
        if (groupOpcode == 0b001) {
          unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
          if (groupOpcode == 0b00000) {
              // 111000000000-----001-----1010011: fclass.s
              if (!__fpu) goto error;
              fclass_s_decode_gen(cpu, pc);
          }
          else {
            goto error;
          }
        }
        else if (groupOpcode == 0b000) {
          unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
          if (groupOpcode == 0b00000) {
              // 111000000000-----000-----1010011: fmv.x.s
              if (!__fpu) goto error;
              fmv_x_s_decode_gen(cpu, pc);
          }
          else {
            goto error;
          }
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b1010000) {
        unsigned int groupOpcode = (opcode & 0x7000) >> 12;
        if (groupOpcode == 0b010) {
            // 1010000----------010-----1010011: feq.s
            if (!__fpu) goto error;
            feq_s_decode_gen(cpu, pc);
        }
        else if (groupOpcode == 0b001) {
            // 1010000----------001-----1010011: flt.s
            if (!__fpu) goto error;
            flt_s_decode_gen(cpu, pc);
        }
        else if (groupOpcode == 0b000) {
            // 1010000----------000-----1010011: fle.s
            if (!__fpu) goto error;
            fle_s_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0010001) {
        unsigned int groupOpcode = (opcode & 0x7000) >> 12;
        if (groupOpcode == 0b000) {
            // 0010001----------000-----1010011: fsgnj.d
            if (!__fpu) goto error;
            fsgnj_d_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b1111000) {
        unsigned int groupOpcode = (opcode & 0x7000) >> 12;
        if (groupOpcode == 0b000) {
          unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
          if (groupOpcode == 0b00000) {
              // 111100000000-----000-----1010011: fmv.s.x
              if (!__fpu) goto error;
              fmv_s_x_decode_gen(cpu, pc);
          }
          else {
            goto error;
          }
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b1100000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00001) {
            // 110000000001-------------1010011: fcvt.wu.s
            if (!__fpu) goto error;
            fcvt_wu_s_decode_gen(cpu, pc);
        }
        else if (groupOpcode == 0b00000) {
            // 110000000000-------------1010011: fcvt.w.s
            if (!__fpu) goto error;
            fcvt_w_s_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0100000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00001) {
            // 010000000001-------------1010011: fcvt.s.d
            if (!__fpu) goto error;
            fcvt_s_d_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0000000) {
          // 0000000------------------1010011: fadd.s
          if (!__fpu) goto error;
          fadd_s_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0100001) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00000) {
            // 010000100000-------------1010011: fcvt.d.s
            if (!__fpu) goto error;
            fcvt_d_s_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b0001000) {
          // 0001000------------------1010011: fmul.s
          if (!__fpu) goto error;
          fmul_s_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b0010000) {
        unsigned int groupOpcode = (opcode & 0x7000) >> 12;
        if (groupOpcode == 0b010) {
            // 0010000----------010-----1010011: fsgnjx.s
            if (!__fpu) goto error;
            fsgnjx_s_decode_gen(cpu, pc);
        }
        else if (groupOpcode == 0b001) {
            // 0010000----------001-----1010011: fsgnjn.s
            if (!__fpu) goto error;
            fsgnjn_s_decode_gen(cpu, pc);
        }
        else if (groupOpcode == 0b000) {
            // 0010000----------000-----1010011: fsgnj.s
            if (!__fpu) goto error;
            fsgnj_s_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else if (groupOpcode == 0b1101000) {
        unsigned int groupOpcode = (opcode & 0x1f00000) >> 20;
        if (groupOpcode == 0b00001) {
            // 110100000001-------------1010011: fcvt.s.wu
            if (!__fpu) goto error;
            fcvt_s_wu_decode_gen(cpu, pc);
        }
        else if (groupOpcode == 0b00000) {
            // 110100000000-------------1010011: fcvt.s.w
            if (!__fpu) goto error;
            fcvt_s_w_decode_gen(cpu, pc);
        }
        else {
          goto error;
        }
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b10011) {
      unsigned int groupOpcode = (opcode & 0x6000000) >> 25;
      if (groupOpcode == 0b00) {
          // -----00------------------1001111: fmsub.s
          if (!__fpu) goto error;
          fmsub_s_1_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b10000) {
      unsigned int groupOpcode = (opcode & 0x6000000) >> 25;
      if (groupOpcode == 0b00) {
          // -----00------------------1000011: fmadd.s
          if (!__fpu) goto error;
          fmadd_s_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b10001) {
      unsigned int groupOpcode = (opcode & 0x6000000) >> 25;
      if (groupOpcode == 0b00) {
          // -----00------------------1000111: fmsub.s
          if (!__fpu) goto error;
          fmsub_s_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b01001) {
      unsigned int groupOpcode = (opcode & 0x7000) >> 12;
      if (groupOpcode == 0b010) {
          // -----------------010-----0100111: fsw
          if (!__fpu) goto error;
          fsw_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b00) {
    unsigned int groupOpcode = (opcode & 0xe000) >> 13;
    if (groupOpcode == 0b111) {
        // 111-----------00: c.fsw
        if (!__fpu) goto error;
        c_fsw_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b011) {
        // 011-----------00: c.flw
        if (!__fpu) goto error;
        c_flw_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b10) {
    unsigned int groupOpcode = (opcode & 0xe000) >> 13;
    if (groupOpcode == 0b111) {
        // 111-----------10: c.fswsp
        if (!__fpu) goto error;
        c_fswsp_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b011) {
        // 011-----------10: c.flwsp
        if (!__fpu) goto error;
        c_flwsp_0_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_fpud(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x3) >> 0;
  if (groupOpcode == 0b11) {
    unsigned int groupOpcode = (opcode & 0x7c) >> 2;
    if (groupOpcode == 0b00001) {
      unsigned int groupOpcode = (opcode & 0x7000) >> 12;
      if (groupOpcode == 0b011) {
          // -----------------011-----0000111: fld
          if (!__fpud) goto error;
          fld_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b01001) {
      unsigned int groupOpcode = (opcode & 0x7000) >> 12;
      if (groupOpcode == 0b011) {
          // -----------------011-----0100111: fsd
          if (!__fpud) goto error;
          fsd_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b00) {
    unsigned int groupOpcode = (opcode & 0xe000) >> 13;
    if (groupOpcode == 0b101) {
        // 101-----------00: c.fsd
        if (!__fpud) goto error;
        c_fsd_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b001) {
        // 001-----------00: c.fld
        if (!__fpud) goto error;
        c_fld_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b10) {
    unsigned int groupOpcode = (opcode & 0xe000) >> 13;
    if (groupOpcode == 0b101) {
        // 101-----------10: c.fsdsp
        if (!__fpud) goto error;
        c_fsdsp_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b001) {
        // 001-----------10: c.flwsp
        if (!__fpud) goto error;
        c_flwsp_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int decodePc_pulp_v1(cpu_t *cpu, pc_t *pc)
{
  unsigned int opcode = pc->value;
  unsigned int groupOpcode = (opcode & 0x7f) >> 0;
  if (groupOpcode == 0b0110011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b000) {
      unsigned int groupOpcode = (opcode & 0xfff00000) >> 20;
      if (groupOpcode == 0b000101000000) {
          // 000101000000-----000-----0110011: p.abs
          p_abs_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b1011011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b101) {
      unsigned int groupOpcode = (opcode & 0xc0000000) >> 30;
      if (groupOpcode == 0b11) {
          // 11---------------101-----1011011: p.mac.sl.sh
          p_mac_sl_sh_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b10) {
          // 10---------------101-----1011011: p.mac.sl.zh
          p_mac_sl_zh_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b01) {
          // 01---------------101-----1011011: p.mac.zl.sh
          p_mac_zl_sh_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b00) {
          // 00---------------101-----1011011: p.mac.zl.zh
          p_mac_zl_zh_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b111) {
      unsigned int groupOpcode = (opcode & 0xc0000000) >> 30;
      if (groupOpcode == 0b11) {
          // 11---------------111-----1011011: p.mac.sh.sh
          p_mac_sh_sh_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b10) {
          // 10---------------111-----1011011: p.mac.sh.zh
          p_mac_sh_zh_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b01) {
          // 01---------------111-----1011011: p.mac.zh.sh
          p_mac_zh_sh_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b00) {
          // 00---------------111-----1011011: p.mac.zh.zh
          p_mac_zh_zh_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b110) {
      unsigned int groupOpcode = (opcode & 0xc0000000) >> 30;
      if (groupOpcode == 0b11) {
          // 11---------------110-----1011011: p.mac.sh.sl
          p_mac_sh_sl_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b10) {
          // 10---------------110-----1011011: p.mac.sh.zl
          p_mac_sh_zl_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b01) {
          // 01---------------110-----1011011: p.mac.zh.sl
          p_mac_zh_sl_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b00) {
          // 00---------------110-----1011011: p.mac.zh.zl
          p_mac_zh_zl_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b100) {
      unsigned int groupOpcode = (opcode & 0xc0000000) >> 30;
      if (groupOpcode == 0b11) {
          // 11---------------100-----1011011: p.mac.sl.sl
          p_mac_sl_sl_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b10) {
          // 10---------------100-----1011011: p.mac.sl.zl
          p_mac_sl_zl_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b01) {
          // 01---------------100-----1011011: p.mac.zl.sl
          p_mac_zl_sl_decode_gen(cpu, pc);
      }
      else if (groupOpcode == 0b00) {
          // 00---------------100-----1011011: p.mac.zl.zl
          p_mac_zl_zl_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else if (groupOpcode == 0b000) {
      unsigned int groupOpcode = (opcode & 0xc0000000) >> 30;
      if (groupOpcode == 0b00) {
          // 00---------------000-----1011011: p.mac
          p_mac_decode_gen(cpu, pc);
      }
      else {
        goto error;
      }
    }
    else {
      goto error;
    }
  }
  else if (groupOpcode == 0b0100011) {
    unsigned int groupOpcode = (opcode & 0x7000) >> 12;
    if (groupOpcode == 0b100) {
        // -----------------100-----0100011: SB_RR
        SB_RR_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b110) {
        // -----------------110-----0100011: SW_RR
        SW_RR_decode_gen(cpu, pc);
    }
    else if (groupOpcode == 0b101) {
        // -----------------101-----0100011: SH_RR
        SH_RR_decode_gen(cpu, pc);
    }
    else {
      goto error;
    }
  }
  else {
    goto error;
  }
  return 0;
  error:
  return -1;
}
int processPc(cpu_t *cpu, pc_t *pc)
{
  if (decodePc_rv32i(cpu, pc) == 0) goto found;
  if (decodePc_rv32m(cpu, pc) == 0) goto found;
  if (decodePc_rv32c(cpu, pc) == 0) goto found;
  if (decodePc_priv(cpu, pc) == 0) goto found;
  if (__priv_pulp && decodePc_priv_pulp(cpu, pc) == 0) goto found;
  if (__priv_1_9 && decodePc_priv_1_9(cpu, pc) == 0) goto found;
  if (decodePc_pulp(cpu, pc) == 0) goto found;
  if (__fpu && decodePc_fpu(cpu, pc) == 0) goto found;
  if (__fpud && decodePc_fpud(cpu, pc) == 0) goto found;
  if (decodePc_pulp_v1(cpu, pc) == 0) goto found;
  goto error;
  found:
  return 0;
  error:
  return 1;
}
pc_t *decodePc(cpu_t *cpu, pc_t *pc)
{
  sim_fetch_decode(cpu, pc->addr, (unsigned char *)&pc->value);
  if (decodePc_rv32i(cpu, pc) == 0) goto found;
  if (decodePc_rv32m(cpu, pc) == 0) goto found;
  if (decodePc_rv32c(cpu, pc) == 0) goto found;
  if (decodePc_priv(cpu, pc) == 0) goto found;
  if (getOption(cpu, __priv_pulp) && decodePc_priv_pulp(cpu, pc) == 0) goto found;
  if (getOption(cpu, __priv_1_9) && decodePc_priv_1_9(cpu, pc) == 0) goto found;
  if (decodePc_pulp(cpu, pc) == 0) goto found;
  if (getOption(cpu, __fpu) && decodePc_fpu(cpu, pc) == 0) goto found;
  if (getOption(cpu, __fpud) && decodePc_fpud(cpu, pc) == 0) goto found;
  if (decodePc_pulp_v1(cpu, pc) == 0) goto found;
  goto error;
  found:
  if (cpu->cache->traceEn)
  {
  pc->trace.saved_exec = pc->handler;
  setHandler(pc, trace_exec);
  }
  else
  {
  pc->trace.saved_exec = NULL;
  }
  
  if (cpu->cache->powerEn)
  {
  pc->trace.power_saved_exec = pc->handler;
  setHandler(pc, power_exec);
  }
  
  if (cpu->cache->statsEn)
  {
  pc->trace.stats_saved_exec = pc->handler;
  setHandler(pc, stats_exec);
  }
  
  return handlePc(cpu, pc);
  
  error:
  triggerException(cpu, pc, EXCEPTION_ILLEGAL_INSTR);
  return pc;
}
