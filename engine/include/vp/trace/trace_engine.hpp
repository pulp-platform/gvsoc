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

#ifndef __VP_TRACE_ENGINE_HPP__
#define __VP_TRACE_ENGINE_HPP__

#include "vp/vp.hpp"
#include "vp/component.hpp"
#include "vp/trace/trace.hpp"

namespace vp {

  class trace_engine : public component
  {
  public:
    trace_engine(const char *config);

    virtual void reg_trace(trace *trace, string path, string name) = 0;

    virtual int get_max_path_len() = 0;

  };

};

#endif
