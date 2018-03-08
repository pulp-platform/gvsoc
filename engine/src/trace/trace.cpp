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

#include "vp/trace/trace.hpp"
#include "vp/trace/trace_engine.hpp"


vp::component_trace::component_trace(vp::component &top)
: top(top)
{
}

void vp::component_trace::new_trace(std::string name, trace *trace, trace_level_e level)
{
  traces[name] = trace;
  trace->level = level;
  trace->comp = static_cast<vp::component *>(&top);
  trace->name = top.get_path() + "/" + name;
}

void vp::component_trace::post_post_build()
{
  trace_manager = (vp::trace_engine *)top.get_service("trace");
  for (auto& x: traces) {
    trace_manager->reg_trace(x.second, top.get_path(), x.first);
  }
}

void vp::trace::dump_header()
{
  int max_trace_len = comp->traces.get_trace_manager()->get_max_path_len();
  fprintf(stdout, "%ld: %ld: [\033[34m%-*.*s\033[0m] ", comp->get_clock()->get_time(), comp->get_clock()->get_cycles(), max_trace_len, max_trace_len, name.c_str());
}

void vp::trace::dump_warning_header()
{
  int max_trace_len = comp->traces.get_trace_manager()->get_max_path_len();
  fprintf(stdout, "%ld: %ld: [\033[31m%-*.*s\033[0m] ", comp->get_clock()->get_time(), comp->get_clock()->get_cycles(), max_trace_len, max_trace_len, name.c_str());
}
