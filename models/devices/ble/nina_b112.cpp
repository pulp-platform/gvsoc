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

#include "nina_b112.hpp"

#include <stdio.h>

#include <regex>

Nina_b112::Nina_b112(js::config *config)
    : vp::component(config)
{
}


void Nina_b112::sync_full(void *__this, int data, int clk, int rtr)
{
    Nina_b112 *_this = (Nina_b112 *)__this;

    _this->tx_cts = rtr;

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
    static std::string input_buffer("");

    this->trace.msg(vp::trace::LEVEL_TRACE, "Received byte 0x%x\n", byte);

    /* add incoming byte to input buffer */
    input_buffer.push_back((char)byte);
    //this->trace.msg(vp::trace::LEVEL_TRACE, "Input_buffer (size: %d): %s\n", input_buffer.size(), input_buffer.c_str());

    switch(this->operating_mode)
    {
        case NINA_B112_OPERATING_MODE_COMMAND:
            {
                /* only seach for a command when byte is '\r' */
                if (byte == '\r')
                {
                    /* search for a command in the input_buffer */
                    std::regex command_regexp("AT.*\r");
                    std::smatch match;

                    if(std::regex_match(input_buffer, match, command_regexp))
                    {
                        std::string received_command("");
                        received_command.append(match[0]);
                        this->trace.msg(vp::trace::LEVEL_TRACE, "Received command(size: %d) %s\n",
                                received_command.size(),
                                received_command.c_str());

                        std::string input_cmd = input_buffer;
                        input_cmd.erase(0, 2);
                        input_cmd.pop_back();

                        /* act depending on received command */
                        this->tx_send_buffer(this->parse_cmd(input_cmd));

                        /* reset input buffer */
                        input_buffer.clear();
                    }
                }
                break;
            }
        case NINA_B112_OPERATING_MODE_DATA:
            {
                /* store a buffer, and then send it back */
                //this->trace.msg(vp::trace::LEVEL_TRACE, "Data operating mode\n");
                if (input_buffer.size() >= 200)
                {
                    this->trace.msg(vp::trace::LEVEL_INFO, "Sending back data received\n");
                    this->tx_send_buffer(input_buffer);
                    input_buffer.clear();
                }
                break;
            }
        default:
            {
                this->trace.msg(vp::trace::LEVEL_TRACE, "Unexpected operating mode\n");
                break;
            }

    }
}


