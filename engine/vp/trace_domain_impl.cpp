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

#include <vp/vp.hpp>
#include <vp/itf/clk.hpp>
#include <vp/trace/trace_engine.hpp>
#include <regex.h>
#include <vector>
#include <thread>
#include <string.h>

class trace_domain : public vp::trace_engine
{

public:

  trace_domain(const char *config);

  void add_paths(int events, int nb_path, const char **paths);
  void add_path(int events, const char *path);
  void reg_trace(vp::trace *trace, int event, string path, string name);

  int build();

  int get_max_path_len() { return max_path_len; }

  int exchange_max_path_len(int max_len) {
    if (max_len > max_path_len) max_path_len = max_len;
    return max_path_len;
  }

private:

  std::vector<regex_t *> path_regex;
  std::vector<regex_t *> events_path_regex;
  std::vector<string> events_file;
  int max_path_len = 0;
};


vp::trace_engine::trace_engine(const char *config)
  : vcd_dumper(this), vp::component(config)
{

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);

  for (int i=0; i<TRACE_EVENT_NB_BUFFER; i++)
  {
    event_buffers.push_back(new char[TRACE_EVENT_BUFFER_SIZE]);
  }
  current_buffer = event_buffers[0];
  event_buffers.erase(event_buffers.begin());
  current_buffer_size = 0;

  thread = new std::thread(&trace_engine::vcd_routine, this);
}

trace_domain::trace_domain(const char *config)
: vp::trace_engine(config)
{

}



void trace_domain::reg_trace(vp::trace *trace, int event, string path, string name)
{
  int len = path.size() + name.size() + 1;
  if (len > max_path_len) max_path_len = len;

  string full_path = path + "/" + name;
  int index = 0;
  for (auto& x: event ? events_path_regex : path_regex) {
    if (regexec(x, full_path.c_str(), 0, NULL, 0) == 0)
    {
      if (event)
      {
        vp::Vcd_trace *vcd_trace = vcd_dumper.get_trace(full_path, this->events_file[index], trace->width);
        trace->set_event_active(true);
        trace->vcd_trace = vcd_trace;
      }
      else
        trace->set_active(true);
    }
    index++;
  }
}

int trace_domain::build()
{
  new_service("trace", static_cast<trace_engine *>(this));
  return 0;
}

void trace_domain::add_path(int events, const char *path)
{
  regex_t *regex = new regex_t();
  if (events)
  {
    const char *file_path = "all.vcd";
    char *delim = (char *)::index(path, '@');
    if (delim)
    {
      *delim = 0;
      file_path = delim + 1;
    }
    events_path_regex.push_back(regex);
    events_file.push_back((char *)file_path);
  }
  else
    path_regex.push_back(regex);
  regcomp(regex, path, 0);
}

void trace_domain::add_paths(int events, int nb_path, const char **paths)
{
  for (int i=0; i<nb_path; i++)
  {
    add_path(events, paths[i]);
  }
}

extern "C" void vp_trace_add_paths(void *comp, int events, int nb_path, const char **paths)
{
  ((trace_domain *)comp)->add_paths(events, nb_path, paths);
}

extern "C" int vp_trace_exchange_max_path_len(void *comp, int max_len)
{
  return ((trace_domain *)comp)->exchange_max_path_len(max_len);
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new trace_domain(config);
}
