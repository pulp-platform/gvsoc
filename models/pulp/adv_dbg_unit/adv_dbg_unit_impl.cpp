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

#define TCLK_BIT (1)
#define TRST_BIT (2)
#define TDI_BIT  (3)
#define TMS_BIT  (4)
#define TDO_BIT  (6)

#define IDCODE_INSTR 1
#define USER_INSTR   4

#define INSTR_LENGTH 8

#define ADBG_CRC_POLY 0xedb88320

typedef enum {
  TAP_STATE_TEST_LOGIC_RESET,
  TAP_STATE_RUN_TEST_IDLE,
  TAP_STATE_SELECT_DR_SCAN,
  TAP_STATE_CAPTURE_DR,
  TAP_STATE_SHIFT_DR,
  TAP_STATE_EXIT1_DR,
  TAP_STATE_PAUSE_DR,
  TAP_STATE_EXIT2_DR,
  TAP_STATE_UPDATE_DR,
  TAP_STATE_SELECT_IR_SCAN,
  TAP_STATE_CAPTURE_IR,
  TAP_STATE_SHIFT_IR,
  TAP_STATE_EXIT1_IR,
  TAP_STATE_PAUSE_IR,
  TAP_STATE_EXIT2_IR,
  TAP_STATE_UPDATE_IR
} tap_state_e;

static string tapStateName[] = {
  "Test-Logic-Reset", "Run-Test/Idle", "Select-DR-Scan", "Capture-DR", "Shift-DR", "Exit1-DR", "Pause-DR", "Exit2-DR", "Update-DR", "Select-IT-Scan", "Capture-IR", "Shift-IR", "Exit1-IR", "Pause-IR", "Exit2-IR", "Update-IR"
};

typedef struct {
  uint64_t command;
  uint32_t shift_data;
  int do_burst;
  uint32_t burst_addr;
  uint32_t burst_count;
  uint32_t burst_word_count;
  uint32_t burst_word_bytes;
  uint32_t burst_word;
  int reg_burst;
  uint32_t burst_is_read;
} device_t;

typedef struct {
  tap_state_e state;
  int tclk;
  int module;
  uint32_t id_reg;
  uint32_t instr; // TODO check how many bits there are in real hardware
  uint32_t capture_instr;
  uint32_t ctrl_reg;
} tap_t;

class adv_dbg_unit : public vp::component
{

public:

  adv_dbg_unit(const char *config);

  void build();
  void start();

private:

  static void sync(void *__this, int tck, int tdi, int tms, int trst);
  static void sync_cycle(void *__this, int tdi, int tms, int trst);
  void tck_edge(int tck, int tdi, int tms, int trst);
  void tap_update(int tms, int tclk);
  void tap_init();
  void select_module();
  void update_dr();
  void module_cmd();
  void burst_cmd(int word_bytes, int is_read, uint64_t command);
  void capture_dr();
  void shift_dr();

  vp::trace     trace;
  vp::jtag_slave jtag_itf;

  vp::io_req req;

  tap_t tap;
  device_t dev;
  int tdi;
  int tdo;

  vp::io_master io_itf;
};

adv_dbg_unit::adv_dbg_unit(const char *config)
: vp::component(config)
{

}

void adv_dbg_unit::tap_init() {
  tap.state = TAP_STATE_TEST_LOGIC_RESET;
  tap.tclk = 0;
  tap.id_reg = 1;
  tap.instr = IDCODE_INSTR;
  dev.do_burst = 0;
  trace.msg("Updating instruction (newInstr: IDCODE)\n");
}

static string get_instr_name(uint32_t instr) {
  if (instr == IDCODE_INSTR) return "IDCODE";
  else if (instr == USER_INSTR) return "USER";
  else return "UNKNOWN";
}

void adv_dbg_unit::select_module() {
  uint32_t command = dev.command >> (64 - 6);
  uint32_t module_id = command & 0x1f;
  trace.msg("Selecting new module (module: %d)\n", module_id);
  tap.module = module_id;
}

void adv_dbg_unit::burst_cmd(int word_bytes, int is_read, uint64_t command) {
  dev.do_burst = 1;
  dev.burst_is_read = is_read;
  dev.burst_word_bytes = word_bytes;
  dev.burst_addr = (command >> 16) & 0xffffffff;
  dev.burst_count = (command & 0xffff);
  dev.burst_word_count = 0;
  dev.reg_burst = 0;
  trace.msg("Handling Burst Setup command (isRead: %d, addr: 0x%x, count: 0x%x, wordBytes: %d)\n", is_read, dev.burst_addr, dev.burst_count, word_bytes);
}

