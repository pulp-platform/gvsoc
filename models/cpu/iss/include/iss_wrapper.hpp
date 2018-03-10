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

class iss : public vp::component
{

public:

  iss(const char *config);

  void build();
  void start();

  static void data_grant(void *_this, vp::io_req *req);
  static void data_response(void *_this, vp::io_req *req);

  static void fetch_grant(void *_this, vp::io_req *req);
  static void fetch_response(void *_this, vp::io_req *req);

  static void exec_instr(void *__this, vp::clock_event *event);
  static void exec_first_instr(void *__this, vp::clock_event *event);
  void exec_first_instr(vp::clock_event *event);
  static void exec_instr_check_irq(void *__this, vp::clock_event *event);

  static void irq_req_sync(void *__this, int irq);

  void irq_check();
  void wait_for_interrupt();

  vp::io_master data;
  vp::io_master fetch;

  vp::wire_slave<int>      irq_req_itf;
  vp::wire_master<int>     irq_ack_itf;

  vp::io_req     io_req;

  iss_cpu_t cpu;

  vp::trace     decode_trace;
  vp::trace     insn_trace;
  
private:

  vp::trace     trace;

  vp::clock_event *current_event;
  vp::clock_event *instr_event;
  vp::clock_event *irq_event;

  bool is_active = false;
  bool fetch_enable = false;
  bool stalled = false;
  bool wfi = false;

  vp::wire_slave<uint32_t> bootaddr_itf;
  vp::wire_slave<bool>     fetchen_itf;

  iss_addr_t bootaddr;

  void check_state();
  static void bootaddr_sync(void *_this, uint32_t value);
  static void fetchen_sync(void *_this, bool active);
  inline void enqueue_next_instr(int cycles);
};

typedef iss iss_t;

#endif
