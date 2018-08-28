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

#define ARCHI_UDMA_HAS_SPIM 1

#include "udma_v2_impl.hpp"
#include "archi/udma/udma_periph_v2.h"
#include "archi/utils.h"
#include "vp/itf/qspim.hpp"




Spim_periph_v2::Spim_periph_v2(udma *top, int id, int itf_id) : Udma_periph(top, id)
{
  std::string itf_name = "spim" + std::to_string(itf_id);

  channel0 = new Spim_rx_channel(top, this, UDMA_CHANNEL_ID(id), itf_name + "_rx");
  channel1 = new Spim_tx_channel(top, this, UDMA_CHANNEL_ID(id) + 1, itf_name + "_tx");

  qspim_itf.set_sync_meth(&Spim_periph_v2::slave_sync);
  top->new_master_port(this, itf_name, &qspim_itf);
}

void Spim_periph_v2::reset()
{
  Udma_periph::reset();
  this->waiting_rx = false;
  this->cmd_pending_bits = 0;
  this->nb_received_bits = 0;
  this->pending_word = 0x57575757;
}

  
void Spim_periph_v2::slave_sync(void *__this, int data_0, int data_1, int data_2, int data_3, int mask)
{
  Spim_periph_v2 *_this = (Spim_periph_v2 *)__this;
  (static_cast<Spim_rx_channel *>(_this->channel0))->handle_rx_bits(data_0, data_1, data_2, data_3, mask);
}

void Spim_rx_channel::handle_rx_bits(int data_0, int data_1, int data_2, int data_3, int mask)
{

  if (this->periph->qpi)
  {
    this->periph->pending_word = (data_3 << 3) | (data_2 << 2) | (data_1 << 1) | (data_0 << 0) | (this->periph->pending_word << 4);
  }
  else
  {
    this->periph->pending_word = data_1 | (this->periph->pending_word << 1);
  }
}

void Spim_tx_channel::check_state()
{
  if ((this->pending_bits != 0 && !pending_word_event->is_enqueued()) || this->periph->waiting_rx)
  {
    int latency = 1;
    int64_t cycles = this->top->get_clock()->get_cycles();
    if (next_bit_cycle > cycles)
      latency = next_bit_cycle - cycles;

    top->event_enqueue(pending_word_event, latency);
  }
}


Spim_tx_channel::Spim_tx_channel(udma *top, Spim_periph_v2 *periph, int id, string name)
: Udma_tx_channel(top, id, name), periph(periph)
{
  pending_word_event = top->event_new(this, Spim_tx_channel::handle_pending_word);
}


void Spim_tx_channel::handle_ready_reqs()
{
  if (!this->periph->waiting_rx && this->pending_bits == 0 && !ready_reqs->is_empty())
  {
    vp::io_req *req = this->ready_reqs->pop();
    this->pending_req = req;
    this->pending_word = *(uint32_t *)req->get_data();
    this->pending_bits = req->get_size() * 8;
    this->check_state();
  }
}

void Spim_tx_channel::handle_pending_word(void *__this, vp::clock_event *event)
{
  Spim_tx_channel *_this = (Spim_tx_channel *)__this;
  _this->handle_data(_this->pending_word);
  _this->check_state();
}

