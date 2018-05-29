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

#ifndef __VP_COMPONENT_HPP__
#define __VP_COMPONENT_HPP__

#include "vp/vp_data.hpp"
#include <map>
#include <list>
#include <string>
#include <vector>
#include <functional>
#include "vp/ports.hpp"
#include "vp/config.hpp"
#include "vp/clock/clock_event.hpp"
#include "vp/itf/clk.hpp"
#include "vp/clock/component_clock.hpp"
#include "vp/trace/component_trace.hpp"
#include "vp/power/component_power.hpp"
#include "json.hpp"


#define   likely(x) __builtin_expect(x, 1)
#define unlikely(x) __builtin_expect(x, 0)

using namespace std;

#define VP_ERROR_SIZE (1<<16)
extern char vp_error[];

namespace vp {

  class config;

  class clock_engine;

  class component : public component_clock
  {

  public:
    component(const char *config);

    virtual int build() { return 0; }
    virtual void pre_start() {}
    virtual void start() {}
    virtual void stop() {}
    virtual void reset() {}
    virtual void load() {}
    virtual string run() { return "error"; }
    virtual int run_status() { return 0; }


    inline config *get_config();

    inline js::config *get_js_config() { return comp_js_config; }

    inline config *get_config(std::string name);


    inline int get_config_int(std::string name, int index);

    inline int get_config_int(std::string name);

    inline bool get_config_bool(std::string name);

    inline std::string get_config_str(std::string name);

    string get_path() { return path; }


    void conf(string path) { this->path = path; }

    config *import_config(const char *config_string);


    void post_post_build();

    void pre_build() {
      component_clock::pre_build(this);
    }

    void new_master_port(std::string name, master_port *port);

    void new_slave_port(std::string name, slave_port *port);

    void new_slave_port(void *comp, std::string name, slave_port *port);

    void new_service(std::string name, void *service);


    int get_ports(bool master, int size, const char *names[], void *ports[]);

    int get_services(int size, const char *names[], void *services[]);

    void *get_service(string name);

    void set_services(int nb_services, const char *name[], void *services[]);


    component_trace traces;
    component_power power;


  protected:
    template<typename P> int get_ports(std::map<std::string, P *> ports_map,
      int size, const char *names[], void *ports[]);

    std::map<std::string, master_port *> master_ports;
    std::map<std::string, slave_port *> slave_ports;
    std::map<std::string, void *> services;
    std::map<std::string, void *> all_services;
 
  private:
    inline config *get_config(std::string name, int index);

    config *comp_config;
    js::config *comp_js_config;

    string path;

    component *parent = NULL;

  };

};  

#endif
