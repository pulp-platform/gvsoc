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

#ifndef __VP_COMPONENT_HPP__
#define __VP_COMPONENT_HPP__

#include "vp/vp_data.hpp"
#include <map>
#include <list>
#include <string>
#include <vector>
#include <functional>
#include "vp/ports.hpp"
#include "vp/comp-model/config.hpp"
#include "vp/clock/clock_event.hpp"
#include "vp/itf/clk.hpp"
#include "vp/comp-model/cm.hpp"
#include "vp/clock/component_clock.hpp"
#include "vp/trace/component_trace.hpp"

using namespace std;

namespace vp {

  class config;

  class clock_engine;

  class component : public cm::component, public component_clock
  {

  public:
    component(const char *config);

    void post_post_build();

    void pre_build() {
      component_clock::pre_build(this);
    }


    component_trace traces;


  };

};  

#endif
