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
#include <stdio.h>
#include <string.h>
#include <vp/itf/jtag.hpp>
#include <vp/itf/wire.hpp>

#define IDCODE_INSTR 1

typedef enum {
  JTAG_STATE_TEST_LOGIC_RESET,
  JTAG_STATE_RUN_TEST_IDLE,
  JTAG_STATE_SELECT_DR_SCAN,
  JTAG_STATE_CAPTURE_DR,
  JTAG_STATE_SHIFT_DR,
  JTAG_STATE_EXIT1_DR,
  JTAG_STATE_PAUSE_DR,
  JTAG_STATE_EXIT2_DR,
  JTAG_STATE_UPDATE_DR,
  JTAG_STATE_SELECT_IR_SCAN,
  JTAG_STATE_CAPTURE_IR,
  JTAG_STATE_SHIFT_IR,
  JTAG_STATE_EXIT1_IR,
  JTAG_STATE_PAUSE_IR,
  JTAG_STATE_EXIT2_IR,
  JTAG_STATE_UPDATE_IR
} JTAG_STATE_e;

static string tapStateName[] = {
  "Test-Logic-Reset", "Run-Test/Idle", "Select-DR-Scan", "Capture-DR", "Shift-DR", "Exit1-DR", "Pause-DR", "Exit2-DR", "Update-DR", "Select-IT-Scan", "Capture-IR", "Shift-IR", "Exit1-IR", "Pause-IR", "Exit2-IR", "Update-IR"
};

class riscv_dtm : public vp::component
{

public:

  riscv_dtm(const char *config);

  int build();
  void start();
  void reset(bool value);

private:

  static void sync(void *__this, int tck, int tdi, int tms, int trst);
  static void sync_cycle(void *__this, int tdi, int tms, int trst);
  void tck_edge(int tck, int tdi, int tms, int trst);
  void tap_update(int tms, int tclk);
  void tap_init();
  void tap_reset();
  void select_module(uint64_t command);
  void update_dr();
  void module_cmd(uint64_t dev_command);
  void burst_cmd(int word_bytes, int is_read, uint64_t command);
  void capture_dr();
  void shift_dr();

  vp::trace     trace;
  vp::trace     debug;
  vp::jtag_slave  jtag_slave_itf;
  vp::jtag_master jtag_master_itf;

  JTAG_STATE_e state;

  uint32_t tdi;
  int      dr_len;
};

riscv_dtm::riscv_dtm(const char *config)
: vp::component(config)
{

}

void riscv_dtm::tap_reset()
{
  this->state = JTAG_STATE_TEST_LOGIC_RESET;
  this->tdi = 0x57575757;
  this->dr_len = 1;
}

void riscv_dtm::reset(bool value)
{
  if (value)
    this->tap_reset();
}

static string get_instr_name(uint32_t instr) {
  if (instr == IDCODE_INSTR) return "IDCODE";
  else return "UNKNOWN";
}



void riscv_dtm::update_dr()
{
}

void riscv_dtm::capture_dr() 
{
}

void riscv_dtm::shift_dr()
{
}

