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
#include <vp/itf/io.hpp>
#include <stdio.h>
#include <string.h>

class memory : public vp::component
{

public:

  memory(const char *config);

  void build();
  void start();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

private:

  vp::trace     trace;
  vp::io_slave in;

  uint64_t size = 0;
  bool check = false;

  uint8_t *mem_data;
  uint8_t *check_mem;
};

memory::memory(const char *config)
: vp::component(config)
{

}

vp::io_req_status_e memory::req(void *__this, vp::io_req *req)
{
  memory *_this = (memory *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();

  _this->trace.msg("Memory access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, req->get_is_write());

  if (offset + size > _this->size) {
    //gv_trace_dumpWarning(&warning, "Received out-of-bound request (reqAddr: 0x%x, reqSize: 0x%x, memSize: 0x%x)\n", offset, size, this->size);
    return vp::IO_REQ_INVALID;
  }

  if (req->get_is_write()) {
    if (_this->check_mem) {
      for (unsigned int i=0; i<size; i++) {
        _this->check_mem[(offset + i) / 8] |= 1 << ((offset + i) % 8);
      }
    }
    memcpy((void *)&_this->mem_data[offset], (void *)data, size);
  } else {
    if (_this->check_mem) {
      for (unsigned int i=0; i<size; i++) {
        int access = (_this->check_mem[(offset + i) / 8] >> ((offset + i) % 8)) & 1;
        if (!access) {
          //trace.msg("Unitialized access (offset: 0x%x, size: 0x%x, isRead: %d)\n", offset, size, isRead);
          return vp::IO_REQ_INVALID;
        }
      }
    }
    memcpy((void *)data, (void *)&_this->mem_data[offset], size);
  }

  return vp::IO_REQ_OK;
}

void memory::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);
  in.set_req_meth(&memory::req);
  new_slave_port("input", &in);
}

void memory::start()
{
  size = get_config_int("size");
  check = get_config_bool("check");

  trace.msg("Building memory (size: 0x%x, check: %d)\n", size, check);

  mem_data = new uint8_t[size];

  if (check)
  {
    check_mem = new uint8_t[(size + 7)/8];
  }
  else
  {
    check_mem = NULL;
  }

  memset(mem_data, 0x57, size);
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new memory(config);
}