void Spim_tx_channel::handle_data(uint32_t data)
{
  if (this->periph->cmd_pending_bits <= 0)
  {
    this->command = data >> SPI_CMD_ID_OFFSET;
  }

  bool handled_all = true;

  switch (this->command)
  {
    case SPI_CMD_CFG_ID: {
      this->periph->clkdiv = (data >> SPI_CMD_CFG_CLK_DIV_OFFSET) & ((1<<SPI_CMD_CFG_CLK_DIV_WIDTH)-1);
      unsigned int cpol = (data >> SPI_CMD_CFG_CPOL_OFFSET) & 1;
      unsigned int cpha = (data >> SPI_CMD_CFG_CPHA_OFFSET) & 1;
      trace.msg("Received command CFG (clock_div: %d, cpol: %d, cpha: %d)\n", this->periph->clkdiv, cpol, cpha);
      //period = top->getEngine()->getPeriod() * clockDiv * 2;
      break;
    }

    case SPI_CMD_SOT_ID: {
      int cs = (data >> SPI_CMD_SOT_CS_OFFSET) & ((1<<SPI_CMD_SOT_CS_WIDTH)-1);
      trace.msg("Received command SOT (cs: %d)\n", cs);
      if (!periph->qspim_itf.is_bound())
        trace.warning("Trying to set chip select to unbound QSPIM interface\n");
      else
        periph->qspim_itf.cs_sync(cs, 1);
      break;
    }

    case SPI_CMD_SEND_CMD_ID: {
      unsigned short cmd = (data >> SPI_CMD_SEND_CMD_CMD_OFFSET) & ((1<<SPI_CMD_SEND_CMD_CMD_WIDTH)-1);
      unsigned int size = ((data>>SPI_CMD_SEND_CMD_SIZE_OFFSET) & ((1<<SPI_CMD_SEND_CMD_SIZE_WIDTH)-1)) + 1;
      int qpi = (data>>SPI_CMD_SEND_CMD_QPI_OFFSET) & 0x1;
      trace.msg("Received command SEND_CMD (cmd: 0x%x, size: %d, qpi: %d)\n", cmd, size, qpi);
      break;
    }

    case SPI_CMD_SEND_ADDR_ID: {
      #if 0
      if (waitWord) {
        int64_t latency = 0;
        trace.msg("Received address: 0x%x\n", pendingWord);
        if (checkCs(cmdCs)) break;

        pendingWord = swapData(pendingWord, 4);

        spiPorts[cmdCs]->bsData((uint8_t *)&pendingWord, addrSize, &latency);
        waitWord = 0;
      } else {
        #endif
        uint32_t addr_size = ((data >> SPI_CMD_SEND_ADDR_SIZE_OFFSET) & ((1<<SPI_CMD_SEND_ADDR_SIZE_WIDTH)-1)) + 1;
        int qpi = (data>>SPI_CMD_SEND_ADDR_QPI_OFFSET) & 0x1;
        trace.msg("Received command SEND_ADDR (size: %d, qpi: %d)\n", addr_size, qpi);
        //waitWord = 1;
      //}
      break;
    }

    case SPI_CMD_DUMMY_ID: {
      int cycles = (data >> SPI_CMD_DUMMY_CYCLE_OFFSET) & ((1<<SPI_CMD_DUMMY_CYCLE_WIDTH)-1);
      trace.msg("Received command DUMMY (cycles: %d)\n", cycles);
      break;
    }

    case SPI_CMD_WAIT_ID: {
      trace.msg("Received command WAIT\n");
      //waitingEvent = (data >> SPI_CMD_WAIT_EVENT_OFFSET) & ((1<<SPI_CMD_WAIT_EVENT_WIDTH)-1);
      //isReadyToExecCommand = false;
      break;
    }

    case SPI_CMD_TX_DATA_ID: {
      if (this->periph->cmd_pending_bits <= 0)
      {
        unsigned int bits = ((data >> SPI_CMD_TX_DATA_SIZE_OFFSET) & ((1<<SPI_CMD_TX_DATA_SIZE_WIDTH)-1))+1;
        this->periph->byte_align = (data >> SPI_CMD_TX_DATA_BYTE_ALIGN_OFFSET) & 1;
        this->periph->qpi = ARCHI_REG_FIELD_GET(data, SPI_CMD_TX_DATA_QPI_OFFSET, 1);
        trace.msg("Received command TX_DATA (size: %d, byte_align: %d, qpi: %d)\n", bits, this->periph->byte_align, this->periph->qpi);
        this->periph->cmd_pending_bits = bits;
      }
      else
      {
        if (!this->periph->byte_align && this->pending_bits == 32) this->pending_word = __bswap_32(this->pending_word);

        int nb_bits = periph->qpi ? 4 : 1;
        this->next_bit_cycle = this->top->get_clock()->get_cycles() + this->periph->clkdiv;
        unsigned int bits = ARCHI_REG_FIELD_GET(this->pending_word, 32 - nb_bits, nb_bits);
        this->pending_word <<= nb_bits;
        this->top->get_trace()->msg("Sending bits (nb_bits: %d, value: 0x%x)\n", nb_bits, bits);
        if (!this->periph->qspim_itf.is_bound())
        {
          this->top->get_trace()->warning("Trying to send to SPIM interface while it is not connected\n");
        }
        else
        {
          this->periph->qspim_itf.sync_cycle(
            (bits >> 0) & 1, (bits >> 1) & 1, (bits >> 2) & 1, (bits >> 3) & 1, (1<<nb_bits)-1
          );
        }
        this->periph->cmd_pending_bits -= nb_bits;
        this->pending_bits -= nb_bits;
        handled_all = false;
      }
      #if 0
      if (waitWord) {
        int64_t latency = 0;
        unsigned int wordSize = 32;
        if (wordSize > dataSize) wordSize = dataSize;
        pendingWord >>= 32 - wordSize;
        trace.msg("Received data: 0x%x\n", pendingWord);
        if (checkCs(cmdCs)) break;
      // TODO this should be controlled by the datasize
        if (byte_align) pendingWord = swapData(pendingWord, 4);
        spiPorts[cmdCs]->bsData((uint8_t *)&pendingWord, wordSize, &latency);
        dataSize -= wordSize;
        if (dataSize == 0) waitWord = 0;
      } else {
        #endif
        //waitWord = 1;
      //}
      break;
    }

    case SPI_CMD_RX_DATA_ID: {
      if (this->periph->cmd_pending_bits <= 0)
      {
        unsigned int bits = ((data >> SPI_CMD_RX_DATA_SIZE_OFFSET) & ((1<<SPI_CMD_RX_DATA_SIZE_WIDTH)-1))+1;
        this->periph->byte_align = (data >> SPI_CMD_RX_DATA_BYTE_ALIGN_OFFSET) & 1;
        this->periph->qpi = ARCHI_REG_FIELD_GET(data, SPI_CMD_RX_DATA_QPI_OFFSET, 1);
        trace.msg("Received command RX_DATA (size: %d, byte_align: %d, qpi: %d)\n", bits, periph->byte_align, this->periph->qpi);
        this->periph->cmd_pending_bits = bits;
        this->periph->waiting_rx = true;
      }
      else
      {
        int nb_bits = periph->qpi ? 4 : 1;
        this->next_bit_cycle = this->top->get_clock()->get_cycles() + this->periph->clkdiv;

        if (!this->periph->qspim_itf.is_bound())
        {
          this->top->get_trace()->warning("Trying to receive from SPIM interface while it is not connected\n");
        }
        else
        {
          this->periph->qspim_itf.sync_cycle(0, 0, 0, 0, 0
          );
        }

        this->top->get_trace()->msg("Received bits (nb_bits: %d, value: 0x%x)\n", nb_bits, this->periph->pending_word & ((1<<nb_bits) - 1));

        this->periph->cmd_pending_bits -= nb_bits;
        this->periph->nb_received_bits += nb_bits;

        if (this->periph->nb_received_bits == 32 || this->periph->cmd_pending_bits <= 0)
        {
          if (!this->periph->byte_align) this->periph->pending_word = __bswap_32(this->periph->pending_word);

          (static_cast<Spim_rx_channel *>(this->periph->channel0))->push_data((uint8_t *)&this->periph->pending_word, 4);
          
          this->periph->nb_received_bits = 0;
          this->pending_word = 0x57575757;
        }


        if (this->periph->cmd_pending_bits <= 0)
        {
          this->periph->waiting_rx = false;
        }
        handled_all = false;
      }

      //int size = (bits+7)/8;
      //uint8_t data[(size+3) & 0xfffffffc];
      //int64_t latency = 0;
      //if (checkCs(cmdCs)) break;
      //spiPorts[cmdCs]->bsData((uint8_t *)data, bits, &latency);
      //// TODO this should be controlled by the datasize
      //if (byte_align)
      //{
      //  for (int i=0; i<size/4; i++) {
      //    ((int *)data)[i] = swapData(((int *)data)[i], 4);
      //  }
      //}
      //rxChannel->enqueueData(data, size, 0);
      //nextCommandCycles = top->getEngine()->getCycles() + latency;
      break;
    }
    case SPI_CMD_RPT_ID: {
      unsigned short iter = (data >> SPI_CMD_RPT_NB_OFFSET) & ((1<<SPI_CMD_RPT_NB_WIDTH)-1);
      trace.msg("Received command RPT (nbIter: %d)\n", iter);
      //pendingRepeatCount = iter;
      //if (!usingCommandCache) {
      //  fillingCommandCache = true;
      //  commandCacheIndex = 0;
      //}
      break;
    }
    case SPI_CMD_EOT_ID: {
      trace.msg("Received command EOT\n");
      //cmdCs = -1;
      //int evt = (data >> SPI_CMD_EOT_GEN_EVT_OFFSET) & 1;
      //if (evt) top->raiseEvent(eotEvent);
      break;
    }
    case SPI_CMD_RPT_END_ID: {
      trace.msg("Received command RPT_END\n");
      //pendingRepeatCount--;
      //if (pendingRepeatCount >= 0) {
      //  trace.msg("Repeating sequence (newCount: %d)\n", pendingRepeatCount);
      //  fillingCommandCache = false;
      //  usingCommandCache = true;
      //  isReadyToExecCommand = false;
      //  commandCacheIndex = 0;
      //} else {
      //  trace.msg("End of repeat reached\n");
      //  isReadyToExecCommand = true;
      //  usingCommandCache = false;
      //}
      break;
    }
    case SPI_CMD_RX_CHECK_ID: {
      trace.msg("Received command RX_CHECK\n");
      //int mode = (data >> SPI_CMD_RX_CHECK_MODE_OFFSET) & ((1<<SPI_CMD_RX_CHECK_MODE_WIDTH)-1);
      //uint16_t value = (data >> SPI_CMD_RX_CHECK_VALUE_OFFSET) & ((1<<SPI_CMD_RX_CHECK_VALUE_WIDTH)-1);
      //uint16_t bits = ((data >> SPI_CMD_RX_CHECK_SIZE_OFFSET) & ((1<<SPI_CMD_RX_CHECK_SIZE_WIDTH)-1))+1;
      //int size = (bits+7)/8;
      //uint16_t data = 0;
      //int64_t latency = 0;
      //if (checkCs(cmdCs)) break;
      //spiPorts[cmdCs]->bsData((uint8_t *)&data, bits, &latency);
      ////data = swapData(data, 4);
      //rxChannel->enqueueData((uint8_t *)&data, size, 0);
      //nextCommandCycles = top->getEngine()->getCycles() + latency;
      //switch (mode) {
      //  case SPI_CMD_RX_CHECK_MODE_MATCH:
      //  reachedCheck = value == data;
      //  trace.msg("Checking exact match (result: %d, readValue: 0x%x, expected: 0x%x)\n", reachedCheck, data, value);
      //  break;
      //  case SPI_CMD_RX_CHECK_MODE_ONES:
      //  reachedCheck = (data & value) == value;
      //  trace.msg("Checking ones (result: %d, readValue: 0x%x, expected: 0x%x)\n", reachedCheck, data, value);
      //  break;
      //  case SPI_CMD_RX_CHECK_MODE_ZEROS:
      //  reachedCheck = (~data & ~value) == ~value;
      //  trace.msg("Checking zeros match (result: %d, readValue: 0x%x, expected: 0x%x)\n", reachedCheck, data, value);
      //  break;
      //}
      break;
    }

    default:
      trace.warning("Received unknown SPI command: %x\n", command);
  }

  #if 0
  if (has_pending_word)
  {
    ready_reqs->push(req);
  }
  else
  {
    pending_word = *(uint32_t *)req->get_data();
    has_pending_word = true;
    trace.msg("Store L2 data to shift register (value: 0x%x)\n", pending_word);
  }
  #endif

  if (handled_all)
    this->pending_bits = 0;

  if (this->pending_bits == 0 && this->pending_req)
  {
    this->handle_ready_req_end(this->pending_req);
    this->pending_req = NULL;
  }
  
  this->handle_ready_reqs();
}

void Spim_tx_channel::reset()
{
  Udma_tx_channel::reset();
  this->next_bit_cycle = -1;
  this->pending_bits = 0;
}

void Spim_rx_channel::reset()
{
  Udma_rx_channel::reset();
}
