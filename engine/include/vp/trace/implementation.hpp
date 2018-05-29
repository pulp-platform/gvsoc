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


#ifndef __VP_TRACE_IMPLEMENTATION_HPP__
#define __VP_TRACE_IMPLEMENTATION_HPP__

#include "vp/vp_data.hpp"
#include "vp/trace/trace_engine.hpp"

namespace vp {

  inline void vp::trace::event(uint8_t *value)
  {
  #ifdef VP_TRACE_ACTIVE
    if (is_event_active)
    {
      trace_manager->dump_event(this, comp->get_clock()->get_time(), value, bytes);
    }
  #endif
  }

  inline void vp::trace::event_string(uint8_t *value, int size)
  {
  #ifdef VP_TRACE_ACTIVE
  #endif
  }

  inline void vp::trace::event_real(double value)
  {
  #ifdef VP_TRACE_ACTIVE
    if (is_event_active)
    {
      trace_manager->dump_event(this, comp->get_clock()->get_time(), (uint8_t *)&value, 8);
    }  	
  #endif
  }


  inline void vp::trace::user_msg(const char *fmt, ...) {
    #if 0
    fprintf(trace_file, "%ld: %ld: [\033[34m%-*.*s\033[0m] ", comp->get_clock()->get_time(), comp->get_clock()->get_cycles(), max_trace_len, max_trace_len, comp->get_path());
    va_list ap;
    va_start(ap, fmt);
    if (vfprintf(trace_file, format, ap) < 0) {}
    va_end(ap);  
    #endif
  }

  inline void vp::trace::warning(const char *fmt, ...) {
  #ifdef VP_TRACE_ACTIVE
    dump_warning_header();
    va_list ap;
    va_start(ap, fmt);
    if (vfprintf(stdout, fmt, ap) < 0) {}
    va_end(ap);
  #else
  #endif
    #if 0
    printf("%ld: %ld: [\033[31m%-*.*s\033[0m] ", comp->get_clock()->get_time(), comp->get_clock()->get_cycles(), max_trace_len, max_trace_len, comp->get_path());
    va_list ap;
    va_start(ap, fmt);
    if (vprintf(format, ap) < 0) {}
    va_end(ap);  
    comp->get_clock()->stop(vp::CLOCK_ENGINE_WARNING);
    #endif
  }

};

#endif
