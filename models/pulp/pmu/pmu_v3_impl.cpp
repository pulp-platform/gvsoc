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
#include <vp/itf/wire.hpp>
#include <string.h>
#include "archi/maestro/maestro_v3.h"

class pmu : public vp::component
{

public:

  pmu(const char *config);

  int build();
  void start();
  void reset(bool active);

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

private:

  vp::io_req_status_e pctrl_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e prdata_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e dlc_sr_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e dlc_imr_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e dlc_ifr_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e dlc_ioifr_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e dlc_idifr_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e dlc_imcifr_req(int reg_offset, int size, bool is_write, uint8_t *data);

  vp::trace     trace;
  vp::io_slave in;

};

pmu::pmu(const char *config)
: vp::component(config)
{

}

vp::io_req_status_e pmu::pctrl_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  maestro_dlc_pctrl_t reg { .raw=*(uint32_t *)data};

  if (is_write)
  {
    if (reg.start)
    {
      unsigned int cs = reg.paddr >> 5;
      unsigned int addr = reg.paddr & 0x1f;
      if (reg.dir == 0)
        this->trace.msg("Generating PICL write access (cs: %d, addr: 0x%x, value: 0x%x)\n", cs, addr, reg.pwdata);
      else
        this->trace.msg("Generating PICL read access (cs: %d, addr: 0x%x)\n", cs, addr);
    }
  }
  return vp::IO_REQ_OK;
}

vp::io_req_status_e pmu::prdata_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  return vp::IO_REQ_OK;
}

vp::io_req_status_e pmu::dlc_sr_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  return vp::IO_REQ_OK;
}

vp::io_req_status_e pmu::dlc_imr_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  return vp::IO_REQ_OK;
}

vp::io_req_status_e pmu::dlc_ifr_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  return vp::IO_REQ_OK;
}

vp::io_req_status_e pmu::dlc_ioifr_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  return vp::IO_REQ_OK;
}

vp::io_req_status_e pmu::dlc_idifr_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  return vp::IO_REQ_OK;
}

vp::io_req_status_e pmu::dlc_imcifr_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  return vp::IO_REQ_OK;
}


vp::io_req_status_e pmu::req(void *__this, vp::io_req *req)
{
  pmu *_this = (pmu *)__this;

  vp::io_req_status_e err = vp::IO_REQ_INVALID;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("PMU access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  if (size != 4) return vp::IO_REQ_INVALID;

  int reg_id = offset / 4;
  int reg_offset = offset % 4;

  switch (reg_id) {
    case MAESTRO_DLC_PCTRL_OFFSET/4  : err = _this->pctrl_req(reg_offset, size, is_write, data); break;
    case MAESTRO_PRDATA_OFFSET/4     : err = _this->prdata_req(reg_offset, size, is_write, data); break;
    case MAESTRO_DLC_SR_OFFSET/4     : err = _this->dlc_sr_req(reg_offset, size, is_write, data); break;
    case MAESTRO_DLC_IMR_OFFSET/4    : err = _this->dlc_imr_req(reg_offset, size, is_write, data); break;
    case MAESTRO_DLC_IFR_OFFSET/4    : err = _this->dlc_ifr_req(reg_offset, size, is_write, data); break;
    case MAESTRO_DLC_IOIFR_OFFSET/4  : err = _this->dlc_ioifr_req(reg_offset, size, is_write, data); break;
    case MAESTRO_DLC_IDIFR_OFFSET/4  : err = _this->dlc_idifr_req(reg_offset, size, is_write, data); break;
    case MAESTRO_DLC_IMCIFR_OFFSET/4 : err = _this->dlc_imcifr_req(reg_offset, size, is_write, data); break;
  }

  if (err != vp::IO_REQ_OK)
    goto error; 


  return vp::IO_REQ_OK;

error:
  _this->warning.force_warning("RTC invalid access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  return vp::IO_REQ_INVALID;
}


int pmu::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);
  in.set_req_meth(&pmu::req);
  new_slave_port("input", &in);

  return 0;
}

void pmu::reset(bool active)
{
}

void pmu::start()
{
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new pmu(config);
}