void Nina_b112::rx_handle_sampling()
{
    //this->trace.msg(vp::trace::LEVEL_TRACE, "Sampling bit (value: %d)\n", this->rx_prev_data);

    static int bytes_counter = 0;

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

            if (this->rtr_trigger && (this->rtr_bit == this->rx_nb_bits))
            {
                this->trace.msg(vp::trace::LEVEL_TRACE, "triggering cts\n", this->rx_prev_data);
                this->rx_clock->enqueue(this->rtr_event, this->rtr_duration);
                this->set_rtr(1);
            }

            break;

        case UART_RX_STATE_WAIT_STOP:

            if (this->rx_prev_data == 1)
            {
                bytes_counter++;
                this->trace.msg(vp::trace::LEVEL_TRACE, "Received stop bit\n");
                this->rx_state = UART_RX_STATE_WAIT_START;
                this->rx_stop_sampling();

                /* decide if next byte will trigger cts */
                if (this->rtr_enabled && (bytes_counter > 10))
                {
                    this->trace.msg(vp::trace::LEVEL_INFO, "triggering cts on next byte\n");
                    bytes_counter = 0;
                    this->rtr_trigger = true;
                    this->rtr_bit = ((this->rtr_bit + 1) % 8) + 1;
                    this->rtr_duration = 100;
                }

            }
            break;
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


void Nina_b112::set_rtr(int rtr)
{
    this->rx_rtr = rtr;
    //this->trace.msg(vp::trace::LEVEL_TRACE, "SET CTS\n");
    this->uart_itf.sync_full(this->tx_bit, 2, this->rx_rtr);
}


void Nina_b112::tx_sampling_handler(void *__this, vp::clock_event *event)
{
    Nina_b112 *_this = (Nina_b112 *)__this;
    _this->tx_send_bit();
}


void Nina_b112::tx_send_bit()
{
    int bit = 1;
    static int bits_sent;

    //TODO need to implement CTS support

    switch (this->tx_state)
    {
        case UART_TX_STATE_IDLE:
        {
            if(!this->tx_pending_bytes.empty())
            {
                this->trace.msg(vp::trace::LEVEL_TRACE, "Initiating new byte transfer\n");
                this->tx_state = UART_TX_STATE_START;
            }
            break;
        }

        case UART_TX_STATE_START:
        {
            if(!this->tx_pending_bytes.empty())
            {
                this->trace.msg(vp::trace::LEVEL_TRACE, "Sending start bit\n");
                this->tx_parity = 0;
                this->tx_state = UART_TX_STATE_DATA;
                this->tx_current_stop_bits = 1;

                this->tx_current_pending_byte = this->tx_pending_bytes.front();
                this->tx_pending_bytes.pop();
                this->tx_pending_bits = 8;

                bit = 0;
                bits_sent = 0;
            }
            break;
        }

        case UART_TX_STATE_DATA:
        {
            if (this->tx_pending_bits > 0)
            {
                bits_sent++;
                bit = this->tx_current_pending_byte & 1;
                this->trace.msg(vp::trace::LEVEL_TRACE, "Sending data bit #%d (value: %d)\n", bits_sent, bit);
                this->tx_current_pending_byte >>= 1;
                this->tx_pending_bits -= 1;
                this->tx_parity ^= bit;

            }
            else
            {
                if (this->tx_parity_en)
                {
                    this->tx_state = UART_TX_STATE_PARITY;
                }
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
                this->tx_state = UART_TX_STATE_IDLE;
            }

            break;
        }
    }

    /* send data bit */
    this->tx_bit = bit;
    this->uart_itf.sync_full(this->tx_bit, 2, this->rx_rtr);

    /* always reenqueue */
    //TODO remove this infinite reenqueue
    this->tx_clock->reenqueue(this->tx_sampling_event, 2);
}

/**
 * \brief Send a buffer of bytes on TX
 *
 * \param buffer buffer of bytes to send
 */
void Nina_b112::tx_send_buffer(std::string buffer)
{
    for (size_t i = 0; i < buffer.size(); i++)
    {
        this->tx_send_byte(buffer[i]);
    }
}

void Nina_b112::tx_check_pending_byte()
{
    if (!this->tx_pending_bytes.empty() && !this->tx_sampling_event->is_enqueued())
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
    this->tx_pending_bytes.push(byte);
    //this->tx_state = UART_TX_STATE_START;
    this->tx_clock_cfg.set_frequency(this->tx_baudrate*2);
    this->tx_check_pending_byte();
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

    this->init_event = this->event_new(Nina_b112::init_handler);
    this->rtr_event = this->event_new(Nina_b112::rtr_end_handler);

    this->rx_sampling_event = this->event_new(Nina_b112::rx_sampling_handler);
    this->rx_state = UART_RX_STATE_WAIT_START;
    this->rx_baudrate = 115200;
    this->rx_prev_data = 1;

    this->tx_sampling_event = this->event_new(Nina_b112::tx_sampling_handler);
    this->tx_state = UART_TX_STATE_IDLE;
    this->tx_baudrate = 115200;
    this->tx_parity_en = 0;
    this->tx_bit = 1;

    this->rx_rtr = 0; /* we are ready to receive */
    this->rtr_enabled = true;
    this->rtr_trigger =false;
    this->rtr_duration = 0;
    this->rtr_bit = 0;

    this->operating_mode = NINA_B112_OPERATING_MODE_COMMAND;

    return 0;
}

void Nina_b112::start()
{
    this->trace.msg(vp::trace::LEVEL_TRACE, "starting nina b112 model\n");

    /* Initialize tx with 1 */
    this->uart_itf.sync_full(1, 2, 2);
    this->rx_clock->enqueue(this->init_event, 1);
}


void Nina_b112::init_handler(void *__this, vp::clock_event *event)
{
    Nina_b112 *_this = (Nina_b112 *)__this;

    _this->uart_itf.sync_full(1, 2, 0);
}

void Nina_b112::rtr_end_handler(void *__this, vp::clock_event *event)
{
    Nina_b112 *_this = (Nina_b112 *)__this;

    _this->trace.msg(vp::trace::LEVEL_TRACE, "rtr_end_handler\n");
    _this->rtr_trigger = false;
    _this->set_rtr(0);
}


extern "C" vp::component *vp_constructor(js::config *config)
{
    return new Nina_b112(config);
}
