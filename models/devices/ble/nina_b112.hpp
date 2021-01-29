/*
 * Copyright (C) 2020 GreenWaves Technologies, SAS, ETH Zurich and
 *                    University of Bologna
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

#pragma once

#include <vp/vp.hpp>
#include <vp/itf/clock.hpp>
#include <vp/itf/uart.hpp>
#include <string>
#include <queue>

typedef enum
{
  UART_TX_STATE_IDLE,
  UART_TX_STATE_START,
  UART_TX_STATE_DATA,
  UART_TX_STATE_PARITY,
  UART_TX_STATE_STOP
} uart_tx_state_e;


typedef enum
{
  UART_RX_STATE_WAIT_START,
  UART_RX_STATE_GET_DATA,
  UART_RX_STATE_WAIT_STOP
} uart_rx_state_e;

typedef enum
{
     NINA_B112_OPERATING_MODE_COMMAND,
     NINA_B112_OPERATING_MODE_DATA,
     //NINA_B112_OPERATING_MODE_EXTENDED_DATA,
} nina_b112_operating_mode_e;

class Nina_b112 : public vp::component
{
public:
    Nina_b112(js::config *config);

    int build();
    void start();

private:
    static void rx_clk_reg(vp::component *__this, vp::component *clock);
    static void tx_clk_reg(vp::component *__this, vp::component *clock);
    static void sync_full(void *__this, int data, int clk, int rtr);

    void set_cts(int cts);

    void rx_start_sampling();
    void rx_stop_sampling();
    static void rx_sampling_handler(void *__this, vp::clock_event *event);
    void rx_handle_sampling();
    void rx_handle_byte(uint8_t byte);

    static void tx_sampling_handler(void *__this, vp::clock_event *event);
    void tx_check_byte();
    void tx_send_buffer(std::string buffer);
    void tx_send_byte(uint8_t byte);
    void tx_send_bit();

    std::string parse_cmd(std::string input_cmd);


    static void init_handler(void *__this, vp::clock_event *event);
    static void cts_end_handler(void *__this, vp::clock_event *event);

    vp::trace trace;

    vp::clock_master rx_clock_cfg;
    vp::clk_slave    rx_clock_itf;

    vp::clock_master tx_clock_cfg;
    vp::clk_slave    tx_clock_itf;


    vp::clock_engine *rx_clock;
    vp::clock_engine *tx_clock;

    vp::uart_slave    uart_itf;

    vp::clock_event *init_event;
    vp::clock_event *cts_event;

    uart_rx_state_e   rx_state;
    int rx_baudrate;
    int rx_nb_bits;
    int rx_prev_data;
    bool rx_sampling;
    uint8_t rx_byte;
    int rx_rtr;
    vp::clock_event *rx_sampling_event;


    uart_tx_state_e   tx_state;
    vp::clock_event *tx_sampling_event;
    std::queue<uint8_t> tx_pending_bytes;
    uint8_t tx_current_pending_byte;
    int tx_pending_bits;
    int tx_baudrate;
    int tx_cts;
    int tx_bit;
    int tx_current_stop_bits;
    int tx_parity;
    int tx_parity_en;

    /* cts fields */
    bool cts_enabled;
    bool cts_trigger;
    int  cts_duration;
    int  cts_bit;

    /* behavior fields */
    nina_b112_operating_mode_e operating_mode;
};
