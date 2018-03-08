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

class trace_domain : public vp::trace_engine
{

public:

  trace_domain(const char *config);

  void add_paths(int nb_path, const char **paths);
  void add_path(const char *path);
  void reg_trace(vp::trace *trace, string path, string name);

  void build();

  int get_max_path_len() { return max_path_len; }

  int exchange_max_path_len(int max_len) {
    if (max_len > max_path_len) max_path_len = max_len;
    return max_path_len;
  }

private:

  std::vector<regex_t *> path_regex;
  int max_path_len = 0;
};


vp::trace_engine::trace_engine(const char *config)
  : vp::component(config)
{
}

trace_domain::trace_domain(const char *config)
: vp::trace_engine(config)
{

}



void trace_domain::reg_trace(vp::trace *trace, string path, string name)
{
  int len = path.size() + name.size() + 1;
  if (len > max_path_len) max_path_len = len;

  string full_path = path + "/" + name;
  for (auto& x: path_regex) {
    if (regexec(x, full_path.c_str(), 0, NULL, 0) == 0)
    {
      trace->set_active(true);
    }
  }
}

void trace_domain::build()
{
  new_service("trace", static_cast<trace_engine *>(this));
}

void trace_domain::add_path(const char *path)
{
  regex_t *regex = new regex_t();
  path_regex.push_back(regex);
  regcomp(regex, path, 0);
}

void trace_domain::add_paths(int nb_path, const char **paths)
{
  for (int i=0; i<nb_path; i++)
  {
    add_path(paths[i]);
  }
}

extern "C" void vp_trace_add_paths(void *comp, int nb_path, const char **paths)
{
  ((trace_domain *)comp)->add_paths(nb_path, paths);
}

extern "C" int vp_trace_exchange_max_path_len(void *comp, int max_len)
{
  return ((trace_domain *)comp)->exchange_max_path_len(max_len);
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new trace_domain(config);
}
