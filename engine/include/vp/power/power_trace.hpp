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


#pragma once

#include "vp/vp_data.hpp"
#include "vp/power/power_engine.hpp"



inline double vp::power::power_trace::get_dynamic_energy_for_cycle()
{
  this->flush_dynamic_energy_for_cycle();
  return this->dynamic_energy_for_cycle;
}

inline void vp::power::power_trace::flush_dynamic_energy_for_cycle()
{
  if (this->timestamp < this->top->get_time())
  {
    this->timestamp = this->top->get_time();
    this->dynamic_energy_for_cycle = 0;
  }
}

inline double vp::power::power_trace::get_dynamic_energy()
{
  this->account_dynamic_power();
  return this->total_dynamic_energy;
}

inline double vp::power::power_trace::get_leakage_energy()
{
  this->account_leakage_power();
  return this->total_leakage_energy;
}
