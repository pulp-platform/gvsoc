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
#include <vp/itf/qspim.hpp>
#include <vp/itf/jtag.hpp>
#include <stdio.h>
#include <string.h>
#include <vector>

using namespace std;

class Pad_group
{

};

class Qspim_group : public Pad_group
{
public:
  vp::qspim_slave slave;
  vp::qspim_master master;
};

class Jtag_group : public Pad_group
{
public:
  vp::jtag_slave slave;
  vp::jtag_master master;
};

class padframe : public vp::component
{

public:

  padframe(const char *config);

  void build();
  void start();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

private:

  static void sync(void *__this, int sck, int data_0, int data_1, int data_2, int data_3, int id);
  static void sync_cycle(void *__this, int data_0, int data_1, int data_2, int data_3, int id);
  static void cs_sync(void *__this, int cs, int active, int id);

  static void jtag_sync(void *__this, int tck, int tdi, int tms, int trst, int id);
  static void jtag_sync_cycle(void *__this, int tdi, int tms, int trst, int id);
  static void jtag_master_sync(void *__this, int tdo, int id);

  vp::trace     trace;
  vp::io_slave in;

  vector<Pad_group *> groups;

  int nb_itf = 0;
};

padframe::padframe(const char *config)
: vp::component(config)
{

}

void padframe::sync(void *__this, int sck, int data_0, int data_1, int data_2, int data_3, int id)
{

  printf("%s %d\n", __FILE__, __LINE__);
}

void padframe::sync_cycle(void *__this, int data_0, int data_1, int data_2, int data_3, int id)
{
  printf("%s %d\n", __FILE__, __LINE__);
}

void padframe::cs_sync(void *__this, int cs, int active, int id)
{
  printf("%s %d\n", __FILE__, __LINE__);
}



void padframe::jtag_sync(void *__this, int tck, int tdi, int tms, int trst, int id)
{
  padframe *_this = (padframe *)__this;
  Jtag_group *group = static_cast<Jtag_group *>(_this->groups[id]);
  group->master.sync(tck, tdi, tms, trst);
}


void padframe::jtag_master_sync(void *__this, int tdo, int id)
{
  padframe *_this = (padframe *)__this;
  Jtag_group *group = static_cast<Jtag_group *>(_this->groups[id]);
  group->slave.sync(tdo);
}


void padframe::jtag_sync_cycle(void *__this, int tdi, int tms, int trst, int id)
{
  padframe *_this = (padframe *)__this;
  Jtag_group *group = static_cast<Jtag_group *>(_this->groups[id]);
  group->master.sync_cycle(tdi, tms, trst);
}



vp::io_req_status_e padframe::req(void *__this, vp::io_req *req)
{
  padframe *_this = (padframe *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();

  _this->trace.msg("IO access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, req->get_is_write());

  return vp::IO_REQ_OK;
}

void padframe::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);
  in.set_req_meth(&padframe::req);

  new_slave_port("in", &in);



  cm::config *groups = get_config()->get("groups");

  for (auto& group: groups->get_childs())
  {
    std::string name = group.first;
    cm::config *config = group.second;
    std::string type = config->get("type")->get_str();

    trace.msg("Found pad group (group: %s, type: %s)\n",
      name.c_str(), type.c_str());

    if (type == "qspim")
    {
      Qspim_group *group = new Qspim_group();
      new_master_port(name + "_pad", &group->master);
      new_slave_port(name, &group->slave);
      group->slave.set_sync_meth_muxed(&padframe::sync, nb_itf);
      group->slave.set_sync_cycle_meth_muxed(&padframe::sync_cycle, nb_itf);
      group->slave.set_cs_sync_meth_muxed(&padframe::cs_sync, nb_itf);
      this->groups.push_back(group);
      nb_itf++;
    }
    else if (type == "jtag")
    {
      Jtag_group *group = new Jtag_group();
      new_master_port(name, &group->master);
      new_slave_port(name + "_pad", &group->slave);
      group->master.set_sync_meth_muxed(&padframe::jtag_master_sync, nb_itf);
      group->slave.set_sync_meth_muxed(&padframe::jtag_sync, nb_itf);
      group->slave.set_sync_cycle_meth_muxed(&padframe::jtag_sync_cycle, nb_itf);
      this->groups.push_back(group);
      nb_itf++;
    }
    else
    {
      trace.warning("Unknown pad group type (group: %s, type: %s)\n",
        name.c_str(), type.c_str());
    }
  }
}

void padframe::start()
{
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new padframe(config);
}
