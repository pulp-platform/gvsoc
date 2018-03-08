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

#ifndef __VP_TRACE_TRACE_HPP__
#define __VP_TRACE_TRACE_HPP__

#define VP_TRACE_ACTIVE 1

#include "vp/vp.hpp"
#include <stdarg.h>

namespace vp {

  class trace
  {

    friend class component_trace;

  public:

    inline void msg(const char *fmt, ...);
    inline void user_msg(const char *fmt, ...);
    inline void warning(const char *fmt, ...);

    void dump_header();
    void dump_warning_header();

    void set_active(bool active) { is_active = active; }

    bool get_active() { return is_active; }

  protected:
    int level;
    component *comp;
    FILE *trace_file;
    bool is_active = false;
    string name;
  };    

  inline void vp::trace::msg(const char *fmt, ...) {
  #ifdef VP_TRACE_ACTIVE
    if (is_active)
    {
      dump_header();
      va_list ap;
      va_start(ap, fmt);
      if (vfprintf(stdout, fmt, ap) < 0) {}
      va_end(ap);  
    }
  #else
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
