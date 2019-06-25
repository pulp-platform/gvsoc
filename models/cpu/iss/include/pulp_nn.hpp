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
 * Authors:
 */
#ifndef  __CPU_ISS_PULP_NN_HPP
#define __CPU_ISS_PULP_NN_HPP

 #define PV_OP_RS_EXEC_NN(insn_name,lib_name)                                           \
 static inline iss_insn_t *pv_##insn_name##_n_exec(iss_t *iss, iss_insn_t *insn)                \
 {                                                                                            \
   REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_int4_t_to_int32_t, REG_GET(0), REG_GET(1)));    \
   return insn->next;                                                                         \
 }                                                                                            \
                                                                                              \
 static inline iss_insn_t *pv_##insn_name##_sc_n_exec(iss_t *iss, iss_insn_t *insn)             \
 {                                                                                            \
   REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_SC_int4_t_to_int32_t, REG_GET(0), REG_GET(1))); \
   return insn->next;                                                                         \
 }                                                                                           \
                                                                                              \
 static inline iss_insn_t *pv_##insn_name##_c_exec(iss_t *iss, iss_insn_t *insn)                \
 {                                                                                            \
   REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_int2_t_to_int32_t, REG_GET(0), REG_GET(1)));    \
   return insn->next;                                                                         \
 }                                                                                            \
                                                                                              \
 static inline iss_insn_t *pv_##insn_name##_sc_c_exec(iss_t *iss, iss_insn_t *insn)             \
 {                                                                                            \
   REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_SC_int2_t_to_int32_t, REG_GET(0), REG_GET(1))); \
   return insn->next;                                                                         \
 }


 #define PV_OP_RU_EXEC_NN(insn_name,lib_name)                                           \
 static inline iss_insn_t *pv_##insn_name##_n_exec(iss_t *iss, iss_insn_t *insn)                \
 {                                                                                            \
   REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_uint4_t_to_uint32_t, REG_GET(0), REG_GET(1)));    \
   return insn->next;                                                                         \
 }                                                                                            \
                                                                                              \
 static inline iss_insn_t *pv_##insn_name##_sc_n_exec(iss_t *iss, iss_insn_t *insn)             \
 {                                                                                            \
   REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_SC_uint4_t_to_uint32_t, REG_GET(0), REG_GET(1))); \
   return insn->next;                                                                         \
 }                                                                                           \
                                                                                               \
 static inline iss_insn_t *pv_##insn_name##_c_exec(iss_t *iss, iss_insn_t *insn)                \
 {                                                                                            \
   REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_uint2_t_to_uint32_t, REG_GET(0), REG_GET(1)));    \
   return insn->next;                                                                         \
 }                                                                                            \
                                                                                              \
 static inline iss_insn_t *pv_##insn_name##_sc_c_exec(iss_t *iss, iss_insn_t *insn)             \
 {                                                                                            \
   REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_SC_uint2_t_to_uint32_t, REG_GET(0), REG_GET(1))); \
   return insn->next;                                                                         \
 }


 PV_OP_RS_EXEC_NN(add,ADD)
 PV_OP_RS_EXEC_NN(sub,SUB)
 PV_OP_RS_EXEC_NN(avg,AVG)
 PV_OP_RU_EXEC_NN(avgu,AVGU)
 PV_OP_RS_EXEC_NN(max,MAX)
 PV_OP_RU_EXEC_NN(maxu,MAXU)
 PV_OP_RS_EXEC_NN(min,MIN)
 PV_OP_RU_EXEC_NN(minu,MINU)

// PV_OP_RS_EXEC(min,MIN)  to implement

// PV_OP_RU_EXEC(minu,MINU)  to implement

// PV_OP_RU_EXEC(srl,SRL)  to implement

// PV_OP_RS_EXEC(sra,SRA)  to implement

// PV_OP_RU_EXEC(sll,SLL)  to implement

// PV_OP_RS_EXEC(or,OR)  to implement

// PV_OP_RS_EXEC(xor,XOR)  to implement

// PV_OP_RS_EXEC(and,AND)  to implement

