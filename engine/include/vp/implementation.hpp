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

#ifndef __VP_IMPLEMENTATION_HPP__
#define __VP_IMPLEMENTATION_HPP__

#include <map>
#include <list>
#include <string>
#include <vector>

#include "vp/vp.hpp"
#include "vp/clock/implementation.hpp"

using namespace std;

namespace vp {

  inline config *component::get_config() {
    return comp_config;
  }


  inline int component::get_config_int(std::string name, int index) {
    config *conf = get_config(name, index);
    if (conf) return conf->get_int();
    return 0;
  }


  inline int component::get_config_int(std::string name) {
    config *conf = get_config(name);
    if (conf) return conf->get_int();
    return 0;
  }

  inline bool component::get_config_bool(std::string name) {
    config *conf = get_config(name);
    if (conf) return conf->get_bool();
    return false;
  }

  inline std::string component::get_config_str(std::string name) {
    config *conf = get_config(name);
    if (conf) return conf->get_str();
    return "";
  }

  inline config *component::get_config(std::string name) {
    return comp_config->get(name);
  }

  inline config *component::get_config(std::string name, int index) {
    config *conf = comp_config->get(name);
    if (conf == NULL) return NULL;
    return conf->get_elem(index);
  }


};

#endif
