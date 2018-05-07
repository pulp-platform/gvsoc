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

#ifndef __CPU_ISS_ISS_INSN_EXEC_HPP
#define __CPU_ISS_ISS_INSN_EXEC_HPP

iss_insn_t *iss_exec_insn_with_trace(iss *iss, iss_insn_t *insn);
void iss_trace_dump(iss *iss, iss_insn_t *insn);


static inline void iss_exec_insn_resume(iss *iss)
{
  if (iss->insn_trace.get_active())
  {
    iss_trace_dump(iss, iss->cpu.stall_insn);
  }
}

static inline void iss_exec_insn_stall(iss *iss)
{
  iss->cpu.stall_insn = iss->cpu.current_insn;
  iss->cpu.state.insn_cycles = -1;
}

static inline iss_insn_t *iss_exec_insn_handler(iss *instance, iss_insn_t *insn, iss_insn_t *(*handler)(iss *, iss_insn_t *))
{
  return handler(instance, insn);
}


static inline iss_insn_t *iss_exec_insn_fast(iss *iss, iss_insn_t *insn)
{
  return iss_exec_insn_handler(iss, insn, insn->fast_handler);
}

static inline iss_insn_t *iss_exec_insn(iss *iss, iss_insn_t *insn)
{
  return iss_exec_insn_handler(iss, insn, insn->handler);
}

static inline iss_insn_t *iss_exec_stalled_insn_fast(iss *iss, iss_insn_t *insn)
{
  iss_perf_account_ld_stall(iss);
  return iss_exec_insn_handler(iss, insn, insn->stall_fast_handler);
}

static inline iss_insn_t *iss_exec_stalled_insn(iss *iss, iss_insn_t *insn)
{
  iss_perf_account_ld_stall(iss);
  iss_pccr_account_event(iss, CSR_PCER_LD_STALL, 1);
  return iss_exec_insn_handler(iss, insn, insn->stall_handler);
}



#define ISS_EXEC_NO_FETCH_COMMON(iss,func) \
do { \
  iss->cpu.state.insn_cycles = 1; \
  iss_insn_t *insn = iss->cpu.current_insn; \
  iss->cpu.prev_insn = insn; \
  iss->cpu.current_insn = func(iss, insn); \
} while(0)


static inline int iss_exec_step_nofetch(iss_t *iss)
{
  ISS_EXEC_NO_FETCH_COMMON(iss,iss_exec_insn_fast);

  return iss->cpu.state.insn_cycles;
}


static inline int iss_exec_step_nofetch_perf(iss_t *iss)
{
  ISS_EXEC_NO_FETCH_COMMON(iss,iss_exec_insn);
  int cycles = iss->cpu.state.insn_cycles;

  if (iss->cpu.csr.pcmr & CSR_PCMR_ACTIVE)
  {
    if (cycles >= 0 && (iss->cpu.csr.pcer & (1<<CSR_PCER_CYCLES)))
    {
      iss->cpu.csr.pccr[CSR_PCER_CYCLES] += cycles;
    }

    if (iss->cpu.csr.pcer & (1<<CSR_PCER_INSTR))
      iss->cpu.csr.pccr[CSR_PCER_INSTR] += 1;
  }

  return cycles;
}


static inline int iss_exec_is_stalled(iss *iss)
{
  return iss->cpu.state.insn_cycles == -1;
}

static inline int iss_exec_step(iss_t *iss)
{
  uint64_t opcode = prefetcher_get_word(iss, 0x1a000000);
  return 1;
}

#endif
