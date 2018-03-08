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

#ifndef __CPU_ISS_RV32C_HPP
#define __CPU_ISS_RV32C_HPP

#include "iss_core.hpp"
#include "isa_lib/int.h"
#include "isa_lib/macros.h"



static inline iss_insn_t *c_addi4spn_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return addi_exec(iss, insn);
}



static inline iss_insn_t *c_lw_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return lw_exec(iss, insn);
}



static inline iss_insn_t *c_sw_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return sw_exec(iss, insn);
}



static inline iss_insn_t *c_swsp_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return sw_exec(iss, insn);
}



static inline iss_insn_t *c_nop_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return nop_exec(iss, insn);
}



static inline iss_insn_t *c_addi_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return addi_exec(iss, insn);
}


static inline iss_insn_t *c_jal_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return jal_exec(iss, insn);
}

static inline iss_insn_t *c_li_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return addi_exec(iss, insn);
}

static inline iss_insn_t *c_addi16sp_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return addi_exec(iss, insn);
}

static inline iss_insn_t *c_jalr_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return jalr_exec(iss, insn);
}

static inline iss_insn_t *c_lui_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return lui_exec(iss, insn);
}

static inline iss_insn_t *c_srli_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return srli_exec(iss, insn);
}

static inline iss_insn_t *c_srai_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return srai_exec(iss, insn);
}

static inline iss_insn_t *c_andi_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return andi_exec(iss, insn);
}

static inline iss_insn_t *c_sub_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return sub_exec(iss, insn);
}

static inline iss_insn_t *c_xor_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return xor_exec(iss, insn);
}

static inline iss_insn_t *c_or_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return or_exec(iss, insn);
}

static inline iss_insn_t *c_and_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return and_exec(iss, insn);
}

static inline iss_insn_t *c_j_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return jal_exec(iss, insn);
}

static inline iss_insn_t *c_beqz_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return beq_exec(iss, insn);
}

static inline iss_insn_t *c_bnez_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return bne_exec(iss, insn);
}

static inline iss_insn_t *c_slli_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return slli_exec(iss, insn);
}

static inline iss_insn_t *c_lwsp_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return lw_exec(iss, insn);
}

static inline iss_insn_t *c_jr_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return jalr_exec(iss, insn);
}

static inline iss_insn_t *c_mv_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return add_exec(iss, insn);
}

static inline iss_insn_t *c_add_exec(iss *iss, iss_insn_t *insn)
{
  //perfAccountRvc(cpu);
  return add_exec(iss, insn);
}

static inline iss_insn_t *c_ebreak_exec(iss *iss, iss_insn_t *insn)
{
  return insn->next;
}

static inline iss_insn_t *c_sbreak_exec(iss *iss, iss_insn_t *insn)
{
  return insn->next;
}



#endif