void adv_dbg_unit::module_cmd()
{
  uint32_t opcode = (dev.command >> (64 - 5)) & 0xf;
  trace.msg("Handling module command (module: %d, opcode: %d)\n", tap.module, opcode);

  if (tap.module == 1) {

    //int cpuId = (dev.command >> (64 - 9)) & 0xf;
    // TODO do something with cpuId

    switch (opcode) {
    case 0: {
      trace.msg("Received NOP command\n");
      break;
    }
    case 3: {
      uint32_t command = dev.command >> (64 - 57);
      dev.do_burst = 1;
      dev.burst_is_read = 0;
      dev.burst_addr = (command >> 16) & 0xffffffff;
      dev.burst_count = (command & 0xffff);
      dev.burst_word_count = 0;
      dev.burst_word_bytes = 4;
      trace.msg("Received Burst Setup Write command (addr: 0x%x, count: 0x%x)\n", dev.burst_addr, dev.burst_count);
      break;
    }
    case 7: {
      uint32_t command = dev.command >> (64 - 57);
      dev.do_burst = 1;
      dev.burst_is_read = 1;
      dev.burst_addr = (command >> 16) & 0xffffffff;
      dev.burst_count = (command & 0xffff);
      dev.burst_word_count = 0;
      dev.burst_word_bytes = 4;
      trace.msg("Received Burst Setup Read command (addr: 0x%x, count: 0x%x)\n", dev.burst_addr, dev.burst_count);
      break;
    }   
    case 9: {
      uint32_t data = (dev.command >> (64 - 12)) & (0x3);
      trace.msg("Received Internal Register Write command (data: %x)\n", data);
      // TODO support all cluster cores
      //cpuCtrl[0]->regAccess(-1, 0, &data);
      break;
    }  
    case 13: {
      trace.msg("Received Internal Register Select command (addr: 0x%x, count: 0x%x)\n");
      break;
    } 
    }
  } else if (tap.module == 0) {
    switch (opcode) {
    case 0: {
      trace.msg("Received NOP command\n");
      break;
    }
    case 1:
      burst_cmd(1, 0, dev.command >> (64 - 53));
      break;
    case 2:
      burst_cmd(2, 0, dev.command >> (64 - 53));
      break;
    case 3:
      burst_cmd(4, 0, dev.command >> (64 - 53));
      break;
    case 5:
      burst_cmd(1, 1, dev.command >> (64 - 53));
      break;
    case 6:
      burst_cmd(2, 1, dev.command >> (64 - 53));
      break;
    case 7:
      burst_cmd(4, 1, dev.command >> (64 - 53));
      break;
    case 13:
      trace.msg("Received Internal Register Select command (addr: 0x%x, count: 0x%x)\n");
      break;
    }
  }
}

void adv_dbg_unit::update_dr()
{
  if (tap.instr == USER_INSTR) {
    if (dev.command & (1ULL<<63)) {
      select_module();
    } else {
      module_cmd();
    }
    dev.command = 0;
  }
}

void adv_dbg_unit::capture_dr() 
{
  if (tap.module == 1) {
      // TODO support all cluster cores
      //cpuCtrl[0]->regAccess(-1, 1, &tap.ctrl_reg);
  } else if (tap.module == 0) {
    tap.ctrl_reg = 0;
  }
}

static uint32_t adbg_compute_crc(uint32_t crc_in, uint32_t data_in, int length_bits)
{
  int i;
  unsigned int d, c;
  uint32_t crc_out = crc_in;
  
  for(i = 0; i < length_bits; i = i+1) 
    {
      d = ((data_in >> i) & 0x1) ? 0xffffffff : 0;
      c = (crc_out & 0x1) ? 0xffffffff : 0;
      crc_out = crc_out >> 1;
      crc_out = crc_out ^ ((d ^ c) & ADBG_CRC_POLY);
    }
  return crc_out;
}

