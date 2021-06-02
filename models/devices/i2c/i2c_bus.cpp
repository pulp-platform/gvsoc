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


#include <vp/vp.hpp>
#include <vp/itf/i2c.hpp>

#include <map>

typedef struct {
    int scl;
    int sda;
} i2c_pair_t;


class I2c_bus : public vp::component
{
public:
    I2c_bus(js::config *config);

    int build();

private:

    static void sync(void *__this, int scl, int sda, int id);

    vp::trace trace;

    vp::i2c_slave in;

    std::map<int, i2c_pair_t> i2c_values;
};


I2c_bus::I2c_bus(js::config *config)
    : vp::component(config)
{
}

int I2c_bus::build()
{
    traces.new_trace("trace", &trace, vp::DEBUG);

    this->in.set_sync_meth_demuxed(&I2c_bus::sync);
    new_slave_port("input", &in);

    return 0;
}


void I2c_bus::sync(void *__this, int scl, int sda, int id)
{
    I2c_bus *_this = (I2c_bus *)__this;

    printf("[%p] SYNC scl %d sda %d id %d\n", _this, scl, sda, id);

    /* store incoming values in maps */
    _this->i2c_values[id].scl = scl;
    _this->i2c_values[id].sda = sda;

    /* browse all values and compute resulting SCL and SDA */
    int res_scl_value = 1;
    int res_sda_value = 1;

    for (std::pair<int, i2c_pair_t> i2c_val : _this->i2c_values)
    {
        if (i2c_val.second.scl == 0)
        {
            res_scl_value = 0;
        }
        if (i2c_val.second.sda == 0)
        {
            res_sda_value = 0;
        }
    }

    /* broadcast the values to all peripherals */
    _this->in.sync(res_scl_value, res_sda_value);
}


extern "C" vp::component *vp_constructor(js::config *config)
{
    return new I2c_bus(config);
}
