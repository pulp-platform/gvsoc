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
#include "vp/trace/vcd_dumper.hpp"
#include <string.h>

static int vcd_id = 0;

vp::Vcd_file::Vcd_file(vp::Vcd_dumper *dumper, string path)
{
  file = fopen(path.c_str(), "w");
  if (file == NULL)
  {
    dumper->comp->get_engine()->fatal("Error while opening VCD file (path: %s, error: %s)\n", path.c_str(), strerror(errno));
  }

  fprintf(file, "%s", "\n\
    $timescale 1ps $end\n\
    ");
}


string vp::Vcd_file::parse_path(string path, bool begin)
{
  std::string delim = "/";
  
  auto start = 0U;
  auto end = path.find(delim);
  while (end != std::string::npos)
  {
    if (begin) {
      fprintf(file, "$scope module %s $end\n", path.substr(start, end - start).c_str());
    } else {
      fprintf(file, "$upscope $end\n");
    }
    start = end + delim.length();
    end = path.find(delim, start);
  }
  return path.substr(start, end);
}

void vp::Vcd_file::add_trace(string path, int id, int width, bool is_real)
{
  string name = parse_path(path, true);
  
  if (is_real)
  	fprintf(file, "$var real 64 %d %s $end\n", id, name.c_str());
  else
  	fprintf(file, "$var wire %d %d %s $end\n", width, id, name.c_str());

  parse_path(string(path), false);

  fprintf(file, "$enddefinitions $end\n");
  
}

vp::Vcd_trace::Vcd_trace(string trace_name, Vcd_file *file, int width, bool is_real) : is_real(is_real), file(file)
{
  id = vcd_id++;
  file->add_trace(trace_name, id, width, is_real);
}



void vp::Vcd_file::close()
{
  fclose(file);
}

static unsigned int get_bit(uint8_t *value, int i) {
  return (value[i/8] >> (i%8)) & 1;
}

void vp::Vcd_file::dump(int64_t timestamp, int id, uint8_t *event, int width, bool is_real)
{
  if (!header_dumped)
  {
    header_dumped = true;
    fprintf(file, "%s", "\n\
      $enddefinitions $end\n\
      $dumpvars\n");    
    fprintf(file, "$end\n");
  }

  if (last_timestamp != timestamp) {
    last_timestamp = timestamp;
    fprintf(file, "#%ld\n", timestamp);
  }

  if (is_real)
  {
    fprintf(file, "r%f %d\n", *(double *)event, id);
  }
  else if (width > 1) {
    int i;
    char str[width+1+1];
    char *str_ptr = str;
    *str_ptr++ = 'b';

    if (event) {
      for (int i=width-1; i>=0; i--)
      {
        *str_ptr++ = get_bit(event, i) + '0';
      }
      *str_ptr = 0;
    }
    else
    {
      for (i=0; i<width; i++)
        str[i+1] = 'x';
    }

    fprintf(file, "%s %d\n", str, id);
  }
  else
  {
    if (event) {
      fprintf(file, "%u%d\n", get_bit(event, 0), id);
    } else {
      fprintf(file, "x%d\n", id);
    }
  }
}



vp::Vcd_trace *vp::Vcd_dumper::get_trace(string trace_name, string file_name, int width, bool is_real)
{
  vp::Vcd_trace *trace = vcd_traces[trace_name];
  if (trace == NULL)
  {
    vp::Vcd_file *vcd_file = vcd_files[file_name];

    if (vcd_file == NULL)
    {
      vcd_file = new Vcd_file(this, file_name);
      vcd_files[file_name] = vcd_file;
    }

    trace = new Vcd_trace(trace_name, vcd_file, width, is_real);
  }

  return trace;
}

vp::Vcd_trace *vp::Vcd_dumper::get_trace_real(string trace_name, string file_name)
{
  vp::Vcd_trace *trace = this->get_trace(trace_name, file_name, 8, true);
  return trace;
}

void vp::Vcd_dumper::close()
{
  for (auto &x: vcd_files)
  {
    x.second->close();
  }
}