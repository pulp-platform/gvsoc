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
#include <vp/itf/wire.hpp>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "archi/gpio/gpio_v2.h"

class gpio : public vp::component
{

public:

  gpio(const char *config);

  int build();
  void start();

private:

  static void gpio_sync(void *_this, bool value, int gpio);
  static vp::io_req_status_e req(void *__this, vp::io_req *req);

  vp::io_req_status_e paddir_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e padin_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e padout_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e inten_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e inttype0_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e inttype1_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e intstatus_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e gpioen_req(int reg_offset, int size, bool is_write, uint8_t *data);
  vp::io_req_status_e padcfg_req(int id, int reg_offset, int size, bool is_write, uint8_t *data);

  vp::io_slave in;

  std::vector<vp::wire_slave<bool> *> gpio_itf;

  int nb_gpio;

  vp_gpio_apbgpio_paddir               r_paddir;
  vp_gpio_apbgpio_padin                r_padin;
  vp_gpio_apbgpio_padout               r_padout;
  vp_gpio_apbgpio_inten                r_inten;
  vp_gpio_apbgpio_inttype0             r_inttype0;
  vp_gpio_apbgpio_inttype1             r_inttype1;
  vp_gpio_apbgpio_intstatus            r_intstatus;
  vp_gpio_apbgpio_gpioen               r_gpioen;
  std::vector<vp_gpio_apbgpio_padcfg0> r_padcfg;
};



gpio::gpio(const char *config)
: vp::component(config)
{

}





vp::io_req_status_e gpio::paddir_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  this->r_paddir.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}

vp::io_req_status_e gpio::padin_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  this->r_padin.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}

vp::io_req_status_e gpio::padout_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  // TODO GPIO output should be propagated to pads. This should take gpioen into account only on some architecture
  this->r_padout.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}

vp::io_req_status_e gpio::inten_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  this->r_inten.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}

vp::io_req_status_e gpio::inttype0_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  this->r_inttype0.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}

vp::io_req_status_e gpio::inttype1_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  this->r_inttype1.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}

vp::io_req_status_e gpio::intstatus_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  this->r_intstatus.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}

vp::io_req_status_e gpio::gpioen_req(int reg_offset, int size, bool is_write, uint8_t *data)
{
  this->r_gpioen.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}

vp::io_req_status_e gpio::padcfg_req(int id, int reg_offset, int size, bool is_write, uint8_t *data)
{
  this->r_padcfg[id].access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e gpio::req(void *__this, vp::io_req *req)
{
  gpio *_this = (gpio *)__this;

  vp::io_req_status_e err = vp::IO_REQ_INVALID;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  uint64_t is_write = req->get_is_write();

  _this->get_trace()->msg("GPIO access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, req->get_is_write());

  int reg_id = offset / 4;
  int reg_offset = offset % 4;

  if (reg_offset + size > 4) {
    _this->get_trace()->force_warning("Accessing 2 registers in one access\n");
    goto error;
  }

  switch (reg_id)
  {
    case GPIO_APBGPIO_PADDIR_OFFSET/4:
      err = _this->paddir_req(reg_offset, size, is_write, data);
      break;

    case GPIO_APBGPIO_PADIN_OFFSET/4:
      err = _this->padin_req(reg_offset, size, is_write, data);
      break;

    case GPIO_APBGPIO_PADOUT_OFFSET/4:
      err = _this->padout_req(reg_offset, size, is_write, data);
      break;

    case GPIO_APBGPIO_INTEN_OFFSET/4:
      err = _this->inten_req(reg_offset, size, is_write, data);
      break;

    case GPIO_APBGPIO_INTTYPE0_OFFSET/4:
      err = _this->inttype0_req(reg_offset, size, is_write, data);
      break;

    case GPIO_APBGPIO_INTTYPE1_OFFSET/4:
      err = _this->inttype1_req(reg_offset, size, is_write, data);
      break;

    case GPIO_APBGPIO_INTSTATUS_OFFSET/4:
      err = _this->intstatus_req(reg_offset, size, is_write, data);
      break;

    case GPIO_APBGPIO_GPIOEN_OFFSET/4:
      err = _this->gpioen_req(reg_offset, size, is_write, data);
      break;

  }

  if (reg_id >= GPIO_APBGPIO_PADCFG0_OFFSET/4 && reg_id <= GPIO_APBGPIO_PADCFG7_OFFSET/4)
    err = _this->padcfg_req(reg_id - GPIO_APBGPIO_PADCFG0_OFFSET/4, reg_offset, size, is_write, data);

  if (err != vp::IO_REQ_OK)
    goto error; 


  return vp::IO_REQ_OK;

error:
  _this->get_trace()->force_warning("GPIO invalid access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  return vp::IO_REQ_INVALID;
}



void gpio::gpio_sync(void *_this, bool value, int gpio)
{
  // TODO generate interrupt
#if 0
  gv_trace_dumpMsg(&trace, "Received new gpio value (gpio: %d, value: %d)\n", pad, value);
  unsigned int oldVal = (padin >> pad) & 1;
  padin = (padin & ~(1<<pad)) | (value << pad);
  if (((inten >> pad) & 1) && oldVal != value) {
    unsigned int inttype;
    if (PLP_GPIO_INTTYPE_NO(pad) == 0) inttype = PLP_GPIO_INTTYPE_GET(pad,inttype0);
    else inttype = PLP_GPIO_INTTYPE_GET(pad - PLP_GPIO_INTTYPE_GPIO(1),inttype1);

    if (((inttype & PLP_GPIO_INTTYPE_RISE) && oldVal == 0) || ((inttype & PLP_GPIO_INTTYPE_FALL) && oldVal == 1)) {
      intstatus |= 1 << pad;
      if (irqItf && event != -1) {
        gv_trace_dumpMsg(&trace, "Raising event (id: %d)\n", event);
        irqItf->sync(event);
      }
    }
  }
#endif
}



int gpio::build()
{
  this->in.set_req_meth(&gpio::req);
  this->new_slave_port("input", &this->in);

  this->nb_gpio = this->get_js_config()->get_int("nb_gpio");

  for (int i=0; i<this->nb_gpio; i++)
  {
    vp::wire_slave<bool> *itf = new vp::wire_slave<bool>();
    itf->set_sync_meth_muxed(&gpio::gpio_sync, i);
    new_slave_port("gpio" + std::to_string(i), itf);
    this->gpio_itf.push_back(itf);
  }

  this->new_reg("paddir", &this->r_paddir, 0);
  this->new_reg("padin", &this->r_padin, 0);
  this->new_reg("padout", &this->r_padout, 0);
  this->new_reg("inten", &this->r_inten, 0);
  this->new_reg("inttype0", &this->r_inttype0, 0);
  this->new_reg("inttype1", &this->r_inttype1, 0);
  this->new_reg("intstatus", &this->r_intstatus, 0);
  this->new_reg("gpioen", &this->r_gpioen, 0);

  this->r_padcfg.resize(this->nb_gpio/4);

  for (int i=0; i<this->nb_gpio/4; i++)
  {
    this->new_reg("padcfg" + std::to_string(i), &this->r_padcfg[i], 0);
  }

  return 0;
}



void gpio::start()
{
}



extern "C" void *vp_constructor(const char *config)
{
  return (void *)new gpio(config);
}
