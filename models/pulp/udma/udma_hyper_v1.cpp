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


#define ARCHI_UDMA_HAS_HYPER 1

#include "udma_v2_impl.hpp"
#include "archi/udma/udma_periph_v2.h"
#include "archi/utils.h"
#include "vp/itf/hyper.hpp"


Hyper_periph_v1::Hyper_periph_v1(udma *top, int id, int itf_id) : Udma_periph(top, id)
{
  std::string itf_name = "hyper" + std::to_string(itf_id);

  top->traces.new_trace(itf_name, &trace, vp::DEBUG);

  channel0 = new Hyper_rx_channel(top, this, UDMA_CHANNEL_ID(id), itf_name + "_rx");
  channel1 = new Hyper_tx_channel(top, this, UDMA_CHANNEL_ID(id) + 1, itf_name + "_tx");

  top->new_master_port(this, itf_name, &hyper_itf);

  this->regs = new unsigned int[HYPER_NB_REGS];

  //hyper_itf.set_sync_cycle_meth(&Hyper_periph_v1::sync_cycle);
  //hyper_itf.set_cs_sync_meth(&Hyper_periph_v1::cs_sync);
}
 

void Hyper_periph_v1::reset()
{
  Udma_periph::reset();
  memset(this->regs, 0, sizeof(unsigned int)*HYPER_NB_REGS);
  this->clkdiv = 0;
}





vp::io_req_status_e Hyper_periph_v1::custom_req(vp::io_req *req, uint64_t offset)
{
  if (req->get_size() != 4)
    return vp::IO_REQ_INVALID;

  int reg_id = offset / 4;
  if (reg_id >= HYPER_NB_REGS) return vp::IO_REQ_INVALID;

  if (!req->get_is_write())
    *(uint32_t *)(req->get_data()) = this->regs[reg_id];
  else 
    this->regs[reg_id] = *(uint32_t *)(req->get_data());


  return vp::IO_REQ_OK;
}


#if 0
void Hyper_periph_v1::sync_cycle(void *__this, int data)
{
  Hyper_periph_v1 *_this = (Hyper_periph_v1 *)__this;
  (static_cast<Hyper_rx_channel *>(_this->channel0))->handle_rx_bit(data);
}
#endif




Hyper_tx_channel::Hyper_tx_channel(udma *top, Hyper_periph_v1 *periph, int id, string name)
: Udma_tx_channel(top, id, name), periph(periph)
{
  pending_word_event = top->event_new(this, Hyper_tx_channel::handle_pending_word);
}







void Hyper_tx_channel::reset()
{
  Udma_tx_channel::reset();
  this->pending_bytes = 0;
  this->next_bit_cycle = -1;
  this->state = HYPER_STATE_IDLE;
}

void Hyper_tx_channel::handle_pending_word(void *__this, vp::clock_event *event)
{
  Hyper_tx_channel *_this = (Hyper_tx_channel *)__this;
  uint8_t byte;
  int cs;
  int cs_value;
  bool send_byte = false;
  bool send_cs = false;
  bool end = false;

  if (_this->state == HYPER_STATE_IDLE)
  {
    if (_this->pending_bytes > 0)
    {
      _this->state = HYPER_STATE_CS;
      _this->ca_count = 6;
      _this->ca.low_addr = ARCHI_REG_FIELD_GET(_this->periph->regs[HYPER_EXT_ADDR_CHANNEL_OFFSET], 1, 4);
      _this->ca.high_addr = ARCHI_REG_FIELD_GET(_this->periph->regs[HYPER_EXT_ADDR_CHANNEL_OFFSET], 4, 29);
      _this->ca.burst_type = 0;
      _this->ca.address_space = 0;
      _this->ca.read = ARCHI_REG_FIELD_GET(_this->periph->regs[HYPER_MEM_CFG3_CHANNEL_OFFSET], HYPER_MEM_CFG3_CRT0_OFFSET, 1);
      _this->transfer_size = _this->current_cmd->size;
    }
  }
  else if (_this->state == HYPER_STATE_CS)
  {
    _this->state = HYPER_STATE_CA;
    send_cs = true;
    cs = 0;
    cs_value = 1;
  }
  else if (_this->state == HYPER_STATE_CA)
  {
    send_byte = true;
    _this->ca_count--;
    byte = _this->ca.raw[_this->ca_count];
    if (_this->ca_count == 0)
    {
      _this->state = HYPER_STATE_DATA;
    }
  }
  else if (_this->state == HYPER_STATE_DATA && _this->pending_bytes > 0)
  {
    send_byte = true;
    byte = _this->pending_word & 0xff;
    _this->pending_word >>= 8;
    _this->pending_bytes--;
    if (_this->pending_bytes == 0)
    {
      end = true;
    }
    _this->transfer_size--;
    if (_this->transfer_size == 0)
    {
      _this->state = HYPER_STATE_CS_OFF;
    }
  }
  else if (_this->state == HYPER_STATE_CS_OFF)
  {
    _this->state = HYPER_STATE_IDLE;
    send_cs = true;
    cs = 0;
    cs_value = 0;
  }

  if (send_byte || send_cs)
  {
    if (!_this->periph->hyper_itf.is_bound())
    {
      _this->top->get_trace()->warning("Trying to send to HYPER interface while it is not connected\n");
    }
    else
    {
      _this->next_bit_cycle = _this->top->get_clock()->get_cycles() + _this->periph->clkdiv;
      if (send_byte)
      {
        _this->top->get_trace()->msg("Sending byte (value: 0x%x)\n", byte);
        _this->periph->hyper_itf.sync_cycle(byte);
      }
      else
      {
        _this->top->get_trace()->msg("Updating CS (cs: %d, value: %d)\n", cs, cs_value);
        _this->periph->hyper_itf.cs_sync(cs, cs_value);
      }
    }
  }

  if (end)
  {
    _this->handle_ready_req_end(_this->pending_req);
    _this->handle_ready_reqs();
  }

  _this->check_state();
}

void Hyper_tx_channel::check_state()
{
  if (this->pending_bytes != 0 && !pending_word_event->is_enqueued())
  {
    int latency = 1;
    int64_t cycles = this->top->get_clock()->get_cycles();
    if (next_bit_cycle > cycles)
      latency = next_bit_cycle - cycles;

    top->event_enqueue(pending_word_event, latency);
  }
}

void Hyper_tx_channel::handle_ready_reqs()
{
  if (this->pending_bytes == 0 && !ready_reqs->is_empty())
  {
    vp::io_req *req = this->ready_reqs->pop();
    this->pending_req = req;
    this->pending_word = *(uint32_t *)req->get_data();
    this->pending_bytes = req->get_size();
    this->check_state();
  }
}


Hyper_rx_channel::Hyper_rx_channel(udma *top, Hyper_periph_v1 *periph, int id, string name) : Udma_rx_channel(top, id, name), periph(periph)
{
}

void Hyper_rx_channel::reset()
{
  Udma_rx_channel::reset();
}
