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
#include "vp/trace/trace_engine.hpp"
#include <string.h>




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

void vp::component_trace::new_trace_event(std::string name, trace *trace, int width)
{
  trace_events[name] = trace;
  trace->width = width;
  trace->bytes = (width + 7) / 8;
  trace->comp = static_cast<vp::component *>(&top);
  trace->name = top.get_path() + "/" + name;
}

void vp::component_trace::new_trace_event_string(std::string name, trace *trace)
{
  trace_events[name] = trace;
  trace->comp = static_cast<vp::component *>(&top);
  trace->name = top.get_path() + "/" + name;
}

void vp::component_trace::post_post_build()
{
  trace_manager = (vp::trace_engine *)top.get_service("trace");
  for (auto& x: traces) {
    x.second->trace_manager = trace_manager;
    trace_manager->reg_trace(x.second, 0, top.get_path(), x.first);
  }
  for (auto& x: trace_events) {
    x.second->trace_manager = trace_manager;
    trace_manager->reg_trace(x.second, 1, top.get_path(), x.first);
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



char *vp::trace_engine::get_event_buffer(int bytes)
{
  if (current_buffer == NULL || bytes > TRACE_EVENT_BUFFER_SIZE - current_buffer_size)
  {
    pthread_mutex_lock(&mutex);

    if (current_buffer && bytes > TRACE_EVENT_BUFFER_SIZE - current_buffer_size)
    {
      *(vp::trace **)(current_buffer + current_buffer_size) = NULL;
      ready_event_buffers.push_back(current_buffer);
      pthread_cond_broadcast(&cond);
    }

    while (event_buffers.size() == 0)
    {
      pthread_cond_wait(&cond, &mutex);
    }
    current_buffer = event_buffers[0];
    event_buffers.erase(event_buffers.begin());
    current_buffer_size = 0;
    pthread_mutex_unlock(&mutex);
  }

  char *result = current_buffer + current_buffer_size;

  current_buffer_size += bytes;

  return result;
}

void vp::trace_engine::stop()
{
  this->flush();
  pthread_mutex_lock(&mutex);
  this->end = 1;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);
  this->thread->join();
}

void vp::trace_engine::flush()
{
  if (current_buffer_size)
  {
    pthread_mutex_lock(&mutex);
    *(vp::trace **)(current_buffer + current_buffer_size) = NULL;
    ready_event_buffers.push_back(current_buffer);
    current_buffer = NULL;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
  }

}

void vp::trace_engine::dump_event(vp::trace *trace, int64_t timestamp, uint8_t *event, int bytes)
{
  int size = bytes + sizeof(trace) + sizeof(timestamp);
  char *event_buffer = this->get_event_buffer(size);
  *(vp::trace **)event_buffer = trace;
  event_buffer += sizeof(trace);
  *(int64_t *)event_buffer = timestamp;
  event_buffer += sizeof(timestamp);
  if (bytes == 4)
    *(uint32_t *)event_buffer = *(uint32_t *)event;
  else
    memcpy((void *)event_buffer, (void *)event, bytes);
}

void vp::trace_engine::vcd_routine()
{
  while(1)
  {
    char *event_buffer, *event_buffer_start;

    pthread_mutex_lock(&this->mutex);
    while(this->ready_event_buffers.size() == 0 && !end)
    {
      pthread_cond_wait(&this->cond, &this->mutex);
    }

    if (this->ready_event_buffers.size() == 0 && end)
    {
      pthread_mutex_unlock(&this->mutex);
      break;
    }

    event_buffer = ready_event_buffers[0];
    event_buffer_start = event_buffer;
    ready_event_buffers.erase(ready_event_buffers.begin());

    pthread_mutex_unlock(&this->mutex);

    int size = 0;
    while (size < TRACE_EVENT_BUFFER_SIZE)
    {
      int64_t timestamp;
      vp::trace *trace = *(vp::trace **)event_buffer;
      if (trace == NULL) break;

      event_buffer += sizeof(trace);
      timestamp = *(int64_t *)event_buffer;
      event_buffer += sizeof(timestamp);
      int bytes = trace->bytes;
      uint8_t event[bytes];
      memcpy((void *)&event, (void *)event_buffer, bytes);
      event_buffer += bytes;

      size += sizeof(trace) + bytes;

      if (trace->vcd_trace)
      {
        trace->vcd_trace->dump(timestamp, event, trace->width);
      }

    }

    pthread_mutex_lock(&this->mutex);
    event_buffers.push_back(event_buffer_start);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&this->mutex);
  }

  vcd_dumper.close();
}
