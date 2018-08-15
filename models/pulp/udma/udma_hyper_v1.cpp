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

  //hyper_itf.set_sync_cycle_meth(&Hyper_periph_v1::sync_cycle);
  //hyper_itf.set_cs_sync_meth(&Hyper_periph_v1::cs_sync);
}
 

void Hyper_periph_v1::reset()
{
  Udma_periph::reset();
}





vp::io_req_status_e Hyper_periph_v1::custom_req(vp::io_req *req, uint64_t offset)
{
  if (req->get_size() != 4)
    return vp::IO_REQ_INVALID;


  return vp::IO_REQ_INVALID;
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
}








void Hyper_tx_channel::reset()
{
  Udma_tx_channel::reset();
}




Hyper_rx_channel::Hyper_rx_channel(udma *top, Hyper_periph_v1 *periph, int id, string name) : Udma_rx_channel(top, id, name), periph(periph)
{
}

void Hyper_rx_channel::reset()
{
  Udma_rx_channel::reset();
}
