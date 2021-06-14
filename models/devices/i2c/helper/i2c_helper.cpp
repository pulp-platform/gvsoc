/*
 * Copyright (C) 2021  GreenWaves Technologies, SAS
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "i2c_helper.hpp"

#include <stdio.h>
#include <cassert>

#define I2C_HELPER_DEBUG(...)    (fprintf(stderr, "[I2C-HLP] " __VA_ARGS__))
//#define I2C_HELPER_DEBUG(...)

namespace {
    void null_callback(i2c_operation_e id, i2c_status_e status, int value)
    {
        (void) id;
        (void) status;
        (void) value;
        I2C_HELPER_DEBUG("null callback: id=%d, status=%d, value=%d\n",
                id, status, value);
    }
}

I2C_helper::I2C_helper(vp::component* parent, vp::i2c_master* itf, i2c_enqueue_event_fn_t enqueue_event) :
    parent(parent),
    itf(itf),
    enqueue_event(enqueue_event),
    delay_low_ps(5),
    delay_high_ps(5),
    internal_state(I2C_INTERNAL_IDLE),
    scl(1),
    sda(1),
    desired_scl(1),
    desired_sda(1),
    sda_rise(1),
    is_starting(false),
    is_stopping(false),
    is_clock_enabled(false),
    is_clock_low(false),
    is_driving_scl(false),
    is_driving_sda(false),
    cb_master_operation(null_callback),
    clock_event(parent, this, I2C_helper::st_clock_event_handler),
    data_event(parent, this, I2C_helper::st_data_event_handler)
{
    assert(NULL != this->parent);
    assert(NULL != this->itf);

    I2C_HELPER_DEBUG("Initializing helper interface\n");
}

void I2C_helper::st_data_event_handler(void* __this, vp::clock_event* event)
{
    assert(NULL != __this);
    assert(NULL != event);

    I2C_HELPER_DEBUG("st_data_event_handler: none\n");
    I2C_helper* _this = (I2C_helper*) __this;
    _this->desired_sda = _this->expected_bit_value;
    _this->sync_pins();
}

void I2C_helper::st_clock_event_handler(void* __this, vp::clock_event* event)
{
    assert(NULL != __this);
    assert(NULL != event);

    I2C_HELPER_DEBUG("st_clock_event_handler: none\n");
    I2C_helper* _this = (I2C_helper*) __this;
    _this->clock_event_handler(event);
}

void I2C_helper::clock_event_handler(vp::clock_event* event)
{
    assert(NULL != event);

    I2C_HELPER_DEBUG("clock_event_handler: none\n");

    /* clock toggling */
    if (this->is_clock_enabled)
    {
        if (this->is_clock_low)
        {
            I2C_HELPER_DEBUG("clock_event_handler: LOW (switch to high)\n");
            /* switch to high */
            this->desired_scl = 1;
            this->sync_pins();
        }
        else
        {
            I2C_HELPER_DEBUG("clock_event_handler: HIGH (switch to low)\n");
            /* switch to low */
            this->desired_scl = 0;
            this->sync_pins();
        }
    }
}

void I2C_helper::register_callback(i2c_callback_t callback)
{
    I2C_HELPER_DEBUG("register_callback: none\n");
    this->cb_master_operation = callback;
}

void I2C_helper::update_pins(int scl, int sda)
{
    this->fsm_step(scl, sda);
}


void I2C_helper::sync_pins(void)
{
    int res_scl = this->is_driving_scl ? this->desired_scl : 1;
    int res_sda = this->is_driving_sda ? this->desired_sda : 1;

    I2C_HELPER_DEBUG("sync_pins: scl=%d, sda=%d\n", res_scl, res_sda);
    this->itf->sync(res_scl, res_sda);
}


void I2C_helper::set_timings(uint64_t delay_low_ps, uint64_t delay_high_ps)
{
    I2C_HELPER_DEBUG("set_timings: delay_low_ps=%ld, delay_high_ps=%ld\n",
            delay_low_ps,
            delay_high_ps);
    this->delay_low_ps = delay_low_ps;
    this->delay_high_ps = delay_high_ps;
}

