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

#ifndef __CPU_ISS_RV32I_HPP
#define __CPU_ISS_RV32I_HPP

#include "iss_core.hpp"
#include "isa_lib/int.h"
#include "isa_lib/macros.h"



static inline iss_insn_t *lui_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, UIM_GET(0));
  return insn->next;
}



static inline iss_insn_t *auipc_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, UIM_GET(0));
  return insn->next;
}

static inline void auipc_decode(iss *iss, iss_insn_t *insn)
{
  insn->uim[0] += insn->addr;
}



static inline iss_insn_t *jal_exec(iss *iss, iss_insn_t *insn)
{
  unsigned int D = insn->out_regs[0];
  if (D != 0) REG_SET(0, insn->addr + insn->size);
  //accountJump(cpu);
  return insn->next;
}

static inline void jal_decode(iss *iss, iss_insn_t *insn)
{


  insn->next = insn_cache_get(iss, insn->addr + insn->sim[0]);
}



static inline iss_insn_t *jalr_exec(iss *iss, iss_insn_t *insn)
{
  iss_insn_t *next_insn = insn_cache_get(iss, insn->sim[0] + iss_get_reg_for_jump(iss, insn->in_regs[0]));
  unsigned int D = insn->out_regs[0];
  if (D != 0) REG_SET(0, insn->addr + insn->size);
  //accountJumpRegister(cpu);
  return next_insn;
}



static inline void bxx_decode(iss *iss, iss_insn_t *insn)
{
  unsigned int next_pc = insn->addr + SIM_GET(0);
  insn->branch = insn_cache_get(iss, next_pc);
}

static inline iss_insn_t *beq_exec(iss *iss, iss_insn_t *insn)
{
  if (REG_GET(0) == REG_GET(1)) {
    //accountTakenBranch(cpu);
    return insn->branch;
  } else {
    //accountNotTakenBranch(cpu);
    return insn->next;
  }
}



static inline iss_insn_t *bne_exec(iss *iss, iss_insn_t *insn)
{
  if (REG_GET(0) != REG_GET(1)) {
    //accountTakenBranch(cpu);
    return insn->branch;
  } else {
    //accountNotTakenBranch(cpu);
    return insn->next;
  }
}

static inline iss_insn_t *blt_exec(iss *iss, iss_insn_t *insn)
{
  if ((int32_t)REG_GET(0) < (int32_t)REG_GET(1)) {
    //accountTakenBranch(cpu);
    return insn->branch;
  } else {
    //accountNotTakenBranch(cpu);
    return insn->next;
  }
}

static inline iss_insn_t *bge_exec(iss *iss, iss_insn_t *insn)
{
  if ((int32_t)REG_GET(0) >= (int32_t)REG_GET(1)) {
    //accountTakenBranch(cpu);
    return insn->branch;
  } else {
    //accountNotTakenBranch(cpu);
    return insn->next;
  }
}

static inline iss_insn_t *bltu_exec(iss *iss, iss_insn_t *insn)
{
  if (REG_GET(0) < REG_GET(1)) {
    //accountTakenBranch(cpu);
    return insn->branch;
  } else {
    //accountNotTakenBranch(cpu);
    return insn->next;
  }
}

static inline iss_insn_t *bgeu_exec(iss *iss, iss_insn_t *insn)
{
  if (REG_GET(0) >= REG_GET(1)) {
    //accountTakenBranch(cpu);
    return insn->branch;
  } else {
    //accountNotTakenBranch(cpu);
    return insn->next;
  }
}



static inline iss_insn_t *lb_exec(iss *iss, iss_insn_t *insn)
{
  uint32_t value = 0;
  if (iss_lsu_load(iss, insn, REG_GET(0) + SIM_GET(0), 1, (uint8_t *)&value)) return insn;
  REG_SET(0, iss_get_signed_value(value, 8));
  //setRegDelayed(cpu, pc->outReg[0], value, 2);
  return insn->next;
}



