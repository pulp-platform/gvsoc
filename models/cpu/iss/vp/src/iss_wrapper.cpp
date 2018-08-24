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

#include <vp/vp.hpp>
#include <vp/itf/io.hpp>
#include "iss.hpp"
#include <algorithm>

#define HALT_CAUSE_EBREAK    0
#define HALT_CAUSE_ECALL     1
#define HALT_CAUSE_ILLEGAL   2
#define HALT_CAUSE_INVALID   3
#define HALT_CAUSE_INTERRUPT 4 
#define HALT_CAUSE_HALT      15
#define HALT_CAUSE_STEP      15


#define EXEC_INSTR_COMMON(_this, event, func) \
do { \
  _this->trace.msg("Executing instruction\n"); \
 \
  if (_this->pc_trace_event.get_event_active()) \
  { \
    _this->pc_trace_event.event((uint8_t *)&_this->cpu.current_insn->addr); \
  } \
  if (_this->power_trace.get_active()) \
  { \
  _this->insn_power.account(); \
 } \
 \
  int cycles = func(_this); \
  if (cycles >= 0) \
  { \
    _this->enqueue_next_instr(cycles); \
  } \
  else \
  { \
    if (_this->misaligned_access) \
    { \
      _this->event_enqueue(_this->misaligned_event, _this->misaligned_latency); \
    } \
    else \
    { \
      _this->is_active = false; \
      _this->stalled = true;     \
    } \
  } \
} while(0)

void iss_wrapper::exec_instr(void *__this, vp::clock_event *event)
{
  iss_t *_this = (iss_t *)__this;

  EXEC_INSTR_COMMON(_this, event, iss_exec_step_nofetch);
}

void iss_wrapper::exec_instr_check_all(void *__this, vp::clock_event *event)
{
  iss_t *_this = (iss_t *)__this;

  // Switch back to optimize instruction handler only
  // if HW counters are disabled as they are checked with the slow handler
  if (iss_exec_switch_to_fast(_this))
  {
    _this->current_event = _this->instr_event;
  }

  EXEC_INSTR_COMMON(_this, event, iss_exec_step_nofetch_perf);
  if (_this->step_mode)
  {
    _this->do_step = false;
    _this->hit_reg |= 1;
    _this->set_halt_mode(true, HALT_CAUSE_HALT);
    _this->check_state();
  }
}

void iss_wrapper::exec_first_instr(vp::clock_event *event)
{
  current_event = event_new(iss_wrapper::exec_instr);
  iss_start(this);
  exec_instr((void *)this, event);
}

void iss_wrapper::exec_first_instr(void *__this, vp::clock_event *event)
{
  iss_t *_this = (iss_t *)__this;
  _this->exec_first_instr(event);
}

void iss_wrapper::data_grant(void *__this, vp::io_req *req)
{
}

void iss_wrapper::data_response(void *__this, vp::io_req *req)
{
  iss_t *_this = (iss_t *)__this;
  _this->stalled = false;
  _this->wakeup_latency = req->get_latency();
  if (_this->misaligned_access)
    _this->misaligned_access = false;
  else
  {
    // First call the ISS to finish the instruction
    _this->cpu.state.stall_callback(_this);
    iss_exec_insn_resume(_this);
  }
  _this->check_state();
}

void iss_wrapper::fetch_grant(void *_this, vp::io_req *req)
{

}

void iss_wrapper::fetch_response(void *_this, vp::io_req *req)
{

}

void iss_wrapper::bootaddr_sync(void *__this, uint32_t value)
{
  iss_t *_this = (iss_t *)__this;
  _this->trace.msg("Setting boot address (value: 0x%x)\n", value);
  _this->bootaddr = value;
  iss_irq_set_vector_table(_this, _this->bootaddr);
}