// PV_OP1_RS_EXEC(abs,ABS)  to implement


  #define PV_OP_RS_EXEC_NN_2(insn_name,lib_name)                                           \
  static inline iss_insn_t *pv_##insn_name##_n_exec(iss_t *iss, iss_insn_t *insn)                \
  {                                                                                            \
    REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_4, REG_GET(0), REG_GET(1)));    \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_n_sc_exec(iss_t *iss, iss_insn_t *insn)             \
  {                                                                                            \
    REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_SC_4, REG_GET(0), REG_GET(1))); \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_c_exec(iss_t *iss, iss_insn_t *insn)                \
  {                                                                                            \
    REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_2, REG_GET(0), REG_GET(1)));    \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_c_sc_exec(iss_t *iss, iss_insn_t *insn)             \
  {                                                                                            \
    REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_SC_2, REG_GET(0), REG_GET(1))); \
    return insn->next;                                                                         \
  }

  #define PV_OP_RU_EXEC_NN_2(insn_name,lib_name)                                           \
  static inline iss_insn_t *pv_##insn_name##_n_exec(iss_t *iss, iss_insn_t *insn)                \
  {                                                                                            \
    REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_4, REG_GET(0), REG_GET(1)));    \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_n_sc_exec(iss_t *iss, iss_insn_t *insn)             \
  {                                                                                            \
    REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_SC_4, REG_GET(0), REG_GET(1))); \
    return insn->next;                                                                         \
  }\
                                                                                                 \
  static inline iss_insn_t *pv_##insn_name##_c_exec(iss_t *iss, iss_insn_t *insn)                \
  {                                                                                            \
    REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_2, REG_GET(0), REG_GET(1)));    \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_c_sc_exec(iss_t *iss, iss_insn_t *insn)             \
  {                                                                                            \
    REG_SET(0, LIB_CALL2(lib_VEC_##lib_name##_SC_2, REG_GET(0), REG_GET(1))); \
    return insn->next;                                                                         \
  }

  //PV_OP_RS_EXEC_NN_2(dotsp,DOTSP)
  //PV_OP_RU_EXEC_NN_2(dotup,DOTUP)
  //PV_OP_RS_EXEC_NN_2(dotusp,DOTUSP)


  #define PV_OP_RRS_EXEC_NN_2(insn_name,lib_name)                                           \
  static inline iss_insn_t *pv_##insn_name##_n_exec(iss_t *iss, iss_insn_t *insn)                \
  {                                                                                            \
    REG_SET(0, LIB_CALL3(lib_VEC_##lib_name##_4, REG_GET(2), REG_GET(0), REG_GET(1)));    \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_n_sc_exec(iss_t *iss, iss_insn_t *insn)             \
  {                                                                                            \
    REG_SET(0, LIB_CALL3(lib_VEC_##lib_name##_SC_4, REG_GET(2), REG_GET(0), REG_GET(1))); \
    return insn->next;                                                                         \
  }                                                                                            \
  static inline iss_insn_t *pv_##insn_name##_c_exec(iss_t *iss, iss_insn_t *insn)                \
  {                                                                                            \
    REG_SET(0, LIB_CALL3(lib_VEC_##lib_name##_2, REG_GET(2), REG_GET(0), REG_GET(1)));    \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_c_sc_exec(iss_t *iss, iss_insn_t *insn)             \
  {                                                                                            \
    REG_SET(0, LIB_CALL3(lib_VEC_##lib_name##_SC_2, REG_GET(2), REG_GET(0), REG_GET(1))); \
    return insn->next;                                                                         \
  }

  PV_OP_RRS_EXEC_NN_2(sdotsp,SDOTSP)
  PV_OP_RRS_EXEC_NN_2(sdotusp,SDOTUSP)

  #define PV_OP_RRU_EXEC_NN_2(insn_name,lib_name)                                           \
  static inline iss_insn_t *pv_##insn_name##_n_exec(iss_t *iss, iss_insn_t *insn)                \
  {                                                                                            \
    REG_SET(0, LIB_CALL3(lib_VEC_##lib_name##_4, REG_GET(2), REG_GET(0), REG_GET(1)));    \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_n_sc_exec(iss_t *iss, iss_insn_t *insn)             \
  {                                                                                            \
    REG_SET(0, LIB_CALL3(lib_VEC_##lib_name##_SC_4, REG_GET(2), REG_GET(0), REG_GET(1))); \
    return insn->next;                                                                         \
  } \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_c_exec(iss_t *iss, iss_insn_t *insn)                \
  {                                                                                            \
    REG_SET(0, LIB_CALL3(lib_VEC_##lib_name##_2, REG_GET(2), REG_GET(0), REG_GET(1)));    \
    return insn->next;                                                                         \
  }                                                                                            \
                                                                                               \
  static inline iss_insn_t *pv_##insn_name##_c_sc_exec(iss_t *iss, iss_insn_t *insn)             \
  {                                                                                            \
    REG_SET(0, LIB_CALL3(lib_VEC_##lib_name##_SC_2, REG_GET(2), REG_GET(0), REG_GET(1))); \
    return insn->next;                                                                         \
  }

PV_OP_RRU_EXEC_NN_2(sdotup,SDOTUP)




#endif
