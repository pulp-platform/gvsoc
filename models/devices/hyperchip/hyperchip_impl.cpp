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
#include <stdio.h>
#include <string.h>
#include "vp/itf/hyper.hpp"
#include "vp/itf/wire.hpp"

#define REGS_AREA_SIZE 1024

#define FLASH_STATE_IDLE 0
#define FLASH_STATE_WRITE_BUFFER_WAIT_SIZE 1
#define FLASH_STATE_WRITE_BUFFER 2
#define FLASH_STATE_WRITE_BUFFER_WAIT_CONFIRM 3
#define FLASH_STATE_CMD_0 4
#define FLASH_STATE_CMD_1 5
#define FLASH_STATE_LOAD_VCR 6

typedef enum
{
  HYPERCHIP_STATE_CA,
  HYPERCHIP_STATE_DATA
} hyperchip_state_e;

class hyperchip : public vp::component
{

public:

  hyperchip(const char *config);

  int build();

  static void sync_cycle(void *_this, int data);
  static void cs_sync(void *__this, bool value);

private:

  vp::trace     trace;
  vp::hyper_slave   in_itf;
  vp::wire_slave<bool> cs_itf;

  uint8_t *ram_data;
  uint8_t *flash_data;
  uint8_t *reg_data;

  int ram_size;
  int flash_size;

  union
  {
    struct {
      int low_addr:3;
      int reserved:13;
      int high_addr:29;
      int burst_type:1;
      int address_space:1;
      int read:1;
    };
    uint8_t raw[6];
  } ca;

  int ca_count;
  int current_address;

  hyperchip_state_e state;
};

hyperchip::hyperchip(const char *config)
: vp::component(config)
{

}

void hyperchip::sync_cycle(void *__this, int data)
{
  hyperchip *_this = (hyperchip *)__this;

  if (_this->state == HYPERCHIP_STATE_CA)
  {
    _this->trace.msg("Received command byte (value: 0x%x)\n", data);

    _this->ca_count--;
    _this->ca.raw[_this->ca_count] = data;
    if (_this->ca_count == 0)
    {
      _this->state = HYPERCHIP_STATE_DATA;
      _this->current_address = (_this->ca.low_addr | (_this->ca.high_addr << 3)) * 2;
      _this->trace.msg("Received command header (addr: 0x%x)\n", _this->current_address);
    }
  }
  else if (_this->state == HYPERCHIP_STATE_DATA)
  {
    _this->trace.msg("Received data byte (value: 0x%x)\n", data);
    if (_this->current_address >= _this->ram_size)
    {
      _this->warning.warning("Received out-of-bound request (addr: 0x%x, ram_size: 0x%x)\n", _this->current_address, _this->ram_size);
    }
    else
    {
      _this->ram_data[_this->current_address] = data;
      _this->current_address++;
    }

  }
}

void hyperchip::cs_sync(void *__this, bool value)
{
  hyperchip *_this = (hyperchip *)__this;
  _this->trace.msg("Received CS sync (value: %d)\n", value);

  _this->state = HYPERCHIP_STATE_CA;
  _this->ca_count = 6;
}

int hyperchip::build()
{

  traces.new_trace("trace", &trace, vp::DEBUG);

  in_itf.set_sync_cycle_meth(&hyperchip::sync_cycle);
  new_slave_port("input", &in_itf);

  cs_itf.set_sync_meth(&hyperchip::cs_sync);
  new_slave_port("cs", &cs_itf);

  this->ram_size = 0;
  this->flash_size = 0;

  js::config *conf = this->get_js_config();

  js::config *ram_conf = conf->get("ram");
  if (ram_conf)
  {
    this->ram_size = ram_conf->get("size")->get_int();
    this->ram_data = new uint8_t[this->ram_size];
    memset(this->ram_data, 0x57, this->ram_size);
  }

  js::config *flash_conf = conf->get("flash");
  if (flash_conf)
  {
    this->flash_size = flash_conf->get("size")->get_int();
    this->flash_data = new uint8_t[this->flash_size];
    memset(this->flash_data, 0x57, this->flash_size);
  }

  this->reg_data = new uint8_t[REGS_AREA_SIZE];
  memset(this->reg_data, 0x57, REGS_AREA_SIZE);
  ((uint16_t *)this->reg_data)[0] = 0x8F1F;

  return 0;
}



extern "C" void *vp_constructor(const char *config)
{
  return (void *)new hyperchip(config);
}