void iss_wrapper::fetchen_sync(void *__this, bool active)
{
  iss_t *_this = (iss_t *)__this;
  _this->trace.msg("Setting fetch enable (active: %d)\n", active);
  _this->fetch_enable = active;
  if (active)
  {
    iss_pc_set(_this, _this->bootaddr + 0x80);
  }
  _this->check_state();
}



void iss_wrapper::set_halt_mode(bool halted, int cause)
{
  this->halt_cause = cause;

  if (this->halted && !halted)
  {
    this->get_clock()->release();
  }
  else if (!this->halted && halted)
  {
    this->get_clock()->retain();
  }

  this->halted = halted;
 
  if (this->halt_status_itf.is_bound()) 
    this->halt_status_itf.sync(this->halted);
}



void iss_wrapper::halt_core()
{
  this->trace.msg("Halting core\n");

  if (this->cpu.prev_insn == NULL)
    this->ppc = 0;
  else
    this->ppc = this->cpu.prev_insn->addr;
  this->npc = this->cpu.current_insn->addr;
}



void iss_wrapper::halt_sync(void *__this, bool halted)
{
  iss_t *_this = (iss_t *)__this;
  _this->trace.msg("Received halt signal sync (halted: 0x%d)\n", halted);
  _this->set_halt_mode(halted, HALT_CAUSE_HALT);

  _this->check_state();
}


void iss_wrapper::check_state()
{
  vp::clock_event *event = current_event;

  current_event = check_all_event;

  if (!is_active)
  {
    if (!halted && fetch_enable && !stalled && (!wfi || irq_req != -1))
    {
      wfi = false;
      is_active = true;
      if (step_mode)
        do_step = true;
      enqueue_next_instr(1 + this->wakeup_latency);

      if (this->cpu.csr.pcmr & CSR_PCMR_ACTIVE && this->cpu.csr.pcer & (1<<CSR_PCER_CYCLES))
      {
        this->cpu.csr.pccr[CSR_PCER_CYCLES] += 1 + this->wakeup_latency;
      }

      this->wakeup_latency = 0;
    }
  }
  else
  {
    if (halted && !do_step)
    {
      is_active = false;
      this->halt_core();
    }
    else if (wfi)
    {
      if (irq_req == -1)
        is_active = false;
      else
        wfi = false;
    }

    if (!is_active)
    {
      if (event->is_enqueued())
      {
        event_cancel(event);
      }
    }
  }
}

int iss_wrapper::data_misaligned_req(iss_addr_t addr, uint8_t *data_ptr, int size, bool is_write)
{

  iss_addr_t addr0 = addr & ADDR_MASK;
  iss_addr_t addr1 = (addr + size - 1) & ADDR_MASK;

  decode_trace.msg("Misaligned data request (addr: 0x%lx, size: 0x%x, is_write: %d)\n", addr, size, is_write);

  // The access is a misaligned access
  // Change the event so that we can do the first access now and the next access
  // during the next cycle
  int size0 = addr1 - addr;
  int size1 = size - size0;
  
  misaligned_access = true;

  // Remember the access properties for the second access
  misaligned_size = size1;
  misaligned_data = data_ptr + size0;
  misaligned_addr = addr1;
  misaligned_is_write = is_write;

  // And do the first one now
  int err = data_req_aligned(addr, data_ptr, size0, is_write);
  if (err == vp::IO_REQ_OK)
  {
    // As the transaction is split into 2 parts, we must tell the ISS
    // that the access is pending as the instruction must be executed
    // only when the second access is finished.
    misaligned_latency = io_req.get_latency() + 1;
    iss_exec_insn_stall(this);
    return vp::IO_REQ_PENDING;
  }
  else
  {
    trace.warning("UNIMPLEMENTED AT %s %d\n", __FILE__, __LINE__);
  }
}

void iss_wrapper::irq_check()
{
  current_event = check_all_event;
}


void iss_wrapper::wait_for_interrupt()
{
  wfi = true;
  check_state();
}


