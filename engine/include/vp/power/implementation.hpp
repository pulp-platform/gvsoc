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
 * Authors: Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */



#ifndef __VP_POWER_IMPLEMENTATION_HPP__
#define __VP_POWER_IMPLEMENTATION_HPP__

#include "vp/vp_data.hpp"
#include "vp/power/power_engine.hpp"

namespace vp {

  namespace power {

    class Linear_volt_table
    {
    public:
      Linear_volt_table(double temp, js::config *config);
      inline double get(double frequency) { return any; }

      double volt;

    private:
      double any;
    };



    class Linear_temp_table
    {
    public:
      Linear_temp_table(double temp, js::config *config);
      double get(double volt, double frequency);

      double temp;

    private:
      std::vector<Linear_volt_table *> volt_tables;
    };



    class Linear_table
    {
    public:
      Linear_table(js::config *config);
      double get(double temp, double volt, double frequency);

    private:
      std::vector<Linear_temp_table *> temp_tables;
    };

  };

};


inline void vp::power::power_source::leakage_power_start()
{
    if (!this->is_on)
        this->trace->incr_leakage_power(this->quantum);
    this->is_on = true;
}

inline void vp::power::power_source::leakage_power_stop()
{
    if (this->is_on)
        this->trace->incr_leakage_power(-this->quantum);
    this->is_on = false;
}

inline void vp::power::power_source::dynamic_power_start()
{
    if (!this->is_on)
        this->trace->incr_dynamic_power(this->quantum);
    this->is_on = true;
}

inline void vp::power::power_source::dynamic_power_stop()
{
    if (this->is_on)
        this->trace->incr_dynamic_power(-this->quantum);
    this->is_on = false;
}

inline void vp::power::power_source::account_event()
{
  this->trace->account_quantum(this->quantum);
}


inline void vp::power::power_trace::account_quantum(double quantum)
{
  this->incr(quantum);
  this->trace.event_real_pulse(this->top->get_period(), quantum, 0);
}

inline double vp::power::power_trace::get_value()
{
  if (this->timestamp < this->top->get_time())
  {
    this->timestamp = this->top->get_time();
    this->value = 0;
  }
  return this->value;
}

inline double vp::power::power_trace::get_total()
{
  this->account_power();
  return this->total;
}

inline double vp::power::power_trace::get_total_leakage()
{
  this->account_leakage_power();
  return this->total_leakage;
}

#endif