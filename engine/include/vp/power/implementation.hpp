/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
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
 * Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 */



#ifndef __VP_POWER_IMPLEMENTATION_HPP__
#define __VP_POWER_IMPLEMENTATION_HPP__

#include "vp/vp_data.hpp"


inline void vp::power_trace::account(double quantum)
{
  this->trace.event_real(quantum);

  for (auto &x: top_traces)
  {
    x->incr(quantum);
  }
}

inline void vp::power_trace::incr(double quantum)
{
  if (this->timestamp < this->top->get_time())
  {
    this->timestamp = this->top->get_time();
    this->value = 0;
  }

  this->value += quantum;

  for (auto &x: top_traces)
  {
    x->incr(quantum);
  }

  this->trace.event_real_delayed(this->value);
}

#endif