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

#define NB_MASTER_PORTS 4

class hwce : public vp::component
{

public:

  hwce(const char *config);

  int build();
  void start();
  void reset(bool active);

private:

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

  static void job_queue_handle(void *__this, vp::clock_event *event);
  static void ctrl_handle(void *__this, vp::clock_event *event);

  vp::trace     trace;

  vp::io_slave          in;
  vp::io_master         out[NB_MASTER_PORTS];
  vp::wire_master<bool> irq;

  vp::clock_event *job_queue_event;
  vp::clock_event *ctrl_event;
  
};

hwce::hwce(const char *config)
: vp::component(config)
{

}

void hwce::reset(bool active)
{
  if (active)
  {
  }
}



vp::io_req_status_e hwce::req(void *__this, vp::io_req *req)
{
  hwce *_this = (hwce *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();

  _this->trace.msg("HWCE access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, req->get_is_write());

  return vp::IO_REQ_OK;
}



void hwce::job_queue_handle(void *__this, vp::clock_event *event)
{

}



void hwce::ctrl_handle(void *__this, vp::clock_event *event)
{

}



int hwce::build()
{
  this->traces.new_trace("trace", &this->trace, vp::DEBUG);

  this->in.set_req_meth(&hwce::req);
  this->new_slave_port("input", &this->in);

  for (int i=0; i<NB_MASTER_PORTS; i++)
  {
    this->new_master_port("out" + std::to_string(i), &this->out[i]);
  }

  this->new_master_port("irq", &this->irq);

  this->ctrl_event = this->event_new(&hwce::ctrl_handle);
  this->job_queue_event = this->event_new(&hwce::job_queue_handle);

  return 0;
}

void hwce::start()
{
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new hwce(config);
}
