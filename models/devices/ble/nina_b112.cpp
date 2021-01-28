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

/* 
 * Authors: Paul Luperini, GreenWaves Technologies (paul.luperini@greenwaves-technologies.com)
 */

#include <vp/vp.hpp>
#include <stdio.h>
#include <vp/itf/clock.hpp>
#include <vp/itf/uart.hpp>


typedef enum
{
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
    void tx_send_byte(uint8_t byte);
    void tx_send_bit();

    vp::trace trace;

    vp::clock_master rx_clock_cfg;
    vp::clk_slave    rx_clock_itf;

    vp::clock_master tx_clock_cfg;
    vp::clk_slave    tx_clock_itf;


    vp::clock_engine *rx_clock;
    vp::clock_engine *tx_clock;

    vp::uart_slave    uart_itf;

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
    uint8_t tx_pending_byte;
    int tx_pending_bits;
    int tx_baudrate;
    int tx_cts;
    int tx_bit;
    int tx_current_stop_bits;
    int tx_parity;
    int tx_parity_en;

};


Nina_b112::Nina_b112(js::config *config)
    : vp::component(config)
{
}


void Nina_b112::sync_full(void *__this, int data, int clk, int rtr)
{
    Nina_b112 *_this = (Nina_b112 *)__this;

    _this->rx_rtr = rtr;

    if (_this->rx_state == UART_RX_STATE_WAIT_START && _this->rx_prev_data == 1 && data == 0)
    {
        _this->trace.msg(vp::trace::LEVEL_TRACE, "Received start bit\n");

        _this->rx_start_sampling();
        _this->rx_state = UART_RX_STATE_GET_DATA;
        _this->rx_nb_bits = 0;
    }

    _this->rx_prev_data = data;
}


void Nina_b112::rx_start_sampling()
{
    this->trace.msg(vp::trace::LEVEL_TRACE, "Start RX sampling (baudrate: %d)\n", this->rx_baudrate);

    // We set the frequency to twice the baudrate to be able sampling in the
    // middle of the cycle
    this->rx_clock_cfg.set_frequency(this->rx_baudrate*2);

    this->rx_sampling = 1;

    this->rx_clock->reenqueue(this->rx_sampling_event, 3);
}


void Nina_b112::rx_stop_sampling()
{
    this->rx_sampling = 0;
    
    if (this->rx_sampling_event->is_enqueued())
    {
        this->rx_clock->cancel(this->rx_sampling_event);
    }
}


void Nina_b112::rx_handle_byte(uint8_t byte)
{
    // For now just do a loopback, should be replaced by real stuff
    this->tx_send_byte(byte);
}


void Nina_b112::rx_handle_sampling()
{
    this->trace.msg(vp::trace::LEVEL_TRACE, "Sampling bit (value: %d)\n", this->rx_prev_data);


    switch (this->rx_state)
    {
        case UART_RX_STATE_GET_DATA:
            this->trace.msg(vp::trace::LEVEL_TRACE, "Received data bit (data: %d)\n", this->rx_prev_data);
            this->rx_byte = (this->rx_byte >> 1) | (this->rx_prev_data << 7);
            this->rx_nb_bits++;
            if (this->rx_nb_bits == 8)
            {
                this->trace.msg(vp::trace::LEVEL_DEBUG, "Sampled RX byte (value: 0x%x)\n", this->rx_byte);
                this->trace.msg(vp::trace::LEVEL_TRACE, "Waiting for stop bit\n");
                this->rx_state = UART_RX_STATE_WAIT_STOP;
                this->rx_handle_byte(this->rx_byte);
            }
            break;

        case UART_RX_STATE_WAIT_STOP:
            if (this->rx_prev_data == 1)
            {
                this->trace.msg(vp::trace::LEVEL_TRACE, "Received stop bit\n", this->rx_prev_data);
                this->rx_state = UART_RX_STATE_WAIT_START;
                this->rx_stop_sampling();
            }
    }
}


void Nina_b112::rx_sampling_handler(void *__this, vp::clock_event *event)
{
    Nina_b112 *_this = (Nina_b112 *)__this;

    _this->rx_handle_sampling();

    if (_this->rx_sampling)
    {
        _this->rx_clock->enqueue(_this->rx_sampling_event, 2);
    }
}


void Nina_b112::set_cts(int cts)
{
    this->tx_cts = cts;
    this->uart_itf.sync_full(this->tx_bit, 2, this->tx_cts);
}


void Nina_b112::tx_sampling_handler(void *__this, vp::clock_event *event)
{
    Nina_b112 *_this = (Nina_b112 *)__this;
    _this->tx_send_bit();
}