void I2C_helper::send_start(void)
{
    I2C_HELPER_DEBUG("send_start: none\n");
    if (!this->is_busy())
    {
        I2C_HELPER_DEBUG("send_start: sda=%d, scl=%d\n", this->sda, this->scl);
        I2C_HELPER_DEBUG("send_start: this=%p\n", (void*) this);
        this->is_driving_scl = true;
        this->is_driving_sda = true;
        this->desired_scl = 1;
        this->desired_sda = 0;
        this->sync_pins();
        this->start_clock();
    }
    else
    {
        this->is_starting = true;
    }
}

bool I2C_helper::is_busy(void)
{
    return (this->internal_state != I2C_INTERNAL_IDLE);
}

void I2C_helper::send_address(int addr, bool is_write, bool is_10bits)
{
    I2C_HELPER_DEBUG("send_address: addr=%d, is_10bits=%s\n",
            addr,
            is_10bits ? "true" : "false");
    //TODO
}

void I2C_helper::send_data(int byte)
{
    I2C_HELPER_DEBUG("send_data: byte=%d\n", byte);
    // TODO verify that we are in data mode ?

    /* load byte in sending queue */
    for (int i = 7; i >= 0; i--)
    {
        int bit = (byte >> i) & 1;
        I2C_HELPER_DEBUG("push to send bit queue:%d\n", bit);
        this->send_bit_queue.push(bit);
    }

    /* enqueue data change if clock is low,
     * else will be done automatically at next falling scl */
    if (this->is_clock_low)
    {
        I2C_HELPER_DEBUG("Directly enqueueing!\n");
        this->expected_bit_value = this->send_bit_queue.front();
        this->send_bit_queue.pop();
        this->enqueue_data_change(this->expected_bit_value);
    }
}

void I2C_helper::send_ack(bool ack)
{
    I2C_HELPER_DEBUG("send_ack: ack=%s\n", ack ? "true" : "false");
    //TODO
}

void I2C_helper::send_stop(void)
{
    I2C_HELPER_DEBUG("send_stop: none\n");
    if(this->is_busy())
    {
        this->is_stopping = true;
    }
}

void I2C_helper::start_clock(void)
{
    I2C_HELPER_DEBUG("Starting clock\n");

    //start high then loop(low -> high)
    this->is_clock_enabled = true;
    this->is_clock_low = false;
    this->enqueue_clock_toggle();
}

void I2C_helper::stop_clock(void)
{
    I2C_HELPER_DEBUG("Stop clock\n");
    this->is_clock_enabled =false;

    this->desired_scl = 1;

    this->is_driving_scl = false;
    this->is_driving_sda = false;
    this->sync_pins();
}