void riscv_dtm::tap_update(int tms, int tclk) {

  if (!tclk)
  {    
    switch (this->state)
    {
    case JTAG_STATE_TEST_LOGIC_RESET:
      this->tap_reset();
      if (!tms) this->state = JTAG_STATE_RUN_TEST_IDLE;
      break;

    case JTAG_STATE_RUN_TEST_IDLE:
      if (tms) this->state = JTAG_STATE_SELECT_DR_SCAN;
      break;

    case JTAG_STATE_SELECT_DR_SCAN:
      if (tms) this->state = JTAG_STATE_SELECT_IR_SCAN;
      else this->state = JTAG_STATE_CAPTURE_DR;
      break;

    case JTAG_STATE_CAPTURE_DR:
      this->capture_dr();
      if (tms) this->state = JTAG_STATE_EXIT1_DR;
      else this->state = JTAG_STATE_SHIFT_DR;
      break;

    case JTAG_STATE_SHIFT_DR:
      this->shift_dr();
      if (tms) this->state = JTAG_STATE_EXIT1_DR;
      break;

    case JTAG_STATE_EXIT1_DR:
      if (tms) this->state = JTAG_STATE_UPDATE_DR;
      else this->state = JTAG_STATE_PAUSE_DR;
      break;

    case JTAG_STATE_PAUSE_DR:
      if (tms) this->state = JTAG_STATE_EXIT2_DR;
      break;

    case JTAG_STATE_EXIT2_DR:
      if (tms) this->state = JTAG_STATE_UPDATE_DR;
      else this->state = JTAG_STATE_SHIFT_DR;
      break;

    case JTAG_STATE_UPDATE_DR:
      this->update_dr();
      if (tms) this->state = JTAG_STATE_SELECT_DR_SCAN;
      else this->state = JTAG_STATE_RUN_TEST_IDLE;
      break;

    case JTAG_STATE_SELECT_IR_SCAN:
      if (tms) this->state = JTAG_STATE_TEST_LOGIC_RESET;
      else this->state = JTAG_STATE_CAPTURE_IR;
      break;

    case JTAG_STATE_CAPTURE_IR:
      if (tms) this->state = JTAG_STATE_EXIT1_IR;
      else this->state = JTAG_STATE_SHIFT_IR;
      break;

    case JTAG_STATE_SHIFT_IR:
      if (tms) this->state = JTAG_STATE_EXIT1_IR;
      break;

    case JTAG_STATE_EXIT1_IR:
      if (tms) this->state = JTAG_STATE_UPDATE_IR;
      else this->state = JTAG_STATE_PAUSE_IR;
      break;

    case JTAG_STATE_PAUSE_IR:
      if (tms) this->state = JTAG_STATE_EXIT2_IR;
      break;

    case JTAG_STATE_EXIT2_IR:
      if (tms) this->state = JTAG_STATE_UPDATE_IR;
      else this->state = JTAG_STATE_SHIFT_IR;
      break;

    case JTAG_STATE_UPDATE_IR:
      if (tms) this->state = JTAG_STATE_SELECT_DR_SCAN;
      else this->state = JTAG_STATE_RUN_TEST_IDLE;
      break;
    }
    
    trace.msg("Changing TAP state (newState: %s)\n", tapStateName[this->state].c_str());
 
  }
  else if (tclk)
  {
  }
}

void riscv_dtm::tck_edge(int tck, int tdi, int tms, int trst)
{
  trace.msg("Executing cycle (TMS_BIT: %1d, TDI_BIT: %1d, TRST_BIT: %1d, TCLK_BIT: %d)\n", tms, tdi, trst, tck);

  if (tck)
  {
    trace.msg("Syncing TDO (TDO_BIT: %1d)\n", 0);
    this->jtag_master_itf.sync(tck, this->tdi & 1, tms, trst);
  }

  this->tap_update(tms, tck);
}

void riscv_dtm::sync(void *__this, int tck, int tdi, int tms, int trst)
{
  riscv_dtm *_this = (riscv_dtm *)__this;

  // First propagate all signals to the next device on the chain
  _this->tck_edge(tck, _this->tdi & 1, tms, trst);

  // And shift in tdi in case it is a raising clock edge
  if (tck)
    _this->tdi = (_this->tdi >> 1) | (tdi << (_this->dr_len - 1));
}


void riscv_dtm::sync_cycle(void *__this, int tdi, int tms, int trst)
{
  riscv_dtm *_this = (riscv_dtm *)__this;
  _this->tck_edge(1, tdi, tms, trst);
  _this->tck_edge(0, tdi, tms, trst);
}



int riscv_dtm::build()
{
  traces.new_trace("trace", &trace, vp::TRACE);
  traces.new_trace("debug", &debug, vp::DEBUG);

  jtag_slave_itf.set_sync_meth(&riscv_dtm::sync);
  jtag_slave_itf.set_sync_cycle_meth(&riscv_dtm::sync_cycle);
  new_slave_port("jtag_in", &jtag_slave_itf);

  new_master_port("jtag_out", &jtag_master_itf);

  return 0;
}

void riscv_dtm::start()
{
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new riscv_dtm(config);
}



