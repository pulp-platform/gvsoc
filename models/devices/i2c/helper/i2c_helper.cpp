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
    bus_is_busy(false),
    scl(1),
    sda(1),
    is_clock_enabled(false),
    is_clock_low(false),
    cb_master_operation(null_callback),
    master_event(parent, this, I2C_helper::st_master_event_handler)
{
    assert(NULL != this->parent);
    assert(NULL != this->itf);

    I2C_HELPER_DEBUG("Initializing helper interface\n");
}

void I2C_helper::st_master_event_handler(void* __this, vp::clock_event* event)
{
    assert(NULL != __this);
    assert(NULL != event);

    I2C_HELPER_DEBUG("st_master_event_handler: none\n");
    I2C_helper* _this = (I2C_helper*) __this;
    _this->master_event_handler(event);
}

void I2C_helper::master_event_handler(vp::clock_event* event)
{
    assert(NULL != event);

    I2C_HELPER_DEBUG("master_event_handler: none\n");

    /* clock toggling */
    if (this->is_clock_enabled)
    {
        if (this->is_clock_low)
        {
            /* switch to high */
            this->itf->sync(1, this->sda);
        }
        else
        {
            /* switch to low */
            this->itf->sync(0, this->sda);
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
        this->itf->sync(1, 0); //falling edge to trigger a start
        this->start_clock();
    }
    else
    {
        //TODO report error to user
    }
}

bool I2C_helper::is_busy(void)
{
    return this->bus_is_busy;
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
    (void) byte;
    I2C_HELPER_DEBUG("send_data: byte=%d\n", byte);
    //TODO
}

void I2C_helper::send_stop(void)
{
    I2C_HELPER_DEBUG("send_stop: none\n");
    if(this->is_busy())
    {
        //TODO need to trigger on next scl high ?
        this->itf->sync(1, 1); // sda rising edge while scl is 1
        this->stop_clock();
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
}

void I2C_helper::fsm_step(int input_scl, int input_sda)
{
    I2C_HELPER_DEBUG("fsm_step: input_scl=%d, input_sda=%d\n", input_scl, input_sda);

    bool scl_rising = (input_scl == 1 && this->scl == 0);
    bool scl_falling = (input_scl == 0 && this->scl == 1);
    bool scl_steady = (input_scl == this->scl);

    bool sda_rising = (input_sda == 1 && this->sda == 0);
    bool sda_falling = (input_sda == 0 && this->sda == 1);
    I2C_HELPER_DEBUG("fsm_step: sda=%d, this->sda=%d\n", input_sda, this->sda);
    I2C_HELPER_DEBUG("fsm_step: this=%p\n", (void*) this);

    this->scl = input_scl;
    this->sda = input_sda;

    if (scl_steady)
    {
        /* START/STOP detection */
        if (this->scl == 1)
        {
            if (sda_falling && !this->is_busy())
            {
                I2C_HELPER_DEBUG("START DETECTED\n");
                this->bus_is_busy = true;
            }
            else if (sda_rising && this->is_busy())
            {
                I2C_HELPER_DEBUG("STOP DETECTED\n");
                this->bus_is_busy = false;
            }
        }
    }
    else if (this->is_busy())
    {
        /* sampling bit*/
        I2C_HELPER_DEBUG("fsm_step: sampling rising bit\n");
        if (scl_rising)
        {
            this->sda_rise = this->sda;
        }
        else if (scl_falling)
        {
            if (this->sda_rise == this->sda)
            {
                this->recv_bit_queue.push(this->sda);
            }
            else
            {
                //TODO framing error ?
                //TODO empty queue
                I2C_HELPER_DEBUG("FRAMING ERROR!\n");
            }

            if (this->recv_bit_queue.size() == 8)
            {
                int byte = 0;
                /* full byte received */
                for (int i = 0; i < 8; i++)
                {
                    int bit = this->recv_bit_queue.front();
                    this->recv_bit_queue.pop();
                    byte = byte << 1 & bit;
                }
                assert(this->recv_bit_queue.empty());

                I2C_HELPER_DEBUG("fsm_step: byte received=%d\n", byte);
                //TODO what ?
            }
        }
    }

    /* clock management */
    if(this->is_clock_enabled && !scl_steady)
    {
        /* manages clock synchronization and clock stretching automatically */
        if(scl_rising)
        {
            this->is_clock_low = false;
        }
        else if (scl_falling)
        {
            this->is_clock_low = true;
        }
        this->enqueue_clock_toggle();
    }
}

void I2C_helper::enqueue_clock_toggle(void)
{
    //TODO
    I2C_HELPER_DEBUG("enqueue_clock_toggle\n");
    if (this->is_clock_enabled)
    {
        const uint64_t delay = this->is_clock_low ? delay_low_ps : this->delay_high_ps;
        this->enqueue_event(&this->master_event, delay);
    }
}
