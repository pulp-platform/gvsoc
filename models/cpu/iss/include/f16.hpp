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

#ifndef __CPU_ISS_F16_HPP
#define __CPU_ISS_F16_HPP

#include "iss_core.hpp"
#include "isa_lib/int.h"
#include "isa_lib/macros.h"



static inline iss_insn_t *flh_exec(iss *iss, iss_insn_t *insn)
{
  iss_lsu_load_async(iss, insn, REG_GET(0) + SIM_GET(0), 2, REG_OUT(0));
  return insn->next;
}



static inline iss_insn_t *fsh_exec(iss *iss, iss_insn_t *insn)
{
  iss_lsu_store_async(iss, insn, REG_GET(0) + SIM_GET(0), 2, REG_IN(1));
  return insn->next;
}


static inline iss_insn_t *fmadd_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL4(lib_float_madd_h_round, REG_GET(0), REG_GET(1), REG_GET(2), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fmsub_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL4(lib_float_msub_h_round, REG_GET(0), REG_GET(1), REG_GET(2), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fnmsub_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL4(lib_float_nmsub_h_round, REG_GET(0), REG_GET(1), REG_GET(2), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fnmadd_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL4(lib_float_nmadd_h_round, REG_GET(0), REG_GET(1), REG_GET(2), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fadd_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL3(lib_float_add_h_round, REG_GET(0), REG_GET(1), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fsub_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL3(lib_float_sub_h_round, REG_GET(0), REG_GET(1), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fmul_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL3(lib_float_mul_h_round, REG_GET(0), REG_GET(1), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fdiv_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL3(lib_float_div_h_round, REG_GET(0), REG_GET(1), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fsqrt_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_sqrt_h_round, REG_GET(0), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fsgnj_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_sgnj_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fsgnjn_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_sgnjn_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fsgnjx_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_sgnjx_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fmin_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_min_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fmax_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_max_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fcvt_w_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_cvt_w_h_round, REG_GET(0), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fcvt_wu_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_cvt_wu_h_round, REG_GET(0), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fmv_x_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL1(lib_float_fmv_x_s, REG_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fmv_h_x_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL1(lib_float_fmv_h_x, REG_GET(0)));
  return insn->next;
}



static inline iss_insn_t *feq_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_eq_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *flt_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_lt_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fle_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_le_s, REG_GET(0), REG_GET(1)));
  return insn->next;
}



static inline iss_insn_t *fclass_h_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL1(lib_float_class_s, REG_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fcvt_h_w_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_cvt_h_w_round, REG_GET(0), UIM_GET(0)));
  return insn->next;
}



static inline iss_insn_t *fcvt_h_wu_exec(iss *iss, iss_insn_t *insn)
{
  //REG_SET(0, LIB_CALL2(lib_float_cvt_h_wu_round, REG_GET(0), UIM_GET(0)));
  return insn->next;
}


#endif