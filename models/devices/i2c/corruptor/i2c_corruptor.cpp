/*
 * Copyright (C) 2021 GreenWaves Technologies, SAS, ETH Zurich and
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

#include "i2c_corruptor.hpp"

#include "stdio.h"
#include <cassert>

#define CORRUPTOR_DEBUG(...) (this->trace.msg(vp::trace::LEVEL_TRACE, __VA_ARGS__))

I2c_corruptor::I2c_corruptor(js::config* config)
    : vp::component(config),
    i2c_helper(this,
            &this->i2c_itf,
            std::bind(&I2c_corruptor::i2c_enqueue_event,
                this,
                std::placeholders::_1,
                std::placeholders::_2)
            )
{
    assert(NULL != config);
    /* set helper callback */
    this->i2c_helper.register_callback(std::bind(&I2c_corruptor::i2c_helper_callback,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3));
}

void I2c_corruptor::i2c_sync(void *__this, int scl, int sda)
{
    assert(NULL != __this);
    I2c_corruptor* _this = (I2c_corruptor*) __this;
    _this->i2c_helper.update_pins(scl, sda);
}


void I2c_corruptor::reset(bool active)
{
    if (active)
    {
        this->trace.msg(vp::trace::LEVEL_TRACE, "Resetting component\n");
        //TODO reset i2c interface
    }
}

int I2c_corruptor::build(void)
{
    traces.new_trace("trace", &this->trace, vp::DEBUG);
    this->trace.msg(vp::trace::LEVEL_TRACE, "Building component\n");

    this->i2c_itf.set_sync_meth(&I2c_corruptor::i2c_sync);
    this->new_master_port("i2c", &this->i2c_itf);

    this->new_master_port("clock_cfg", &this->clock_cfg);

    this->trace.msg(vp::trace::LEVEL_INFO, "Instantiated corruptor\n");
    return 0;
}

void I2c_corruptor::start(void)
{
}

void I2c_corruptor::i2c_helper_callback(i2c_operation_e id, i2c_status_e status, int value)
{
    CORRUPTOR_DEBUG("CALLBACK id=%d, status=%d, value=%d !\n",
            id, status, value);

    switch(id)
    {
        case MASTER_START:
            break;
        case MASTER_DATA:
            break;
        case MASTER_ACK:
            break;
        case MASTER_STOP:
            break;
        default:
            break;
    }
}

void I2c_corruptor::i2c_enqueue_event(vp::clock_event* event, uint64_t time_unit)
{
    this->event_enqueue(event, time_unit);
}

extern "C" vp::component *vp_constructor(js::config *config)
{
    return new I2c_corruptor(config);
}