void adv_dbg_unit::shift_dr()
{
  if (tap.instr == IDCODE_INSTR) {
    tap.id_reg = (tap.id_reg >> 1) | (tdi << 31);
    //vpi_out = tap.id_reg & 1;
  } else if (tap.instr == USER_INSTR) {

    if (dev.do_burst) { 
      static uint32_t crc_calc;
      
      if (dev.burst_is_read) {
        if (dev.do_burst == 1) {
          tdo = 1;
          dev.do_burst = 2;
          crc_calc = 0xffffffff;
        } else if (dev.do_burst == 2) {
          if (dev.burst_word_count == 0) {
            if (tap.module == 0) { 
              req.init();
              req.set_data((uint8_t *)&dev.burst_word);
              req.set_addr(dev.burst_addr);
              req.set_size(dev.burst_word_bytes);
              req.set_is_write(0);
              int err = io_itf.req(&req);
              if (err != vp::IO_REQ_OK)
              {
                trace.warning("UNIMPLEMENTED AT %s %d\n", __FILE__, __LINE__);
              }
            } else {
        // TODO support all cluster cores
        //cpuCtrl[0]->regAccess(dev.burst_addr, 1, &dev.burst_word);
            }
            trace.msg("Sending burst word (value: %x)\n", dev.burst_word);
            crc_calc = adbg_compute_crc(crc_calc, dev.burst_word, dev.burst_word_bytes*8);
            dev.burst_word_count = dev.burst_word_bytes*8;
          }

          tdo = dev.burst_word & 1;
          dev.burst_word >>= 1;

          dev.burst_word_count--;
          if (dev.burst_word_count == 0) {
            dev.burst_addr += dev.burst_word_bytes;
            dev.burst_count--;
            if (dev.burst_count == 0) {
              dev.do_burst = 3;
              dev.burst_word_count = 32;
              dev.burst_word = crc_calc;
              trace.msg("Sending burst CRC (value: %x)\n", crc_calc);
            }
          }
        } else if (dev.do_burst == 3) {
          tdo = dev.burst_word & 1;
          dev.burst_word >>= 1;
          dev.burst_word_count--;
          if (dev.burst_word_count == 0) {
            dev.do_burst = 4;
          }
        } else if (dev.do_burst == 4) {
          dev.do_burst = 0;
        }
      } else {
        if (dev.do_burst == 1) {
          if (tdi == 1) {
            dev.do_burst = 2;
            crc_calc = 0xffffffff;
          }
        } else if (dev.do_burst == 2) {
          if (dev.burst_word_count == 0) {
            dev.burst_word_count = dev.burst_word_bytes*8;
          }

          dev.burst_word >>= 1;
          dev.burst_word |= tdi << (dev.burst_word_bytes*8-1);
          dev.burst_word_count--;
          if (dev.burst_word_count == 0) {
            trace.msg("Received burst word (value: %x)\n", dev.burst_word);
            if (tap.module == 0) {
              req.init();
              req.set_data((uint8_t *)&dev.burst_word);
              req.set_addr(dev.burst_addr);
              req.set_size(dev.burst_word_bytes);
              req.set_is_write(1);
              int err = io_itf.req(&req);
              if (err != vp::IO_REQ_OK)
              {
                trace.warning("UNIMPLEMENTED AT %s %d\n", __FILE__, __LINE__);
              }
            } else {
        // TODO support all cluster cores
        //cpuCtrl[0]->regAccess(dev.burst_addr, 0, &dev.burst_word);
            }
            crc_calc = adbg_compute_crc(crc_calc, dev.burst_word, dev.burst_word_bytes*8);
            dev.burst_addr += dev.burst_word_bytes;
            dev.burst_count--;
            if (dev.burst_count == 0) {
              dev.do_burst = 3;
              dev.burst_word_count = 32;
            }
          }
        } else if (dev.do_burst == 3) {
          dev.burst_word >>= 1;
          dev.burst_word |= tdi << 31;
          dev.burst_word_count--;
          if (dev.burst_word_count == 0) {
            dev.do_burst = 4;
            tdo = crc_calc == dev.burst_word;
            trace.msg("Received burst CRC (value: %x)\n", dev.burst_word);
          }
        } else if (dev.do_burst == 4) {
          dev.do_burst = 0;
        }
      }
    } else {
      dev.command >>= 1;
      dev.command |= ((uint64_t)tdi) << 63;
    }
  }
  else
  {
    tdo = dev.shift_data & 1;
    dev.shift_data >>= 1;
    dev.shift_data |= ((uint64_t)tdi) << 31;

  }
}