void I2C_helper::fsm_step(int input_scl, int input_sda)
{
    bool scl_rising = (input_scl == 1 && this->scl == 0);
    bool scl_falling = (input_scl == 0 && this->scl == 1);
    bool scl_steady = (input_scl == this->scl);

    bool sda_rising = (input_sda == 1 && this->sda == 0);
    bool sda_falling = (input_sda == 0 && this->sda == 1);
    I2C_HELPER_DEBUG("\n\n\n");
    I2C_HELPER_DEBUG("fsm_step: input_scl=%d, input_sda=%d\n", input_scl, input_sda);
    I2C_HELPER_DEBUG("fsm_step: sda=%d, this->sda=%d\n", input_sda, this->sda);
    I2C_HELPER_DEBUG("fsm_step: this=%p\n", (void*) this);

    this->scl = input_scl;
    this->sda = input_sda;

    /* clock management */
    if (!scl_steady)
    {
        /* manages clock synchronization and clock stretching automatically */
        if (scl_rising)
        {
            this->is_clock_low = false;
        }
        else if (scl_falling)
        {
            this->is_clock_low = true;
        }

        if (this->is_clock_enabled)
        {
            this->enqueue_clock_toggle();
        }
    }

    /* I2C logic */
    if (scl_steady)
    {
        /* START/STOP detection */
        if (this->scl == 1)
        {
            if (sda_falling && !this->is_busy())
            {
                this->is_starting = false;
                this->internal_state = I2C_INTERNAL_DATA;

                this->sda_rise = this ->sda;
                I2C_HELPER_DEBUG("START DETECTED\n");
                this->cb_master_operation(MASTER_START, MASTER_OK, 0);
            }
            else if (sda_rising && this->is_busy())
            {
                this->internal_state = I2C_INTERNAL_IDLE;
                this->is_stopping = false;
                /* stop clock */
                this->stop_clock();

                I2C_HELPER_DEBUG("STOP DETECTED\n");
                this->cb_master_operation(MASTER_STOP, MASTER_OK, 0);
            }
        }
    }
    else if (this->is_busy())
    {
        /* sampling bit*/
        if (scl_rising)
        {
            I2C_HELPER_DEBUG("SCL rising\n");
            I2C_HELPER_DEBUG("fsm_step: sampling rising bit\n");
            this->sda_rise = this->sda;
            //TODO add check expected_bit_value
            if (is_stopping)
            {
                this->expected_bit_value = 1;
                this->enqueue_data_change(this->expected_bit_value);
            }
            else if (is_starting)
            {
                this->expected_bit_value = 0;
                this->enqueue_data_change(this->expected_bit_value);
            }
        }
        else if (scl_falling)
        {
            I2C_HELPER_DEBUG("SCL falling\n");
            I2C_HELPER_DEBUG("INTERNAL_STATE = %d\n", this->internal_state);
            if (this->sda_rise == this->sda)
            {
                this->recv_bit_queue.push(this->sda);
                I2C_HELPER_DEBUG("fsm_step: received bit=%d (#%ld)\n", this->sda, this->recv_bit_queue.size());
            }
            else
            {
                //TODO framing error ?
                //TODO empty queue
                I2C_HELPER_DEBUG("FRAMING ERROR!, sda_rise=%d, sda=%d\n", this->sda_rise, this->sda);
                this->cb_master_operation(MASTER_STOP, ANY_ERROR_FRAMING, 0);
            }

            if (is_stopping)
            {
                this->expected_bit_value = 0;
                this->enqueue_data_change(this->expected_bit_value);
            }
            else if (is_starting)
            {
                this->expected_bit_value = 1;
                this->enqueue_data_change(this->expected_bit_value);
            }
            else if (this->internal_state == I2C_INTERNAL_DATA)
            {
                /* send data */
                if (!this->send_bit_queue.empty())
                {
                    int bit = this->send_bit_queue.front();
                    this->send_bit_queue.pop();
                    this->expected_bit_value = bit;

                    this->enqueue_data_change(this->expected_bit_value);
                }

                /* receiving data */
                if (this->recv_bit_queue.size() == 8)
                {
                    int byte = 0;
                    /* full byte received */
                    for (int i = 0; i < 8; i++)
                    {
                        int bit = this->recv_bit_queue.front();
                        this->recv_bit_queue.pop();
                        byte = byte << 1 | bit;
                    }
                    assert(this->recv_bit_queue.empty());

                    I2C_HELPER_DEBUG("fsm_step: byte received=%d\n", byte);

                    this->cb_master_operation(MASTER_DATA, MASTER_OK, byte);

                    this->internal_state = I2C_INTERNAL_ACK;
                }
            }
            else if (this->internal_state == I2C_INTERNAL_ACK)
            {
                if (this->recv_bit_queue.size() == 1)
                {
                    const int bit = this->recv_bit_queue.front();
                    this->recv_bit_queue.pop();

                    I2C_HELPER_DEBUG("fsm_step: ACK received=%d\n", bit);

                    const i2c_status_e status = (bit == 1) ? MASTER_KO : MASTER_OK;
                    this->cb_master_operation(MASTER_ACK, status, 0);

                    assert(this->recv_bit_queue.empty());

                    this->internal_state = I2C_INTERNAL_DATA;
                }
            }
        }
    }
}

void I2C_helper::enqueue_clock_toggle(void)
{
    I2C_HELPER_DEBUG("enqueue_clock_toggle\n");
    if (this->is_clock_enabled)
    {
        if (!this->clock_event.is_enqueued())
        {
            const uint64_t delay = this->is_clock_low ? this->delay_low_ps : this->delay_high_ps;
            this->enqueue_event(&this->clock_event, delay);
        }
        else
        {
            assert(false);
        }
    }
}

void I2C_helper::enqueue_data_change(int new_sda)
{
    I2C_HELPER_DEBUG("enqueue_data_change: %d\n", new_sda);
    if (!this->data_event.is_enqueued())
    {
        this->enqueue_event(&this->data_event, 1);
    }
    else
    {
        assert(false);
    }
}
