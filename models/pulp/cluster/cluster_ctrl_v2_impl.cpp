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
#include <vp/itf/wire.hpp>
#include <stdio.h>
#include <string.h>

#include "archi/cluster_ctrl/cluster_ctrl_v2.h"

class Core_cluster_ctrl
{
public:
  vp::wire_master<uint32_t> bootaddr_itf;
  vp::wire_master<bool> fetchen_itf;
};

class cluster_ctrl : public vp::component
{

public:

  cluster_ctrl(const char *config);

  void build();
  void start();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

private:


  vp::io_req_status_e fetch_en_req(bool is_write, uint32_t *data);
  vp::io_req_status_e bootaddr_req(int core, bool is_write, uint32_t *data);

  vp::trace     trace;
  vp::io_slave in;
  int nb_core;
  Core_cluster_ctrl *cores;
};

cluster_ctrl::cluster_ctrl(const char *config)
: vp::component(config)
{
  nb_core = get_config_int("nb_core");
}

vp::io_req_status_e cluster_ctrl::req(void *__this, vp::io_req *req)
{
  cluster_ctrl *_this = (cluster_ctrl *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("cluster_ctrl access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  if (size != 4)
  {
    _this->trace.warning("Only 32 bits accesses are allowed\n");
    return vp::IO_REQ_INVALID;
  } 

  if (offset == ARCHI_CLUSTER_CTRL_FETCH_EN)
  {
    return _this->fetch_en_req(is_write, (uint32_t *)data);
  }
  else if (offset >= ARCHI_CLUSTER_CTRL_BOOTADDR(0) && offset < ARCHI_CLUSTER_CTRL_BOOTADDR(_this->nb_core))
  {
    return _this->bootaddr_req(ARCHI_CLUSTER_CTRL_BOOTADDR_COREID(offset), is_write, (uint32_t *)data);
  }

  _this->trace.warning("Invalid access\n");

  return vp::IO_REQ_INVALID;
}

vp::io_req_status_e cluster_ctrl::fetch_en_req(bool is_write, uint32_t *data)
{
  for (int i=0; i<nb_core; i++)
  {
    cores[i].fetchen_itf.sync(((*data) >> i) & 1);
  }
  return vp::IO_REQ_OK;
}

vp::io_req_status_e cluster_ctrl::bootaddr_req(int core, bool is_write, uint32_t *data)
{
  cores[core].bootaddr_itf.sync(*data);
  return vp::IO_REQ_OK;
}

void cluster_ctrl::build()
{
  cores = (Core_cluster_ctrl *)new Core_cluster_ctrl[nb_core];

  traces.new_trace("trace", &trace, vp::DEBUG);

  in.set_req_meth(&cluster_ctrl::req);
  new_slave_port("in", &in);

  for (int i; i<nb_core; i++)
  {
    new_master_port("bootaddr_" + std::to_string(i), &cores[i].bootaddr_itf);
    new_master_port("fetchen_" + std::to_string(i), &cores[i].fetchen_itf);
  }
}

void cluster_ctrl::start()
{
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new cluster_ctrl(config);
}
