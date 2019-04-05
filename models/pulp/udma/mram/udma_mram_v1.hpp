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

#ifndef __PULP_UDMA_MRAM_UDMA_MRAM_V1_HPP__
#define __PULP_UDMA_MRAM_UDMA_MRAM_V1_HPP__

#include <vp/vp.hpp>
#include "../udma_impl.hpp"



class Mram_periph_v1 : public Io_Periph
{
public:
  Mram_periph_v1(udma *top, int id, int itf_id);
  vp::io_req_status_e custom_req(vp::io_req *req, uint64_t offset);
  void reset(bool active);
  static void handle_pending_word(void *__this, vp::clock_event *event);
  
protected:
  unsigned int *regs;

private:
  void check_state();
  vp::trace     trace;
};



class Mram_v1_rx_channel : public Io_rx_channel
{
public:
  Mram_v1_rx_channel(udma *top, Mram_periph_v1 *periph, int id, string name)
  : Io_rx_channel(top, periph, id, name), periph(periph) {}

private:
  Mram_periph_v1 *periph;
};

class Mram_v1_tx_channel : public Io_tx_channel
{
  friend class Mram_periph_v1;

public:
  Mram_v1_tx_channel(udma *top, Mram_periph_v1 *periph, int id, string name)
  : Io_tx_channel(top, periph, id, name), periph(periph) {}

private:
  Mram_periph_v1 *periph;
};

#endif
