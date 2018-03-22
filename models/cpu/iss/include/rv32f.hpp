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

#ifndef __CPU_ISS_RV32F_HPP
#define __CPU_ISS_RV32F_HPP

#include "iss_core.hpp"
#include "isa_lib/int.h"
#include "isa_lib/macros.h"



static inline iss_insn_t *fmadd_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL4(lib_float_madd_s_round, REG_GET(0), REG_GET(1), REG_GET(2), UIM_GET(0)));
  return insn->next;
}

static inline iss_insn_t *fadd_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL3(lib_float_add_s_round, REG_GET(0), REG_GET(1), UIM_GET(0)));
  return insn->next;
}

static inline iss_insn_t *fmul_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL3(lib_float_mul_s_round, REG_GET(0), REG_GET(1), UIM_GET(0)));
  return insn->next;
}

static inline iss_insn_t *fdiv_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL3(lib_float_div_s_round, REG_GET(0), REG_GET(1), UIM_GET(0)));
  return insn->next;
}

static inline iss_insn_t *fsqrt_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_float_sqrt_s_round, REG_GET(0), UIM_GET(0)));
  return insn->next;
}

static inline iss_insn_t *fmv_x_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL1(lib_float_fmv_x_s, REG_GET(0)));
  return insn->next;
}

static inline iss_insn_t *fmv_s_x_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL1(lib_float_fmv_s_x, REG_GET(0)));
  return insn->next;
}



static inline iss_insn_t *feq_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_float_eq_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *flt_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_float_lt_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fle_s_exec(iss *iss, iss_insn_t *insn)
{
  REG_SET(0, LIB_CALL2(lib_float_le_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}

#endif