void adv_dbg_unit::tap_update(int tms, int tclk) {

  if (tclk && !tap.tclk) {
    tdo = 0;
    
    switch (tap.state) {
    case TAP_STATE_TEST_LOGIC_RESET:
      if (!tms) tap.state = TAP_STATE_RUN_TEST_IDLE;
      break;
    case TAP_STATE_RUN_TEST_IDLE:
      if (tms) tap.state = TAP_STATE_SELECT_DR_SCAN;
      break;
    case TAP_STATE_SELECT_DR_SCAN:
      if (tms) tap.state = TAP_STATE_SELECT_IR_SCAN;
      else tap.state = TAP_STATE_CAPTURE_DR;
      break;
    case TAP_STATE_CAPTURE_DR:
      capture_dr();
      if (tms) tap.state = TAP_STATE_EXIT1_DR;
      else tap.state = TAP_STATE_SHIFT_DR;
      break;
    case TAP_STATE_SHIFT_DR:
      shift_dr();
      if (tms) tap.state = TAP_STATE_EXIT1_DR;
      break;
    case TAP_STATE_EXIT1_DR:
      if (tms) tap.state = TAP_STATE_UPDATE_DR;
      else tap.state = TAP_STATE_PAUSE_DR;
      break;
    case TAP_STATE_PAUSE_DR:
      if (tms) tap.state = TAP_STATE_EXIT2_DR;
      break;
    case TAP_STATE_EXIT2_DR:
      if (tms) tap.state = TAP_STATE_UPDATE_DR;
      else tap.state = TAP_STATE_SHIFT_DR;
      break;
    case TAP_STATE_UPDATE_DR:
      update_dr();
      if (tms) tap.state = TAP_STATE_SELECT_DR_SCAN;
      else tap.state = TAP_STATE_RUN_TEST_IDLE;
      break;
    case TAP_STATE_SELECT_IR_SCAN:
      if (tms) tap.state = TAP_STATE_TEST_LOGIC_RESET;
      else tap.state = TAP_STATE_CAPTURE_IR;
      break;
    case TAP_STATE_CAPTURE_IR:
      if (tms) tap.state = TAP_STATE_EXIT1_IR;
      else tap.state = TAP_STATE_SHIFT_IR;
      break;
    case TAP_STATE_SHIFT_IR:
      tdo = tap.capture_instr & 1;
      tap.capture_instr >>= 1;
      tap.capture_instr |= tdi << (INSTR_LENGTH - 1);
      if (tms) tap.state = TAP_STATE_EXIT1_IR;
      break;
    case TAP_STATE_EXIT1_IR:
      if (tms) tap.state = TAP_STATE_UPDATE_IR;
      else tap.state = TAP_STATE_PAUSE_IR;
      break;
    case TAP_STATE_PAUSE_IR:
      if (tms) tap.state = TAP_STATE_EXIT2_IR;
      break;
    case TAP_STATE_EXIT2_IR:
      if (tms) tap.state = TAP_STATE_UPDATE_IR;
      else tap.state = TAP_STATE_SHIFT_IR;
      break;
    case TAP_STATE_UPDATE_IR:
      tap.instr = tap.capture_instr;
      trace.msg("Updating instruction (newInstr: %s)\n", get_instr_name(tap.instr).c_str());
      if (tms) tap.state = TAP_STATE_SELECT_DR_SCAN;
      else tap.state = TAP_STATE_RUN_TEST_IDLE;
      break;
    }
    
    trace.msg("Changing TAP state (newState: %s)\n", tapStateName[tap.state].c_str());
 
  } else if (!tclk && tap.tclk) {
    if (tap.state == TAP_STATE_SHIFT_DR) {
      if (tap.instr == IDCODE_INSTR) {
        tdo = tap.id_reg & 1;
      } else if (!dev.do_burst) {
        tdo = tap.ctrl_reg & 1;
        tap.ctrl_reg >>= 1;
      }
    }
  }
    

  tap.tclk = tclk;
}

void adv_dbg_unit::tck_edge(int tck, int tdi, int tms, int trst)
{
  trace.msg("Executing cycle (TMS_BIT: %1d, TDI_BIT: %1d, TRST_BIT: %1d, TCLK_BIT: %d)\n", tms, tdi, trst, tck);
  this->tdi = tdi;
  tap_update(tms, tck);
}

void adv_dbg_unit::sync(void *__this, int tck, int tdi, int tms, int trst)
{
  adv_dbg_unit *_this = (adv_dbg_unit *)__this;
  _this->jtag_itf.sync(_this->tdo);
  _this->tck_edge(tck, tdi, tms, trst);
}


void adv_dbg_unit::sync_cycle(void *__this, int tdi, int tms, int trst)
{
  adv_dbg_unit *_this = (adv_dbg_unit *)__this;
  _this->jtag_itf.sync(_this->tdo);
  _this->tck_edge(1, tdi, tms, trst);
  _this->tck_edge(0, tdi, tms, trst);
}

void adv_dbg_unit::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);

  jtag_itf.set_sync_meth(&adv_dbg_unit::sync);
  jtag_itf.set_sync_cycle_meth(&adv_dbg_unit::sync_cycle);
  new_slave_port("jtag", &jtag_itf);

  new_master_port("io", &io_itf);

  tap_init();
}

void adv_dbg_unit::start()
{
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new adv_dbg_unit(config);
}



