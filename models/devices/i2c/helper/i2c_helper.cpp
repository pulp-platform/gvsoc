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

#define I2C_HELPER_DEBUG(...)    (fprintf(stderr, "[I2C-PHY] " __VA_ARGS__))

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
    prescaler(10),
    delay_low(5),
    delay_high(5),
    bus_is_busy(false),
    scl(1),
    sda(1),
    cb_master_operation(null_callback),
    master_event(parent, this, I2C_helper::st_master_event_handler)
{
    assert(NULL != this->parent);
    assert(NULL != this->itf);

    this->itf->set_sync_meth(&I2C_helper::i2c_sync);
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
    // TODO
    // - call cb_master_operation with correct parameters
}

void I2C_helper::register_callback(i2c_callback_t callback)
{
    I2C_HELPER_DEBUG("register_callback: none\n");
    this->cb_master_operation = callback;
}

void I2C_helper::set_timings(int prescaler, int delay_low, int delay_high)
{
    I2C_HELPER_DEBUG("set_timings: prescaler=%d, delay_low=%d, delay_high=%d\n",
            prescaler,
            delay_low,
            delay_high);
    this->prescaler = prescaler;
    this->delay_low = delay_low;
    this->delay_high = delay_high;
}

void I2C_helper::send_start(void)
{
    I2C_HELPER_DEBUG("send_start: none\n");
    if (!this->is_busy())
    {
        I2C_HELPER_DEBUG("send_start: sda=%d, scl=%d\n", this->sda, this->scl);
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
    }
}

void I2C_helper::i2c_sync(void *__this, int input_scl, int input_sda)
{
    assert(NULL != __this);
    I2C_helper* _this = (I2C_helper*) __this;
    I2C_HELPER_DEBUG("i2c_sync: scl=%d, sda=%d\n", input_scl, input_sda);
    _this->fsm_step(input_scl, input_sda);
}

void I2C_helper::start_clock(void)
{
    I2C_HELPER_DEBUG("Starting clock\n");
}

void I2C_helper::stop_clock(void)
{
    I2C_HELPER_DEBUG("Stop clock\n");
}

void I2C_helper::fsm_step(int input_scl, int input_sda)
{
    I2C_HELPER_DEBUG("fsm_step\n");

    bool scl_rising = (input_scl == 1 && this->scl == 0);
    bool scl_falling = (input_scl == 0 && this->scl == 1);
    bool scl_steady = (input_scl == this->scl);

    bool sda_rising = (input_sda == 1 && this->sda == 0);
    bool sda_falling = (input_sda == 0 && this->sda == 1);
    I2C_HELPER_DEBUG("sda=%d, this->sda=%d\n", input_sda, this->sda);

    this->scl = input_scl;
    this->sda = input_sda;

    if (scl_steady)
    {
        if (this->scl == 1)
        {
            if (sda_falling)
            {
                I2C_HELPER_DEBUG("START DETECTED\n");
                this->bus_is_busy = true;
            }
            else if (sda_rising)
            {
                I2C_HELPER_DEBUG("STOP DETECTED\n");
                this->bus_is_busy = false;
            }
        }
    }
}