void iss_wrapper::irq_req_sync(void *__this, int irq)
{
  iss_t *_this = (iss_t *)__this;
  _this->irq_req = irq;
  _this->irq_check();
  iss_irq_req(_this, irq);
  _this->wfi = false;
  _this->check_state();
}

vp::io_req_status_e iss_wrapper::dbg_unit_req(void *__this, vp::io_req *req)
{
  iss_t *_this = (iss_t *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();

  _this->trace.msg("IO access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, req->get_is_write());

  if (size != ISS_REG_WIDTH/8)
    return vp::IO_REQ_INVALID;

  if (offset >= 0x4000)
  {
    offset -= 0x4000;

    if (size != 4) return vp::IO_REQ_INVALID;


    bool err;
    if (req->get_is_write())
      err = iss_csr_write(_this, offset / 4, *(iss_reg_t *)data);
    else
      err = iss_csr_read(_this, offset / 4, (iss_reg_t *)data);

    if (err) return vp::IO_REQ_INVALID;
  }
  else if (offset >= 0x2000)
  {
    if (!_this->halted)
    {
      _this->trace.warning("Trying to access debug registers while core is not halted\n");
      return vp::IO_REQ_INVALID;
    }

    if (offset == 0x2000)
    {
      if (req->get_is_write())
      {
        // Writing NPC will force the core to jump to the written PC
        // even if the core is sleeping
        iss_cache_flush(_this);
        _this->npc = *(iss_reg_t *)data;
        iss_pc_set(_this, _this->npc);
        _this->wfi = false;
        _this->check_state();
      }
      else
        *(iss_reg_t *)data = _this->npc;
    }
    else if (offset == 0x2004)
    {
      if (req->get_is_write())
        _this->trace.warning("UNIMPLEMENTED AT %s %d\n", __FILE__, __LINE__);
      else
        *(iss_reg_t *)data = _this->ppc;
    }
    else
    {
      return vp::IO_REQ_INVALID;
    }
  }
  else if (offset >= 0x400)
  {
    offset -= 0x400;
    int reg_id = offset / 4;

    if (!_this->halted)
    {
      _this->trace.warning("Trying to access GPR while core is not halted\n");
      return vp::IO_REQ_INVALID;
    }

    if (reg_id >= ISS_NB_REGS)
      return vp::IO_REQ_INVALID;

    if (req->get_is_write())
      iss_set_reg(_this, reg_id, *(iss_reg_t *)data);
    else
      *(iss_reg_t *)data = iss_get_reg(_this, reg_id);
  }
  else if (offset < 0x80)
  {
    if (offset == 0x00)
    {
      if (req->get_is_write())
      {
        bool step_mode = (*(iss_reg_t *)data) & 1;
        bool halt_mode = ((*(iss_reg_t *)data) >> 16) & 1;
        _this->trace.msg("Writing DBG_CTRL (value: 0x%x, halt: %d, step: %d)\n", *(iss_reg_t *)data, halt_mode, step_mode);

        _this->set_halt_mode(halt_mode, HALT_CAUSE_HALT);
        _this->step_mode = step_mode;

        _this->check_state();
      }
      else
      {
        *(iss_reg_t *)data = (_this->halted << 16) | _this->step_mode;
      }
    }
    else if (offset == 0x04)
    {
      if (req->get_is_write())
      {
        _this->hit_reg = *(iss_reg_t *)data;
      }
      else
      {
        *(iss_reg_t *)data = _this->hit_reg;
      }
    }
    else if (offset == 0x0C)
    {
      if (req->get_is_write())
        return vp::IO_REQ_INVALID;

      *(iss_reg_t *)data = _this->halt_cause;
    }
  }
  else
  {
    _this->trace.warning("UNIMPLEMENTED AT %s %d\n", __FILE__, __LINE__);
    return vp::IO_REQ_INVALID;
  }

  return vp::IO_REQ_OK;
}


int iss_wrapper::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);
  traces.new_trace("decode_trace", &decode_trace, vp::DEBUG);
  traces.new_trace("insn", &insn_trace, vp::TRACE);
  traces.new_trace("csr", &csr_trace, vp::TRACE);
  traces.new_trace("perf", &perf_counter_trace, vp::TRACE);

  traces.new_trace_event("pc", &pc_trace_event, 32);

  if (power.new_trace("power_trace", &power_trace)) return -1;
  if (power.new_event("power_insn", &insn_power, this->get_js_config()->get("**/insn"), &power_trace)) return -1;
  if (power.new_event("power_clock_gated", &clock_gated_power, this->get_js_config()->get("**/clock_gated"), &power_trace)) return -1;

  data.set_resp_meth(&iss_wrapper::data_response);
  data.set_grant_meth(&iss_wrapper::data_grant);
  new_master_port("data", &data);

  fetch.set_resp_meth(&iss_wrapper::fetch_response);
  fetch.set_grant_meth(&iss_wrapper::fetch_grant);
  new_master_port("fetch", &fetch);

  dbg_unit.set_req_meth(&iss_wrapper::dbg_unit_req);
  new_slave_port("dbg_unit", &dbg_unit);

  bootaddr_itf.set_sync_meth(&iss_wrapper::bootaddr_sync);
  new_slave_port("bootaddr", &bootaddr_itf);

  irq_req_itf.set_sync_meth(&iss_wrapper::irq_req_sync);
  new_slave_port("irq_req", &irq_req_itf);
  new_master_port("irq_ack", &irq_ack_itf);

  fetchen_itf.set_sync_meth(&iss_wrapper::fetchen_sync);
  new_slave_port("fetchen", &fetchen_itf);

  halt_itf.set_sync_meth(&iss_wrapper::halt_sync);
  new_slave_port("halt", &halt_itf);

  new_master_port("halt_status", &halt_status_itf);

  for (int i=0; i<32; i++)
  {
    new_master_port("ext_counter[" + std::to_string(i) + "]", &ext_counter[i]);
  }

  current_event = event_new(iss_wrapper::exec_first_instr);
  instr_event = event_new(iss_wrapper::exec_instr);
  check_all_event = event_new(iss_wrapper::exec_instr_check_all);
  misaligned_event = event_new(iss_wrapper::exec_misaligned);

  this->fetch_enable = get_js_config()->get("fetch_enable")->get_bool();

  this->bootaddr = get_config_int("boot_addr");

  this->cpu.config.mhartid = (get_config_int("cluster_id") << 5) | get_config_int("core_id");
  string isa = get_config_str("isa");
  //transform(isa.begin(), isa.end(), isa.begin(),(int (*)(int))tolower);
  this->cpu.config.isa = strdup(isa.c_str());

  return 0;
}



void iss_wrapper::start()
{

  vp_assert_always(this->data.is_bound(), &this->trace, "Data master port is not connected\n");
  vp_assert_always(this->fetch.is_bound(), &this->trace, "Fetch master port is not connected\n");
  vp_assert_always(this->irq_ack_itf.is_bound(), &this->trace, "IRQ ack master port is not connected\n");



  if (iss_open(this)) throw logic_error("Error while instantiating the ISS");

  iss_pc_set(this, get_config_int("boot_addr") + 0x80);
  iss_irq_set_vector_table(this, get_config_int("boot_addr"));

  trace.msg("ISS start (fetch: %d, is_active: %d, boot_addr: 0x%lx)\n", fetch_enable, is_active, get_config_int("boot_addr"));

  irq_req = -1;
  wakeup_latency = 0;

  check_state();
}


iss_wrapper::iss_wrapper(const char *config)
: vp::component(config)
{
}


extern "C" void *vp_constructor(const char *config)
{
  return (void *)new iss_wrapper(config);
}
