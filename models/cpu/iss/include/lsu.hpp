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

#ifndef __CPU_ISS_LSU_HPP
#define __CPU_ISS_LSU_HPP

#include "iss_core.hpp"


static inline void iss_lsu_load_resume(iss *iss)
{
  // Nothing to do, the zero-extension was done by initializing the register to 0
}

static inline void iss_lsu_load_signed_resume(iss *iss)
{
  int reg = iss->cpu.state.stall_reg;
  iss->cpu.regfile.regs[reg] = iss_get_signed_value(iss->cpu.regfile.regs[reg], iss->cpu.state.stall_size*8);
}

static inline void iss_lsu_load_async(iss *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  iss->cpu.regfile.regs[reg] = 0;
  if (!iss->data_req(addr, (uint8_t *)&iss->cpu.regfile.regs[reg], size, false))
  {
    // We don't need to do anything as the target will write directly to the register
    // and we the zero extension is already managed by the initial value
  }
  else
  {
    iss->cpu.state.stall_callback = iss_lsu_load_resume;
    iss->cpu.state.stall_reg = reg;
    iss_exec_insn_stall(iss);
  }
}

static inline void iss_lsu_load_async_perf(iss *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  iss_pccr_account_event(iss, CSR_PCER_LD, 1);
  iss_lsu_load_async(iss, insn, addr, size, reg);
}

static inline void iss_lsu_load_async_signed(iss *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  if (!iss->data_req(addr, (uint8_t *)&iss->cpu.regfile.regs[reg], size, false))
  {
    iss->cpu.regfile.regs[reg] = iss_get_signed_value(iss->cpu.regfile.regs[reg], size*8);
  }
  else
  {
    iss->cpu.state.stall_callback = iss_lsu_load_signed_resume;
    iss->cpu.state.stall_reg = reg;
    iss->cpu.state.stall_size = size;
    iss_exec_insn_stall(iss);
  }
}

static inline void iss_lsu_load_async_signed_perf(iss *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  iss_pccr_account_event(iss, CSR_PCER_LD, 1);
  iss_lsu_load_async_signed(iss, insn, addr, size, reg);
}

static inline void iss_lsu_store_resume(iss *iss)
{
  // For now we don't have to do anything as the register was written directly
  // by the request but we cold support sign-extended loads here;
}

static inline void iss_lsu_store_async(iss *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  if (!iss->data_req(addr, (uint8_t *)&iss->cpu.regfile.regs[reg], size, true))
  {
    // For now we don't have to do anything as the register was written directly
    // by the request but we cold support sign-extended loads here;
  }
  else
  {
    iss->cpu.state.stall_callback = iss_lsu_store_resume;
    iss->cpu.state.stall_reg = reg;
    iss_exec_insn_stall(iss);
  }
}

static inline void iss_lsu_store_async_perf(iss *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  iss_pccr_account_event(iss, CSR_PCER_ST, 1);
  iss_lsu_store_async(iss, insn, addr, size, reg);
}

#if 0
static inline int ldAccessAligned(cpu_t *cpu, unsigned int addr, uint8_t *value, int size, int lock)
{
  int latency = 0;
  // In case the access is stalled, the latency must not be taken into account
  // It will be returned after the second call to take into account the stall time
  int err;

  if (lock){
    err = sim_read_lock(cpu, addr, value, size, &latency);
  }
  else
  {
    err = sim_read(cpu, addr, value, size, &latency);
  }

  if (err) {
#ifdef RISCV
    if (cpu->state.error == GVSIM_STATE_IO_ERROR) {
      triggerException_cause(cpu, cpu->currentPc, EXCEPTION_LSU_ERROR, ECAUSE_LOAD_FAULT);
    }
#endif
    return -1;
  }

  if (latency) {
    accountLdStalls(cpu, latency);
    cpu->cycles += latency;
  }
  perfAccountLd(cpu);
  return 0;
}

static inline int ldAccess(cpu_t *cpu, unsigned int addr, uint8_t *value, int size, int lock)
{
  int maxSize = 4 - (addr & 3);
  if (size > maxSize)
  {
    // Misaligned access
    int reqSize = maxSize;
    // Be careful in case there was a stall on the second access to not redo
    // the first access
    if (!cpu->pendingMisalignedStall)
      {
        if (ldAccessAligned(cpu, addr, value, reqSize, lock)) return -1;
        cpu->data_read_count++;
      }
    int err;
    if ((err = ldAccessAligned(cpu, addr+reqSize, value+reqSize, size-reqSize, lock))) {
      // Be careful, we can also get there in case of bad access, cancel the first access
      // for the next call only in case of stall
#ifdef GVSOC
      if (err == GV_IOREQ_STALLED) cpu->pendingMisalignedStall = 1;
      return -1;
#endif
    }
    cpu->pendingMisalignedStall = 0;
    cpu->mem_stall_count++;
    cpu->cycles++;
  }
  else
  {
    // Standard aligned access
    cpu->data_read_count++;
    if (ldAccessAligned(cpu, addr, value, size, lock)) return -1;
  }

  return 0;
}

