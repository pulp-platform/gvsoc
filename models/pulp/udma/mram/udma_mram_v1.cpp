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


#include "udma_mram_v1.hpp"
#include "archi/udma/mram/udma_mram_v1.h"
#include "archi/utils.h"

#define MRAM_NB_REGS 24

Mram_periph_v1::Mram_periph_v1(udma *top, int id, int itf_id) : Io_Periph(top, id, "mram" + std::to_string(itf_id))
{
  std::string itf_name = "mram" + std::to_string(itf_id);

  top->traces.new_trace(itf_name, &trace, vp::DEBUG);

  channel0 = new Mram_v1_rx_channel(top, this, UDMA_EVENT_ID(id), itf_name + "_rx");
  channel1 = new Mram_v1_tx_channel(top, this, UDMA_EVENT_ID(id) + 1, itf_name + "_tx");

  this->regs = new unsigned int[MRAM_NB_REGS];
}
 

void Mram_periph_v1::reset(bool active)
{
  Udma_periph::reset(active);

  if (active)
  {
    memset(this->regs, 0, sizeof(unsigned int)*MRAM_NB_REGS);
  }
}


void Mram_periph_v1::check_state()
{
}




vp::io_req_status_e Mram_periph_v1::custom_req(vp::io_req *req, uint64_t offset)
{
  if (req->get_size() != 4)
    return vp::IO_REQ_INVALID;

  int reg_id = offset / 4;
  if (reg_id >= MRAM_NB_REGS) return vp::IO_REQ_INVALID;

  if (!req->get_is_write())
    *(uint32_t *)(req->get_data()) = this->regs[reg_id];
  else 
    this->regs[reg_id] = *(uint32_t *)(req->get_data());


  return vp::IO_REQ_OK;
}
