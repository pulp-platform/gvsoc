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

#include "archi/fll/fll_v1.h"

#define REG_MASK(offset,size) (((1<<(size))-1) << (offset))
#define REG_GET(fullValue,offset,size) (((fullValue) & REG_MASK(offset, size)) >> offset)

class fll_v1 : public vp::component
{

public:

  fll_v1(const char *config);

  void build();
  void start();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

private:

  vp::trace     trace;
  vp::io_slave in;

  uint32_t conf1_reg;
  uint32_t conf2_reg;
  uint32_t int_reg;

  //int refClock;
  unsigned int mult_factor;
  unsigned int div_factor;

  vp::io_req_status_e status_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e conf1_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e conf2_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e integrator_req(int reg_offset, int size, bool is_write, uint8_t *_data);

  void setFreq(int frequency);
  //void propagate_frequency();
};

fll_v1::fll_v1(const char *config)
: vp::component(config)
{

}

//void Fll::propagate_frequency() {
//  if (div_factor != 0) {
//    int div = div_factor > 1 ? (1 << (div_factor - 1)) : 1;
//    int newFreq = refClock * mult_factor / div;
//    if (newFreq == 0) {
//      gv_trace_dumpWarning(&warning, "Trying to set frequency 0\n");
//      return;
//    }
//    gv_trace_dumpMsg(&trace, "Setting new frequency (frequency: %d Hz)\n", newFreq);
//    if (clockItf) clockItf->setFreq(newFreq);
//  }
//}

vp::io_req_status_e fll_v1::status_req(int reg_offset, int size, bool is_write, uint8_t *data) {
  if (is_write) return vp::io_req_status_e::IO_REQ_INVALID;

  memcpy(data, &(((uint8_t *)&mult_factor)[reg_offset]), size);

  return vp::io_req_status_e::IO_REQ_OK;
}

vp::io_req_status_e fll_v1::conf1_req(int reg_offset, int size, bool is_write, uint8_t *data) {
  if (!is_write) {
    memcpy(data, &(((uint8_t *)&conf1_reg)[reg_offset]), size);
  } else {
    memcpy(&(((uint8_t *)&conf1_reg)[reg_offset]), data, size);
    int mode = REG_GET(conf1_reg, HAL_FLL_CONF1_MODE_BIT, HAL_FLL_CONF1_MODE_WIDTH);
    int lock = REG_GET(conf1_reg, HAL_FLL_CONF1_LOCK_BIT, HAL_FLL_CONF1_LOCK_WIDTH);
    div_factor = REG_GET(conf1_reg, HAL_FLL_CONF1_DIV_BIT, HAL_FLL_CONF1_DIV_WIDTH);
    int dco = REG_GET(conf1_reg, HAL_FLL_CONF1_DCO_BIT, HAL_FLL_CONF1_DCO_WIDTH);
    mult_factor = REG_GET(conf1_reg, HAL_FLL_CONF1_FACTOR_BIT, HAL_FLL_CONF1_FACTOR_WIDTH);
    trace.msg("Setting configuration 1 register (raw: 0x%x, mode: %d, lock: %d, div: %d, dco: %d, factor: %d)\n", conf1_reg, mode, lock, div_factor, dco, mult_factor);
    //propagate_frequency();
  }
  return vp::io_req_status_e::IO_REQ_OK;
}

vp::io_req_status_e fll_v1::conf2_req(int reg_offset, int size, bool is_write, uint8_t *data) {
  if (!is_write) {
    memcpy(data, &(((uint8_t *)&conf2_reg)[reg_offset]), size);
  } else {
    memcpy(&(((uint8_t *)&conf2_reg)[reg_offset]), data, size);
    int dith = REG_GET(conf2_reg, HAL_FLL_CONF2_DITH_BIT, HAL_FLL_CONF2_DITH_WIDTH);
    int open = REG_GET(conf2_reg, HAL_FLL_CONF2_OPEN_BIT, HAL_FLL_CONF2_OPEN_WIDTH);
    int clkSel = REG_GET(conf2_reg, HAL_FLL_CONF2_CLKSEL_BIT, HAL_FLL_CONF2_CLKSEL_WIDTH);
    int lockTol = REG_GET(conf2_reg, HAL_FLL_CONF2_LOCK_TOL_BIT, HAL_FLL_CONF2_LOCK_TOL_WIDTH);
    int nbStable = REG_GET(conf2_reg, HAL_FLL_CONF2_NB_STABLE_BIT, HAL_FLL_CONF2_NB_STABLE_WIDTH);
    int nbUnstable = REG_GET(conf2_reg, HAL_FLL_CONF2_NB_UNSTABLE_BIT, HAL_FLL_CONF2_NB_UNSTABLE_WIDTH);
    int gain = REG_GET(conf2_reg, HAL_FLL_CONF2_GAIN_BIT, HAL_FLL_CONF2_GAIN_WIDTH);
    trace.msg("Setting configuration 2 register (raw: 0x%x, dith: %d, open: %d, clkSel: %d, lockTol: %d, nbStable: %d, nbUnstable: %d, gain: %d)\n",
      conf2_reg, dith, open, clkSel, lockTol, nbStable, nbUnstable, gain);
  }
  return vp::io_req_status_e::IO_REQ_OK;
}

vp::io_req_status_e fll_v1::integrator_req(int reg_offset, int size, bool is_write, uint8_t *data) {
  if (!is_write) {
    memcpy(data, &(((uint8_t *)&int_reg)[reg_offset]), size);
  } else {
    memcpy(&(((uint8_t *)&int_reg)[reg_offset]), data, size);
    int intStateInt = REG_GET(int_reg, HAL_FLL_INT_STATE_INT_BIT, HAL_FLL_INT_STATE_INT_WIDTH);
    int intStateFract = REG_GET(int_reg, HAL_FLL_INT_STATE_FRACT_BIT, HAL_FLL_INT_STATE_FRACT_WIDTH);
    trace.msg("Setting integrator register (raw: 0x%x, dith: %d, open: %d, clkSel: %d, lockTol: %d, nbStable: %d, nbUnstable: %d, gain: %d)\n",
      int_reg, intStateInt, intStateFract);
  }
  return vp::io_req_status_e::IO_REQ_OK;
}

vp::io_req_status_e fll_v1::req(void *__this, vp::io_req *req)
{
  fll_v1 *_this = (fll_v1 *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("Fll_v1 access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  int reg_id = offset / 4;
  int reg_offset = offset % 4;

  if (reg_offset + size > 4) {
    _this->trace.warning("Accessing 2 registers in one access\n");
    return vp::io_req_status_e::IO_REQ_INVALID;
  }

  switch (reg_id) {
    case HAL_FLL_STATUS_OFFSET/4: return _this->status_req(reg_offset, size, is_write, data);
    case HAL_FLL_CONF1_OFFSET/4: return _this->conf1_req(reg_offset, size, is_write, data);
    case HAL_FLL_CONF2_OFFSET/4: return _this->conf2_req(reg_offset, size, is_write, data);
    case HAL_FLL_INTEGRATOR_OFFSET/4: return _this->integrator_req(reg_offset, size, is_write, data);
  }

  return vp::IO_REQ_OK;
}

void fll_v1::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);
  in.set_req_meth(&fll_v1::req);
  new_slave_port("in", &in);

  mult_factor = 0x5f5;
  div_factor = 1;

  conf1_reg = (mult_factor << HAL_FLL_CONF1_FACTOR_BIT) | 
    (div_factor << HAL_FLL_CONF1_DIV_BIT);

  conf2_reg = 0;

}

void fll_v1::start()
{
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new fll_v1(config);
}
