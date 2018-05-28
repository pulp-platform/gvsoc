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

#include "vp/vp.hpp"
#include "vp/trace/trace.hpp"

vp::component_power::component_power(vp::component &top)
: top(top)
{
}

void vp::component_power::new_event(std::string name, power_trace *trace, js::config *config)
{
  if (config == NULL)
    throw logic_error("Didn't find power trace (name: " + name + ")");

  js::config *type_cfg = config->get("type");
  if (type_cfg == NULL)
    throw logic_error("Didn't find power trace type (name: " + name + ")");
}
