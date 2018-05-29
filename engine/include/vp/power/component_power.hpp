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

#ifndef __VP_TRACE_COMPONENT_POWER_HPP__
#define __VP_TRACE_COMPONENT_POWER_HPP__

#include "vp/component.hpp"
#include "json.hpp"

using namespace std;

namespace vp {

  class power_trace;
  class power_source;

  class component_power
  {

  public:

    component_power(component &top);

    int new_event(std::string name, power_source *source, js::config *config, power_trace *trace);
    int new_trace(std::string name, power_trace *trace);

  protected:

  private:
    component &top;

  };

};  


#endif