static inline int loadByteSigned(cpu_t *cpu, pc_t *pc, unsigned int addr, void *result)
{
  uint8_t value;
  if (loadByte(cpu, pc, addr, (void *)&value)) return -1;
  *(int *)result = getSignedValue(value, 8); 
  return 0; 
}

static inline int loadByteHigh(cpu_t *cpu, pc_t *pc, unsigned int addr, void *result)
{
  uint8_t value;
  if (ldAccess(cpu, addr, (uint8_t *)&value, 1, 0)) return -1;
  *(uint32_t *)result = value << 24;
  return 0;
}

static inline int loadHalf(cpu_t *cpu, pc_t *pc, unsigned int addr, void *value)
{
  return ldAccess(cpu, addr, (uint8_t *)value, 2, 0);
}

static inline int loadHalfSigned(cpu_t *cpu, pc_t *pc, unsigned int addr, void *result)
{
  uint16_t value;
  if (loadHalf(cpu, pc, addr, (void *)&value)) return -1;
  *(int *)result = getSignedValue(value, 16);  
  return 0;
}

static inline int loadHalfHigh(cpu_t *cpu, pc_t *pc, unsigned int addr, void *result)
{
  uint16_t value;
  if (ldAccess(cpu, addr, (uint8_t *)&value, 2, 0)) return -1;
  *(uint32_t *)result = value << 16;
  return 0;
}

static inline int loadWord(cpu_t *cpu, pc_t *pc, unsigned int addr, void *value)
{
  return ldAccess(cpu, addr, (uint8_t *)value, 4, 0);
}

static inline int loadWordLock(cpu_t *cpu, pc_t *pc, unsigned int addr, void *value)
{
  return ldAccess(cpu, addr, (uint8_t *)value, 4, 1);
}

static inline int stAccessAligned(cpu_t *cpu, unsigned int addr, uint8_t *value, int size, int lock)
{
  int latency = 0;
  int err;
  int retval = 0;

  if (lock)
  {
    err = sim_write_lock(cpu, addr, value, size, &latency);
  }
  else
  {
    err = sim_write(cpu, addr, value, size, &latency);
  }

  if (err) {
#ifdef RISCV
    if (cpu->state.error == GVSIM_STATE_IO_ERROR) {
      triggerException_cause(cpu, cpu->currentPc, EXCEPTION_LSU_ERROR, ECAUSE_STORE_FAULT);
    }
#endif
    retval = -1;
  }
  cpu->cycles += latency + cpu->storeLatency;
  perfAccountSt(cpu);
  return retval;
}

static inline int stAccess(cpu_t *cpu, unsigned int addr, uint8_t *value, int size, int lock)
{
  int maxSize = 4 - (addr & 3);
  int reqSize = size > maxSize ? maxSize : size;
  if (size > maxSize)
  {
    // Misaligned access 
    cpu->data_write_count++;
    stAccessAligned(cpu, addr, value, reqSize, lock);
    if (reqSize != size) {
      cpu->mem_stall_count++;
      stAccessAligned(cpu, addr+reqSize, value+reqSize, size-reqSize, lock);
    }
  }
  else
  {
    // Standard aligned access
    cpu->data_write_count++;
    int err = stAccessAligned(cpu, addr, value, reqSize, lock);
    if (err) return -1;
  }
  return 0;
}

static inline int storeByte(cpu_t *cpu, pc_t *pc, unsigned int addr, uint8_t value)
{
  return stAccess(cpu, addr, (uint8_t *)&value, 1, 0);
}

static inline int storeHalf(cpu_t *cpu, pc_t *pc, unsigned int addr, uint16_t value)
{
  return stAccess(cpu, addr, (uint8_t *)&value, 2, 0);
}

static inline int storeWord(cpu_t *cpu, pc_t *pc, unsigned int addr, uint32_t value)
{
  return stAccess(cpu, addr, (uint8_t *)&value, 4, 0);
}

static inline int storeWordLock(cpu_t *cpu, pc_t *pc, unsigned int addr, uint32_t value, uint32_t *error)
{
  int err = stAccess(cpu, addr, (uint8_t *)&value, 4, 1);
  if (err && cpu->state.error == GVSIM_STATE_LOCK_ERROR)
  {
    *error = -1;
    cpu->state.error = 0;
    return 0;
  }
  else
  {
    *error = 0;
  }
  return err;
}

#endif

#endif