void Nina_b112::tx_send_bit()
{
    int bit;

    switch (this->tx_state)
    {
        case UART_TX_STATE_START:
        {
            this->trace.msg(vp::trace::LEVEL_TRACE, "Sending start bit\n", this->rx_prev_data);
            this->tx_parity = 0;
            this->tx_state = UART_TX_STATE_DATA;
            this->tx_current_stop_bits = 1;
            bit = 0;
            break;
        }

        case UART_TX_STATE_DATA:
        {
            bit = this->tx_pending_byte & 1;
            this->trace.msg(vp::trace::LEVEL_TRACE, "Sending data bit (value: %d)\n", bit);
            this->tx_pending_byte >>= 1;
            this->tx_pending_bits -= 1;
            this->tx_parity ^= bit;

            if (this->tx_pending_bits == 0)
            {
                if (this->tx_parity_en)
                    this->tx_state = UART_TX_STATE_PARITY;
                else
                {
                    this->tx_state = UART_TX_STATE_STOP;
                }
            }
            break;
        }

        case UART_TX_STATE_PARITY:
        {
            bit = this->tx_parity;
            this->trace.msg(vp::trace::LEVEL_TRACE, "Sending parity bit (value: %d)\n", bit);
            this->tx_state = UART_TX_STATE_STOP;
            break;
        }

        case UART_TX_STATE_STOP:
        {
            this->trace.msg(vp::trace::LEVEL_TRACE, "Sending stop bit\n", this->rx_prev_data);
            bit = 1;
            this->tx_current_stop_bits--;
            if (this->tx_current_stop_bits == 0)
            {
                this->tx_state = UART_TX_STATE_START;
            }
            break;
        }
    }

    this->tx_bit = bit;
    this->uart_itf.sync_full(this->tx_bit, 2, this->tx_cts);

    if (this->tx_state != UART_TX_STATE_START)
    {
        this->tx_clock->reenqueue(this->tx_sampling_event, 2);
    }
}



void Nina_b112::tx_check_byte()
{
    if (this->tx_pending_bits && !this->tx_sampling_event->is_enqueued())
    {
        // TODO enable rtr
        //if (this->rx_rtr == 0)
        {
            this->tx_clock->reenqueue(this->tx_sampling_event, 2);
        }
    }
}


void Nina_b112::tx_send_byte(uint8_t byte)
{
    this->trace.msg(vp::trace::LEVEL_DEBUG, "Sending byte (value: 0x%x)\n", byte);
    this->tx_pending_byte = byte;
    this->tx_pending_bits = 8;
    this->tx_state = UART_TX_STATE_START;
    this->tx_clock_cfg.set_frequency(this->tx_baudrate*2);

    this->tx_check_byte();
}



void Nina_b112::rx_clk_reg(vp::component *__this, vp::component *clock)
{
    Nina_b112 *_this = (Nina_b112 *)__this;
    _this->rx_clock = (vp::clock_engine *)clock;
}


void Nina_b112::tx_clk_reg(vp::component *__this, vp::component *clock)
{
    Nina_b112 *_this = (Nina_b112 *)__this;
    _this->tx_clock = (vp::clock_engine *)clock;
}


int Nina_b112::build()
{
    traces.new_trace("trace", &trace, vp::DEBUG);

    this->rx_clock_itf.set_reg_meth(&Nina_b112::rx_clk_reg);
    this->new_slave_port(this, "rx_clock", &this->rx_clock_itf);

    this->tx_clock_itf.set_reg_meth(&Nina_b112::tx_clk_reg);
    this->new_slave_port(this, "tx_clock", &this->tx_clock_itf);

    this->new_master_port(this, "tx_clock_cfg", &this->tx_clock_cfg);

    this->new_master_port(this, "rx_clock_cfg", &this->rx_clock_cfg);

    this->uart_itf.set_sync_full_meth(&Nina_b112::sync_full);
    this->new_slave_port(this, "uart", &this->uart_itf);

    this->rx_sampling_event = this->event_new(Nina_b112::rx_sampling_handler);
    this->rx_state = UART_RX_STATE_WAIT_START;
    this->rx_baudrate = 115200;
    this->rx_prev_data = 1;

    this->tx_sampling_event = this->event_new(Nina_b112::tx_sampling_handler);
    this->tx_state = UART_TX_STATE_START;
    this->tx_baudrate = 115200;
    this->tx_parity_en = 0;

    return 0;
}

void Nina_b112::start()
{
    //this->set_cts(0);
}


extern "C" vp::component *vp_constructor(js::config *config)
{
    return new Nina_b112(config);
}
