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

#ifndef __PULP_UDMA_UDMA_SPIM_V3_IMPL_HPP__
#define __PULP_UDMA_UDMA_SPIM_V3_IMPL_HPP__

#include <vp/vp.hpp>
#include <vp/itf/io.hpp>
#include <vp/itf/qspim.hpp>
#include <vp/itf/uart.hpp>
#include <vp/itf/cpi.hpp>
#include <vp/itf/wire.hpp>
#include <vp/itf/hyper.hpp>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "archi/udma/udma_v3.h"

/*
 * SPIM
 */

class Spim_periph_v3;


class Spim_v3_rx_channel : public Udma_rx_channel
{
public:
  Spim_v3_rx_channel(udma *top, Spim_periph_v3 *periph, int id, string name) : Udma_rx_channel(top, id, name), periph(periph) {}

  void reset();

  void handle_rx_bits(int data_0, int data_1, int data_2, int data_3, int mask);

private:
  Spim_periph_v3 *periph;
};


class Spim_v3_tx_channel : public Udma_tx_channel
{
public:
  Spim_v3_tx_channel(udma *top, Spim_periph_v3 *periph, int id, string name);
  void handle_ready_reqs();
  void check_state();

private:
  void reset();
  static void handle_pending_word(void *__this, vp::clock_event *event);
  static void handle_spi_pending_word(void *__this, vp::clock_event *event);
  void handle_eot();
  void handle_data(uint32_t data);

  vp::clock_event *pending_word_event;
  vp::clock_event *pending_spi_word_event;

  Spim_periph_v3 *periph;

  uint32_t spi_tx_pending_word;   // Word being flushed to spi pads
  int      spi_tx_pending_bits;   // Tell how many bits are ready to be sent to SPI pads
  bool spi_tx_quad;
  bool spi_tx_byte_align;

  bool     has_tx_pending_word;   // Tell if a TX pending word is present
  uint32_t tx_pending_word;       // Word received by last L2 req
  uint32_t rx_pending_word;
  int64_t next_bit_cycle;
  vp::io_req *pending_req;
  uint32_t command;
  int cs;
  bool gen_eot_with_evt;
};



class Spim_periph_v3 : public Udma_periph
{
  friend class Spim_v3_tx_channel;
  friend class Spim_v3_rx_channel;

public:
  Spim_periph_v3(udma *top, int id, int itf_id);
  static void slave_sync(void *_this, int data_0, int data_1, int data_2, int data_3, int mask);
  void reset();
  vp::io_req_status_e custom_req(vp::io_req *req, uint64_t offset);

protected:
  vp::qspim_master qspim_itf;
  int clkdiv;
  bool waiting_rx;
  bool waiting_tx;
  bool is_full_duplex;
  int byte_align;
  int qpi;
  int cmd_pending_bits;
  int nb_received_bits;
  uint32_t rx_pending_word;
  uint32_t tx_pending_word;
  int eot_event;
  
  int      spi_rx_pending_bits;   // Tell how many bits should be received from spi pads

};

#endif