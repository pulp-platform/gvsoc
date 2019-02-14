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


#include "udma_impl.hpp"
#include "archi/utils.h"
#include "vp/itf/cpi.hpp"


I2s_periph_v1::I2s_periph_v1(udma *top, int id, int itf_id) : Udma_periph(top, id)
{
  std::string itf_name = "i2s" + std::to_string(itf_id);

  top->traces.new_trace(itf_name, &trace, vp::DEBUG);

  channel0 = new I2s_rx_channel(top, this, UDMA_EVENT_ID(id), itf_name + "_rx");
}
 

void I2s_periph_v1::reset(bool active)
{
  Udma_periph::reset(active);
}





vp::io_req_status_e I2s_periph_v1::custom_req(vp::io_req *req, uint64_t offset)
{
  bool is_write = req->get_is_write();
  uint32_t *data = (uint32_t *)req->get_data();

  if (req->get_size() != 4)
    return vp::IO_REQ_INVALID;

  return vp::IO_REQ_OK;
}



I2s_rx_channel::I2s_rx_channel(udma *top, I2s_periph_v1 *periph, int id, string name) : Udma_rx_channel(top, id, name), periph(periph)
{
}

void I2s_rx_channel::reset(bool active)
{
  Udma_rx_channel::reset(active);
}
