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

#ifndef __VP_TRACE_VCD_DUMPER_HPP__
#define __VP_TRACE_VCD_DUMPER_HPP__

#include <stdio.h>

namespace vp {

  class Vcd_dumper;

  class Vcd_file 
  {
  public:
    Vcd_file(Vcd_dumper *dumper, string path);
    void dump(int64_t timestamp, int id, uint8_t *event, int width, bool is_real);
    void close();
    void add_trace(string name, int id, int width, bool is_real);

  private:
    string parse_path(string path, bool begin);

    int64_t last_timestamp = -1;
    FILE *file;
    bool header_dumped = false;
  };

  class Vcd_trace
  {
  public:
    Vcd_trace(string trace_name, Vcd_file *file, int width, bool is_real);
    void reg(int64_t timestamp, uint8_t *event, int width);
    inline void dump(int64_t timestamp) { file->dump(timestamp, id, this->buffer, this->width, this->is_real); }
    bool is_real = false;
    Vcd_trace *next;
    bool is_enqueued;

  private:
    Vcd_file *file;
    int id;
    uint8_t *buffer;
    int width;

  };

  class Vcd_dumper
  {
  public:
    Vcd_dumper(vp::component *comp) : comp(comp) {}
    Vcd_trace *get_trace(string trace_name, string file_name, int width, bool is_real=false);
    Vcd_trace *get_trace_real(string trace_name, string file_name);
    void close();

    vp::component *comp;

  private:
    std::map<std::string, Vcd_trace *> vcd_traces;
    std::map<std::string, Vcd_file *> vcd_files;
  };

};

#endif
