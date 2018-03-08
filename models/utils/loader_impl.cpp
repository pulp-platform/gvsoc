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

class loader : public vp::component
{

public:

  loader(const char *config);

  void build();
  void start();

  void io_req(uint64_t addr, uint64_t size, bool is_write, uint8_t *data);
  void memset(uint64_t addr, uint64_t size, uint8_t value);

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

  static void grant(void *_this, vp::io_req *req);
  static void response(void *_this, vp::io_req *req);

private:

  vp::trace     trace;
  vp::io_master out;
  vp::io_req    io_request;
};

loader::loader(const char *config)
: vp::component(config)
{

}

void loader::grant(void *_this, vp::io_req *req)
{
}

void loader::response(void *_this, vp::io_req *req)
{
}

void loader::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);

  out.set_resp_meth(&loader::response);
  out.set_grant_meth(&loader::grant);
  new_master_port("out", &out);

}

void loader::start()
{
}

void loader::io_req(uint64_t addr, uint64_t size, bool is_write, uint8_t *data)
{
  trace.msg("Loading section (base: 0x%x, size: 0x%x, is_write: %d)\n", addr, size, is_write);

  vp::io_req *req = &io_request;
  req->set_addr(addr);
  req->set_size(size);
  req->set_is_write(is_write);
  req->set_data(data);
  out.req(req);
}

void loader::memset(uint64_t addr, uint64_t size, uint8_t value)
{
  trace.msg("Padding section (base: 0x%x, size: 0x%x, value: %d)\n", addr, size, value);

  uint8_t data[size];
  ::memset((void *)data, value, size);
  vp::io_req *req = &io_request;
  req->set_addr(addr);
  req->set_size(size);
  req->set_is_write(true);
  req->set_data(data);
  out.req(req);
}

extern "C" void loader_io_req(void *__this, uint64_t addr, uint64_t size, bool is_write, uint8_t *data)
{
  loader *_this = (loader *)__this;
  _this->io_req(addr, size, is_write, data);
}

extern "C" void loader_memset(void *__this, uint64_t addr, uint64_t size, uint8_t value)
{
  loader *_this = (loader *)__this;
  _this->memset(addr, size, value);
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new loader(config);
}
