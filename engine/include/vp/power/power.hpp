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

#ifndef __VP_POWER_POWER_HPP__
#define __VP_POWER_POWER_HPP__

#include "vp/vp_data.hpp"

namespace vp {

  #define VP_POWER_DEFAULT_TEMP  25
  #define VP_POWER_DEFAULT_VOLT  1.2
  #define VP_POWER_DEFAULT_FREQ  50

  class Linear_table;

  class power_trace
  {
  public:
    int init(component *top, std::string name);

    inline bool get_active() { return trace.get_event_active(); }

    inline void account(double quantum);

    inline void incr(double quantum);

    void collect();

    void reg_top_trace(vp::power_trace *trace);

    vp::trace     trace;

  private:
    component *top;
    std::vector<power_trace *> top_traces;
    double value;
    int64_t timestamp;
  };

  class power_source
  {
    friend class component_power;

  public:
    inline void account() { this->trace->account(this->quantum); }

  protected:
    int init(component *top, std::string name, js::config *config, power_trace *trace);
    void setup(double temp, double volt, double freq);

  private:
    Linear_table *table = NULL;
    double quantum;
    component *top;
    power_trace *trace;
  };

};

#endif