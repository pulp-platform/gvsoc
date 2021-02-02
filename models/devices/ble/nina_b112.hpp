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

/**
 * \brief ublox nina-b112 basic model
 *
 * This module is a programmable wireless serial link.
 * It can be configured using AT commands, and then put in data mode to receive
 * and transmit data on the serial interface.
 *
 * JSON configuration can be used to customize its behavior.
 */
class Nina_b112 : public vp::component
{
public:
    /**
     * \brief construct the model using given configuration
     *
     * \param config JSON model configuration
     */
    Nina_b112(js::config *config);

    /**
     * \brief build the model (ports, events, clocks)
     *
     * \returns TODO
     */
    int build();

    /**
     * \brief called when the model starts
     *
     * This function enqueues the initialization event needed to
     * put default UART output pin values.
     */
    void start();

private:

    /**
     * TODO
     */
    static void rx_clk_reg(vp::component *__this, vp::component *clock);

    /**
     * TODO
     */
    static void tx_clk_reg(vp::component *__this, vp::component *clock);

    /**
     * \brief called when UART pins are modified
     *
     * Starting point of incoming data sampling/processing.
     */
    static void sync_full(void *__this, int data, int clk, int rtr);

    /**
     * \brief Update the CTS pin value
     *
     * \param CTS new CTS value
     */
    void set_cts(int cts);

    /**
     * \brief start sampling on UART RX
     *
     * start sampling data on UART RX.
     * Once a whole byte is processed (and stop byte is received), process
     * the received byte.
     */
    void rx_start_sampling();

    /**
     * \brief stop sampling on UART RX
     */
    void rx_stop_sampling();

    /**
     * \brief core rx sampling function
     *
     * Update the RX byte state machine to retrieve a byte.
     * Calls rx_handle_byte if a byte needs to be processed.
     */
    void rx_handle_sampling();

    /**
     * \brief called when a whole byte has been received
     *
     * Triggers the processing of incoming byte.
     * Processing depends on the current model operating mode.
     *
     * \param byte byte received
     */
    void rx_handle_byte(uint8_t byte);

    /**
     * \brief enqueue a tx event if there is a byte to send
     *
     * Check if there is a byte to send, and enqueue an tx event
     * if one is needed.
     */
    void tx_check_pending_byte();

    /**
     * \brief enqueue a buffer to be transmitted on TX
     *
     * \param buffer buffer to be transmitted on UART TX
     */
    void tx_send_buffer(std::string buffer);

    /**
     * \brief send a byte on UART TX
     *
     * Push a byte on the to-be-send queue.
     * Enqueue a tx_event if one is needed.
     *
     * \param byte byte to be sent
     */
    void tx_send_byte(uint8_t byte);

    /**
     * \brief update the TX state machine to send data
     *
     * Update the TX state machine to send enqueued data.
     */
    void tx_send_bit();

    /**
     * \brief parse an AT command, process it and return a response
     *
     * This function is used to process incoming AT command,
     * modify the model behavior and send a response.
     *
     * Not all AT commands supported by the real module
     * are supported. Only a subset is implemented.
     *
     * \param input_cmd buffer of input command
     *
     * \returns response to be sent on serial interface
     */
    std::string parse_cmd(std::string input_cmd);

    /* ========= */
    /* Callbacks */
    /* ========= */

    /**
     * \brief UART RX sampling event handler
     *
     * This handler is called when UART RX needs to sample data.
     *
     * \param ___this pointer to the model
     * \param event event triggering this handler
     */
    static void rx_sampling_handler(void *__this, vp::clock_event *event);

    /**
     * \brief UART TX sampling event handler
     *
     * This handler is called when UART TX needs to sample data.
     *
     * \param ___this pointer to the model
     * \param event event triggering this handler
     */
    static void tx_sampling_handler(void *__this, vp::clock_event *event);

    /**
     * \brief Init event handler
     *
     * This handler is called when the model needs to initialize
     * its ports (RX, CTS, SCK).
     *
     * \param ___this pointer to the model
     * \param event event triggering this handler
     */
    static void init_handler(void *__this, vp::clock_event *event);

    /**
     * \brief CTS end handler
     *
     * This handler is called when we are done delaying incoming data
     * and need to put CTS back to 0.
     *
     * \param ___this pointer to the model
     * \param event event triggering this handler
     */
    static void cts_end_handler(void *__this, vp::clock_event *event);

    /* ====== */
    /* Fields */
    /* ====== */

    /** used to log information */
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

    /* UART RX */
    uart_rx_state_e   rx_state;
    int               rx_baudrate;
    int               rx_nb_bits;
    int               rx_prev_data;
    bool              rx_sampling;
    uint8_t           rx_byte;
    int               rx_rtr;
    vp::clock_event*  rx_sampling_event;


    /* UART TX */
    uart_tx_state_e     tx_state;
    vp::clock_event*    tx_sampling_event;
    std::queue<uint8_t> tx_pending_bytes;
    uint8_t             tx_current_pending_byte;
    int                 tx_pending_bits;
    int                 tx_baudrate;
    int                 tx_cts;
    int                 tx_bit;
    int                 tx_current_stop_bits;
    int                 tx_parity;
    int                 tx_parity_en;

    /* cts fields */
    bool cts_enabled;
    bool cts_trigger;
    int  cts_duration;
    int  cts_bit;

    /* behavior fields */
    nina_b112_operating_mode_e operating_mode;
};
