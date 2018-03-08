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
    _this->is_active = false;
    _this->stalled = true;
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

void iss::data_grant(void *_this, vp::io_req *req)
{

}

void iss::data_response(void *__this, vp::io_req *req)
{
  iss *_this = (iss *)__this;
  _this->stalled = false;
  _this->cpu.state.stall_callback(_this);
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
    if (fetch_enable && !stalled)
    {
      is_active = true;
      enqueue_next_instr(1);
    }
  }
}

inline void iss::enqueue_next_instr(int cycles)
{
  if (is_active)
  {
    trace.msg("Enqueue next instruction (cycles: %d)\n", cycles);
    event_enqueue(current_event, cycles);
  }
}

void iss::irq_check()
{
  current_event = irq_event;
}


void iss::irq_req_sync(void *__this, int irq)
{
  iss *_this = (iss *)__this;
  _this->irq_check();
  iss_irq_req(_this, irq);
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

  fetch_enable = get_config_bool("fetch_enable");

  this->cpu.config.mhartid = (get_config_int("cluster_id") << 5) | get_config_int("core_id");

  iss_open(this);
  iss_pc_set(this, get_config_int("boot_addr") + 0x80);
  iss_irq_set_vector_table(this, get_config_int("boot_addr"));


}



void iss::start()
{
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
