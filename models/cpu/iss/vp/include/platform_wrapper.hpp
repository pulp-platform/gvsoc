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

#ifndef __CPU_ISS_PLATFORM_WRAPPER_HPP
#define __CPU_ISS_PLATFORM_WRAPPER_HPP

#include "types.hpp"
#include "iss_wrapper.hpp"
#include <string.h>

static inline void iss_handle_ecall(iss_t *iss, iss_insn_t *insn)
{

}

static inline void iss_set_halt_mode(iss_t *iss, bool halted, int cause)
{
  iss->set_halt_mode(halted, cause);
  iss->check_state();
}

static inline void iss_wait_for_interrupt(iss_t *iss)
{
  iss->wait_for_interrupt();
}

static inline void iss_trigger_check_all(iss_t *iss)
{
  iss->trigger_check_all();
}

static inline bool iss_insn_trace_active(iss_t *iss)
{
  return iss->insn_trace.get_active();
}

static bool iss_csr_ext_counter_is_bound(iss_t *iss, int id)
{
  return iss->ext_counter[id].is_bound();
}

#define iss_decoder_msg(iss, fmt, x...) \
  do { \
    iss->decode_trace.msg(fmt, ##x); \
  } while(0)

#define iss_msg(iss, fmt, x...) \
  do { \
    iss->trace.msg(fmt, ##x); \
  } while(0)

#define iss_warning(iss, fmt, x...) \
  do { \
    iss->trace.warning(fmt, ##x); \
  } while(0)

#define iss_csr_msg(iss, fmt, x...) \
  do { \
    iss->csr_trace.msg(fmt, ##x); \
  } while(0)

#define iss_insn_msg(iss, fmt, x...) \
  do { \
    iss->insn_trace.msg(fmt, ##x); \
  } while(0)

#define iss_decoder_msg(iss, fmt, x...) \
  do { \
    iss->decode_trace.msg(fmt, ##x); \
  } while(0)

#define iss_perf_counter_msg(iss, fmt, x...) \
  do { \
    iss->perf_counter_trace.msg(fmt, ##x); \
  } while(0)

static inline void iss_trigger_irq_check(iss_t *iss)
{
  iss->irq_check();
}

static inline int iss_io_req(iss_t *_this, uint64_t addr, uint8_t *data, uint64_t size, bool is_write)
{
  return _this->data.req(&_this->io_req);
}

static inline int iss_fetch_req(iss_t *_this, uint64_t addr, uint8_t *data, uint64_t size, bool is_write)
{
  vp::io_req *req = &_this->fetch_req;
  req->init();
  req->set_addr(addr);
  req->set_size(size);
  req->set_is_write(is_write);
  req->set_data(data);
  if (_this->fetch.req(req))
  {
    _this->trace.force_warning("Unimplemented pending fetch request\n");
    return -1;
  }
  return 0;
}

static inline int iss_irq_ack(iss_t *iss, int irq)
{
  iss->decode_trace.msg("Acknowledging interrupt (irq: %d)\n", irq);
  iss->irq_ack_itf.sync(irq);
}

static inline void iss_init(iss_t *iss)
{
  iss->io_req.set_data(new uint8_t[sizeof(iss_reg_t)]);
}


static inline void iss_csr_ext_counter_set(iss_t *iss, int id, unsigned int value)
{
  if (!iss->ext_counter[id].is_bound())
  {
    iss_warning(iss, "Trying to access external counter through CSR while it is not connected (id: %d)\n", id);
  }
  else
  {
    iss->ext_counter[id].sync(value);
  }
}

static inline void iss_csr_ext_counter_get(iss_t *iss, int id, unsigned int *value)
{
  if (!iss->ext_counter[id].is_bound())
  {
    iss_warning(iss, "Trying to access external counter through CSR while it is not connected (id: %d)\n", id);
  }
  else
  {
    iss->ext_counter[id].sync_back(value);
  }
}

static inline void iss_lsu_load_resume(iss_t *iss)
{
  // Nothing to do, the zero-extension was done by initializing the register to 0
}

static inline void iss_lsu_load_signed_resume(iss_t *iss)
{
  int reg = iss->cpu.state.stall_reg;
  iss_set_reg(iss, reg, iss_get_signed_value(iss_get_reg_untimed(iss, reg), iss->cpu.state.stall_size*8));
}

static inline void iss_lsu_load(iss_t *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  iss_set_reg(iss, reg, 0);
  if (!iss->data_req(addr, (uint8_t *)iss_reg_ref(iss, reg), size, false))
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

static inline void iss_lsu_load_signed(iss_t *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  if (!iss->data_req(addr, (uint8_t *)iss_reg_ref(iss, reg), size, false))
  {
    iss_set_reg(iss, reg, iss_get_signed_value(iss_get_reg_untimed(iss, reg), size*8));
  }
  else
  {
    iss->cpu.state.stall_callback = iss_lsu_load_signed_resume;
    iss->cpu.state.stall_reg = reg;
    iss->cpu.state.stall_size = size;
    iss_exec_insn_stall(iss);
  }
}

static inline void iss_lsu_store_resume(iss_t *iss)
{
  // For now we don't have to do anything as the register was written directly
  // by the request but we cold support sign-extended loads here;
}

static inline void iss_lsu_store(iss_t *iss, iss_insn_t *insn, iss_addr_t addr, int size, int reg)
{
  if (!iss->data_req(addr, (uint8_t *)iss_reg_store_ref(iss, reg), size, true))
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

#endif