static inline iss_insn_t *lh_exec(iss *iss, iss_insn_t *insn)
{
  uint32_t value = 0;
  if (iss_lsu_load(iss, insn, REG_GET(0) + SIM_GET(0), 2, (uint8_t *)&value)) return insn;
  REG_SET(0, iss_get_signed_value(value, 16));
  //setRegDelayed(cpu, pc->outReg[0], value, 2);
  return insn->next;
}



static inline iss_insn_t *lw_exec(iss *iss, iss_insn_t *insn)
{
  uint32_t value = 0;
  if (iss_lsu_load(iss, insn, REG_GET(0) + SIM_GET(0), 4, (uint8_t *)&value)) return insn;
  REG_SET(0, value);
  //setRegDelayed(cpu, pc->outReg[0], value, 2);
  return insn->next;
}



static inline iss_insn_t *lbu_exec(iss *iss, iss_insn_t *insn)
{
  uint32_t value = 0;
  if (iss_lsu_load(iss, insn, REG_GET(0) + SIM_GET(0), 1, (uint8_t *)&value)) return insn;
  REG_SET(0, value);
  //setRegDelayed(cpu, pc->outReg[0], value, 2);
  return insn->next;
}



static inline iss_insn_t *lhu_exec(iss *iss, iss_insn_t *insn)
{
  uint32_t value = 0;
  if (iss_lsu_load(iss, insn, REG_GET(0) + SIM_GET(0), 2, (uint8_t *)&value)) return insn;
  REG_SET(0, value);
  //setRegDelayed(cpu, pc->outReg[0], value, 2);
  return insn->next;
}



static inline iss_insn_t *sb_exec(iss *iss, iss_insn_t *insn)
{
  uint32_t value = REG_GET(1);
  if (iss_lsu_store(iss, insn, REG_GET(0) + SIM_GET(0), 1, (uint8_t *)&value)) return insn;
  return insn->next;
}



static inline iss_insn_t *sh_exec(iss *iss, iss_insn_t *insn)
{
  uint32_t value = REG_GET(1);
  if (iss_lsu_store(iss, insn, REG_GET(0) + SIM_GET(0), 2, (uint8_t *)&value)) return insn;
  return insn->next;
}



static inline iss_insn_t *sw_exec(iss *iss, iss_insn_t *insn)
{
  uint32_t value = REG_GET(1);
  if (iss_lsu_store(iss, insn, REG_GET(0) + SIM_GET(0), 4, (uint8_t *)&value)) return insn;
  return insn->next;
}



static inline iss_insn_t *addi_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_ADD, REG_GET(0), SIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *nop_exec(iss *iss, iss_insn_t *insn)
{
  return insn->next;
}



static inline iss_insn_t *slti_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, (int32_t)REG_GET(0) < insn->sim[0]);
  return insn->next;
}



static inline iss_insn_t *sltiu_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, REG_GET(0) < (uint32_t)SIM_GET(0));
  return insn->next;
}



static inline iss_insn_t *xori_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_XOR, REG_GET(0), SIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *ori_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_OR, REG_GET(0), SIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *andi_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_AND, REG_GET(0), SIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *slli_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_SLL, REG_GET(0), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *srli_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_SRL, REG_GET(0), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *srai_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_SRA, REG_GET(0), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *add_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_ADD, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *sub_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_SUB, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *sll_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_SLL, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *slt_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, (int32_t)REG_GET(0) < (int32_t)REG_GET(1));
  return insn->next;
}



static inline iss_insn_t *sltu_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, REG_GET(0) <REG_GET(1));
  return insn->next;
}



static inline iss_insn_t *xor_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_XOR, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *srl_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_SRL, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *sra_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_SRA, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *or_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_OR, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *and_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_AND, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fence_exec(iss *iss, iss_insn_t *insn)
{
  return insn->next;
}



static inline iss_insn_t *fence_i_exec(iss *iss, iss_insn_t *insn)
{
  return insn->next;
}



static inline iss_insn_t *sbreak_exec(iss *iss, iss_insn_t *insn)
{
  //cpu->state.error = GVSIM_STATE_DEBUG_STALL;
  return insn->next;
}



#endif
