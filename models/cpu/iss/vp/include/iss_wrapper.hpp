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

#ifndef __CPU_ISS_ISS_WRAPPER_HPP
#define __CPU_ISS_ISS_WRAPPER_HPP

#include <vp/vp.hpp>
#include <vp/itf/io.hpp>
#include <vp/itf/wire.hpp>

class iss_wrapper : public vp::component
{

public:

  iss_wrapper(const char *config);

  int build();
  void start();

  static void data_grant(void *_this, vp::io_req *req);
  static void data_response(void *_this, vp::io_req *req);

  static void fetch_grant(void *_this, vp::io_req *req);
  static void fetch_response(void *_this, vp::io_req *req);

  static void exec_instr(void *__this, vp::clock_event *event);
  static void exec_first_instr(void *__this, vp::clock_event *event);
  void exec_first_instr(vp::clock_event *event);
  static void exec_instr_check_all(void *__this, vp::clock_event *event);
  static inline void exec_misaligned(void *__this, vp::clock_event *event);

  static void irq_req_sync(void *__this, int irq);

  inline int data_req(iss_addr_t addr, uint8_t *data, int size, bool is_write);
  inline int data_req_aligned(iss_addr_t addr, uint8_t *data_ptr, int size, bool is_write);
  int data_misaligned_req(iss_addr_t addr, uint8_t *data_ptr, int size, bool is_write);

  static vp::io_req_status_e dbg_unit_req(void *__this, vp::io_req *req);

  void irq_check();
  void wait_for_interrupt();
  
  void set_halt_mode(bool halted, int cause);
  void check_state();

  inline void trigger_check_all() { current_event = check_all_event; }

  vp::io_master data;
  vp::io_master fetch;
  vp::io_slave  dbg_unit;

  vp::wire_slave<int>      irq_req_itf;
  vp::wire_master<int>     irq_ack_itf;

  vp::wire_master<uint32_t> ext_counter[32];

  vp::io_req     io_req;
  vp::io_req     fetch_req;

  iss_cpu_t cpu;

  vp::trace     trace;
  vp::trace     decode_trace;
  vp::trace     insn_trace;
  vp::trace     csr_trace;
  vp::trace     perf_counter_trace;

  vp::power_trace power_trace;

  vp::power_source insn_power;
  vp::power_source clock_gated_power;

  vp::trace     pc_trace_event;
  
private:

  vp::clock_event *current_event;
  vp::clock_event *instr_event;
  vp::clock_event *check_all_event;
  vp::clock_event *misaligned_event;

  int irq_req;

  bool is_active = false;
  bool fetch_enable = false;
  bool stalled = false;
  bool wfi = false;
  bool misaligned_access = false;
  bool halted = false;
  bool step_mode = false;
  bool do_step = false;
  int halt_cause;
  int64_t wakeup_latency;
  iss_reg_t hit_reg = 0;

  iss_reg_t ppc;
  iss_reg_t npc;

  int        misaligned_size;
  uint8_t   *misaligned_data;
  iss_addr_t misaligned_addr;
  bool       misaligned_is_write;
  int64_t    misaligned_latency;

  vp::wire_slave<uint32_t> bootaddr_itf;
  vp::wire_slave<bool>     fetchen_itf;
  vp::wire_slave<bool>     halt_itf;
  vp::wire_master<bool>     halt_status_itf;

  iss_addr_t bootaddr;

  static void bootaddr_sync(void *_this, uint32_t value);
  static void fetchen_sync(void *_this, bool active);
  static void halt_sync(void *_this, bool active);
  inline void enqueue_next_instr(int64_t cycles);
  void halt_core();
};
\
inline void iss_wrapper::enqueue_next_instr(int64_t cycles)
{
  if (is_active)
  {
    trace.msg("Enqueue next instruction (cycles: %ld)\n", cycles);
    event_enqueue(current_event, cycles);
  }
}

void iss_wrapper::exec_misaligned(void *__this, vp::clock_event *event)
{
  iss_wrapper *_this = (iss_wrapper *)__this;
  if (_this->data_req_aligned(_this->misaligned_addr, _this->misaligned_data,
    _this->misaligned_size, _this->misaligned_is_write) == vp::IO_REQ_OK)
  {
    _this->misaligned_access = false;
    iss_exec_insn_resume(_this);
    _this->enqueue_next_instr(_this->io_req.get_latency() + 1);
  }
  else
  {
    _this->trace.warning("UNIMPLEMENTED AT %s %d\n", __FILE__, __LINE__);
  }
}

inline int iss_wrapper::data_req_aligned(iss_addr_t addr, uint8_t *data_ptr, int size, bool is_write)
{
  decode_trace.msg("Data request (addr: 0x%lx, size: 0x%x, is_write: %d)\n", addr, size, is_write);
  vp::io_req *req = &io_req;
  req->init();
  req->set_addr(addr);
  req->set_size(size);
  req->set_is_write(is_write);
  req->set_data(data_ptr);
  int err = data.req(req);
  if (err == vp::IO_REQ_OK) 
  {
    this->cpu.state.insn_cycles += req->get_latency();
  }
  else if (err == vp::IO_REQ_INVALID) 
  {
    this->warning.warning("Invalid access (offset: 0x%x, size: 0x%x, is_write: %d)\n", addr, size, is_write);
  }
  return err;
}

#define ADDR_MASK (~(ISS_REG_WIDTH/8 - 1))

inline int iss_wrapper::data_req(iss_addr_t addr, uint8_t *data_ptr, int size, bool is_write)
{

  iss_addr_t addr0 = addr & ADDR_MASK;
  iss_addr_t addr1 = (addr + size - 1) & ADDR_MASK;

  if (likely(addr0 == addr1))
    return data_req_aligned(addr, data_ptr, size, is_write);
  else
    return data_misaligned_req(addr, data_ptr, size, is_write);
}

#endif
