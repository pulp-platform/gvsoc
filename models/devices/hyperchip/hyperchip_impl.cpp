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
#include "archi/utils.h"
#include "archi/udma/hyper.h"

#define REGS_AREA_SIZE 1024

#define FLASH_STATE_IDLE 0
#define FLASH_STATE_WRITE_BUFFER_WAIT_SIZE 1
#define FLASH_STATE_WRITE_BUFFER 2
#define FLASH_STATE_WRITE_BUFFER_WAIT_CONFIRM 3
#define FLASH_STATE_CMD_0 4
#define FLASH_STATE_CMD_1 5
#define FLASH_STATE_LOAD_VCR 6


class hyperchip;


class Hyperflash
{
public:
  Hyperflash(hyperchip *top, int size);

  void handle_access(int reg_access, int address, int read, uint8_t data);
  int preload_file(char *path);

private:
  hyperchip *top;
  int size;
  uint8_t *data;
  uint8_t *reg_data;
};



class Hyperram
{
public:
  Hyperram(hyperchip *top, int size);

  void handle_access(int reg_access, int address, int read, uint8_t data);

private:
  hyperchip *top;
  int size;
  uint8_t *data;
  uint8_t *reg_data;
};



typedef enum
{
  HYPERCHIP_STATE_CA,
  HYPERCHIP_STATE_DATA
} hyperchip_state_e;

class hyperchip : public vp::component
{
  friend class Hyperram;
  friend class Hyperflash;

public:

  hyperchip(const char *config);

  int build();

  static void sync_cycle(void *_this, int data);
  static void cs_sync(void *__this, bool value);

protected:
  vp::trace     trace;
  vp::hyper_slave   in_itf;
  vp::wire_slave<bool> cs_itf;

private:
  Hyperflash *flash;
  Hyperram *ram;

  union
  {
    struct {
      unsigned int low_addr:3;
      unsigned int reserved:13;
      unsigned int high_addr:29;
      unsigned int burst_type:1;
      unsigned int address_space:1;
      unsigned int read:1;
    } __attribute__((packed));;
    uint8_t raw[6];
  } ca;

  int ca_count;
  int current_address;
  int flash_access;
  int reg_access;

  hyperchip_state_e state;
};



Hyperram::Hyperram(hyperchip *top, int size) : top(top), size(size)
{
  this->data = new uint8_t[this->size];
  memset(this->data, 0x57, this->size);

  this->reg_data = new uint8_t[REGS_AREA_SIZE];
  memset(this->reg_data, 0x57, REGS_AREA_SIZE);
  ((uint16_t *)this->reg_data)[0] = 0x8F1F;
}


void Hyperram::handle_access(int reg_access, int address, int read, uint8_t data)
{
  if (address >= this->size)
  {
    this->top->warning.warning("Received out-of-bound request (addr: 0x%x, ram_size: 0x%x)\n", address, this->size);
  }
  else
  {
    if (read)
    {
      uint8_t data = this->data[address];
      this->top->trace.msg("Sending data byte (value: 0x%x)\n", data);
      this->top->in_itf.sync_cycle(data);

    }
    else
    {
      this->top->trace.msg("Received data byte (value: 0x%x)\n", data);
      this->data[address] = data;
    }
  }
}



Hyperflash::Hyperflash(hyperchip *top, int size) : top(top), size(size)
{
  this->data = new uint8_t[this->size];
  memset(this->data, 0x57, this->size);

  this->reg_data = new uint8_t[REGS_AREA_SIZE];
  memset(this->reg_data, 0x57, REGS_AREA_SIZE);
  ((uint16_t *)this->reg_data)[0] = 0x8F1F;
}



void Hyperflash::handle_access(int reg_access, int address, int read, uint8_t data)
{
  if (address >= this->size)
  {
    this->top->warning.warning("Received out-of-bound request (addr: 0x%x, ram_size: 0x%x)\n", address, this->size);
  }
  else
  {
    if (read)
    {
      uint8_t data = this->data[address];
      this->top->trace.msg("Sending data byte (value: 0x%x)\n", data);
      this->top->in_itf.sync_cycle(data);

    }
    else
    {
      this->top->trace.msg("Received data byte (value: 0x%x)\n", data);
      //this->data[address] = data;
    }
  }
}

int Hyperflash::preload_file(char *path)
{
  this->top->get_trace()->msg("Preloading memory with stimuli file (path: %s)\n", path);
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    printf("Unable to open stimulus file (path: %s, error: %s)\n", path, strerror(errno));
    return -1;
  }

  if (fread(this->data, 1, this->size, file) == 0)
    return -1;

  return 0;
}

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
      _this->current_address = (_this->ca.low_addr | (_this->ca.high_addr << 3)) & ~1;

      _this->flash_access = ARCHI_REG_FIELD_GET(_this->current_address, REG_MBR_WIDTH, 1);
      _this->current_address = ARCHI_REG_FIELD_GET(_this->current_address, 0, REG_MBR_WIDTH);
      _this->reg_access = _this->ca.address_space == 1;

      _this->trace.msg("Received command header (flash_access: %d, reg_access: %d, addr: 0x%x, read: %d)\n", _this->flash_access, _this->ca.address_space, _this->current_address, _this->ca.read);
    }
  }
  else if (_this->state == HYPERCHIP_STATE_DATA)
  {
    if (_this->flash_access)
    {
      _this->flash->handle_access(_this->reg_access, _this->current_address, _this->ca.read, data);
    }
    else
    {
      _this->ram->handle_access(_this->reg_access, _this->current_address, _this->ca.read, data);
    }
    _this->current_address++;
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

  int ram_size = 0;
  int flash_size = 0;

  js::config *conf = this->get_js_config();

  js::config *ram_conf = conf->get("ram");
  if (ram_conf)
    ram_size = ram_conf->get("size")->get_int();
  this->ram = new Hyperram(this, ram_size);

  js::config *flash_conf = conf->get("flash");
  if (flash_conf)
    flash_size = flash_conf->get("size")->get_int();
  this->flash = new Hyperflash(this, flash_size);

  if (flash_conf)
  {
    js::config *preload_file_conf = flash_conf->get("preload_file");
    if (preload_file_conf)
    {
      if (this->flash->preload_file((char *)preload_file_conf->get_str().c_str()))
        return -1;
    }
  }

  return 0;
}



extern "C" void *vp_constructor(const char *config)
{
  return (void *)new hyperchip(config);
}
