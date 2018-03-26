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


class Spim_rx_channel : public Udma_rx_channel
{
public:
  Spim_rx_channel(udma *top, int id, string name) : Udma_rx_channel(top, id, name) {}
};



class Spim_tx_channel : public Udma_tx_channel
{
public:
  Spim_tx_channel(udma *top, int id, string name) : Udma_tx_channel(top, id, name) {}
  void handle_ready_req(vp::io_req *req);

private:
  void reset();

  bool has_pending_word;
  uint32_t pending_word;
};



Spim_periph_v2::Spim_periph_v2(udma *top, int id, int itf_id) : Udma_periph(top, id)
{
  channel0 = new Spim_rx_channel(top, UDMA_CHANNEL_ID(id), "spim" + std::to_string(itf_id) + "_rx");
  channel1 = new Spim_tx_channel(top, UDMA_CHANNEL_ID(id) + 1, "spim" + std::to_string(itf_id) + "_tx");
}
  
void Spim_tx_channel::handle_ready_req(vp::io_req *req)
{
  uint32_t data = *(uint32_t *)req->get_data();

  printf("%x\n", data);

  uint32_t command = data >> SPI_CMD_ID_OFFSET;

  switch (command)
  {
    case SPI_CMD_CFG_ID: {
      unsigned int clock_div = (data >> SPI_CMD_CFG_CLK_DIV_OFFSET) & ((1<<SPI_CMD_CFG_CLK_DIV_WIDTH)-1);
      unsigned int cpol = (data >> SPI_CMD_CFG_CPOL_OFFSET) & 1;
      unsigned int cpha = (data >> SPI_CMD_CFG_CPHA_OFFSET) & 1;
      trace.msg("Received command CFG (clock_div: %d, cpol: %d, cpha: %d)\n", clock_div, cpol, cpha);
      //period = top->getEngine()->getPeriod() * clockDiv * 2;
      break;
    }

    case SPI_CMD_SOT_ID: {
      int cs = (data >> SPI_CMD_SOT_CS_OFFSET) & ((1<<SPI_CMD_SOT_CS_WIDTH)-1);
      trace.msg("Received command SOT (cs: %d)\n", cs);
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
        unsigned int bits = ((data >> SPI_CMD_TX_DATA_SIZE_OFFSET) & ((1<<SPI_CMD_TX_DATA_SIZE_WIDTH)-1))+1;
        unsigned int byte_align = (data >> SPI_CMD_TX_DATA_BYTE_ALIGN_OFFSET) & 1;
        trace.msg("Received command TX_DATA (size: %d, byte_align: %d)\n", bits, byte_align);
        //dataSize = bits;
        //waitWord = 1;
      //}
      break;
    }

    case SPI_CMD_RX_DATA_ID: {
      unsigned int bits = ((data >> SPI_CMD_RX_DATA_SIZE_OFFSET) & ((1<<SPI_CMD_RX_DATA_SIZE_WIDTH)-1))+1;
      unsigned int byte_align = (data >> SPI_CMD_TX_DATA_BYTE_ALIGN_OFFSET) & 1;
      trace.msg("Received command RX_DATA (size: %d, byte_align: %d)\n", bits, byte_align);
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
}

void Spim_tx_channel::reset()
{
  Udma_tx_channel::reset();
  has_pending_word = false;
}
