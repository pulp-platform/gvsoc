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

#pragma once

#include <vp/vp.hpp>
#include <vp/itf/clock.hpp>
#include <vp/itf/i2c.hpp>

#include <vector>

#include "i2c_helper.hpp"

class I2c_corruptor : public vp::component
{
    public:
        I2c_corruptor(js::config* config);

        int build(void);
        void start(void);
        void reset(bool active);

    private:
        void i2c_enqueue_event(vp::clock_event* event, uint64_t time_ps);
        static void i2c_sync(void *__this, int scl, int sda);
        void i2c_helper_callback(i2c_operation_e id, i2c_status_e status, int value);

        /**********/
        /* FIELDS */
        /**********/

        /** trace utility */
        vp::trace trace;

        /** I2C interface */
        vp::i2c_master i2c_itf;

        /* clock related */
        vp::clock_master clock_cfg;

        /** I2C slave helper */
        I2C_helper i2c_helper;
};