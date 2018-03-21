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





void iss::exec_instr(void *__this, vp::clock_event *event)
{
  iss *_this = (iss *)__this;
  _this->trace.msg("Executing instruction\n");

  int cycles = iss_exec_step_nofetch(_this);
  if (cycles >= 0)
  {
    _this->enqueue_next_instr(cycles);
  }
  else
  {
    if (_this->misaligned_access)
    {
      _this->event_enqueue(_this->misaligned_event, _this->misaligned_latency);
    }
    else
    {
      _this->is_active = false;
      _this->stalled = true;      
    }
  }
}

void iss::exec_instr_check_irq(void *__this, vp::clock_event *event)
{
  iss *_this = (iss *)__this;
  _this->current_event = _this->instr_event;
  iss_irq_check(_this);
  exec_instr(__this, event);
}

void iss::exec_first_instr(vp::clock_event *event)
{
  current_event = event_new(iss::exec_instr);
  iss_start(this);
  exec_instr((void *)this, event);
}

void iss::exec_first_instr(void *__this, vp::clock_event *event)
{
  iss *_this = (iss *)__this;
  _this->exec_first_instr(event);
}

void iss::data_grant(void *__this, vp::io_req *req)
{
}

void iss::data_response(void *__this, vp::io_req *req)
{
  iss *_this = (iss *)__this;
  _this->stalled = false;
  if (_this->misaligned_access)
    _this->misaligned_access = false;
  else
  {
    _this->cpu.state.stall_callback(_this);
    iss_exec_insn_resume(_this);
  }
  _this->check_state();
}

void iss::fetch_grant(void *_this, vp::io_req *req)
{

}

void iss::fetch_response(void *_this, vp::io_req *req)
{

}

void iss::bootaddr_sync(void *__this, uint32_t value)
{
  iss *_this = (iss *)__this;
  _this->trace.msg("Setting boot address (value: 0x%x)\n", value);
  _this->bootaddr = value;
  iss_irq_set_vector_table(_this, _this->bootaddr);
}

void iss::fetchen_sync(void *__this, bool active)
{
  iss *_this = (iss *)__this;
  _this->trace.msg("Setting fetch enable (active: 0x%d)\n", active);
  _this->fetch_enable = active;
  if (active)
  {
    iss_pc_set(_this, _this->bootaddr + 0x80);
  }
  _this->check_state();
}

void iss::check_state()
{
  if (!is_active)
  {
    if (fetch_enable && !stalled && !wfi)
    {
      is_active = true;
      enqueue_next_instr(1);
    }
  }
  else
  {
    if (wfi) is_active = false;
  }
}

int iss::data_misaligned_req(iss_addr_t addr, uint8_t *data_ptr, int size, bool is_write)
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

void iss::irq_check()
{
  current_event = irq_event;
}


void iss::wait_for_interrupt()
{
  wfi = true;
  check_state();
}


void iss::irq_req_sync(void *__this, int irq)
{
  iss *_this = (iss *)__this;
  _this->irq_check();
  iss_irq_req(_this, irq);
  _this->wfi = false;
  _this->check_state();
}


void iss::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);
  traces.new_trace("decode_trace", &decode_trace, vp::DEBUG);
  traces.new_trace("insn", &insn_trace, vp::TRACE);

  data.set_resp_meth(&iss::data_response);
  data.set_grant_meth(&iss::data_grant);
  new_master_port("data", &data);

  fetch.set_resp_meth(&iss::fetch_response);
  fetch.set_grant_meth(&iss::fetch_grant);
  new_master_port("fetch", &fetch);

  bootaddr_itf.set_sync_meth(&iss::bootaddr_sync);
  new_slave_port("bootaddr", &bootaddr_itf);

  irq_req_itf.set_sync_meth(&iss::irq_req_sync);
  new_slave_port("irq_req", &irq_req_itf);
  new_master_port("irq_ack", &irq_ack_itf);

  fetchen_itf.set_sync_meth(&iss::fetchen_sync);
  new_slave_port("fetchen", &fetchen_itf);

  current_event = event_new(iss::exec_first_instr);
  instr_event = event_new(iss::exec_instr);
  irq_event = event_new(iss::exec_instr_check_irq);
  misaligned_event = event_new(iss::exec_misaligned);

  fetch_enable = get_config_bool("fetch_enable");

  this->cpu.config.mhartid = (get_config_int("cluster_id") << 5) | get_config_int("core_id");
  string isa = get_config_str("isa");
  transform(isa.begin(), isa.end(), isa.begin(),(int (*)(int))tolower);
  this->cpu.config.isa = strdup(isa.c_str());


}



void iss::start()
{
  if (iss_open(this)) throw logic_error("Error while instantiating the ISS");

  iss_pc_set(this, get_config_int("boot_addr") + 0x80);
  iss_irq_set_vector_table(this, get_config_int("boot_addr"));

  trace.msg("ISS start (fetch: %d, is_active: %d, boot_addr: 0x%lx)\n", fetch_enable, is_active, get_config_int("boot_addr"));

  check_state();
}


iss::iss(const char *config)
: vp::component(config)
{
}


extern "C" void *vp_constructor(const char *config)
{
  return (void *)new iss(config);
}
