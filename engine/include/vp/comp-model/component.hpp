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

#ifndef __VP_COMP_MODEL_COMPONENT_HPP__
#define __VP_COMP_MODEL_COMPONENT_HPP__

#include <string>
#include <map>

#include "vp/comp-model/config.hpp"

#define   likely(x) __builtin_expect(x, 1)
#define unlikely(x) __builtin_expect(x, 0)

namespace cm {

  class master_port;
  class slave_port;
  class config;

  class component
  {

  public:
    component(const char *config);

    virtual void pre_build() {}
    virtual void build() {}
    virtual void post_post_build() {}
    virtual void pre_start() {}
    virtual void start() {}
    virtual void stop() {}
    virtual void reset() {}
    virtual void load() {}
    virtual string run() { return "error"; }
    virtual int run_status() { return 0; }

    void new_master_port(std::string name, master_port *port);

    void new_slave_port(std::string name, slave_port *port);

    void new_slave_port(void *comp, std::string name, slave_port *port);

    void new_service(std::string name, void *service);


    inline config *get_config();

    inline config *get_config(std::string name);


    inline int get_config_int(std::string name, int index);

    inline int get_config_int(std::string name);

    inline bool get_config_bool(std::string name);

    inline std::string get_config_str(std::string name);

    string get_path() { return path; }


    int get_ports(bool master, int size, const char *names[], void *ports[]);

    int get_services(int size, const char *names[], void *services[]);

    void *get_service(string name);

    void set_services(int nb_services, const char *name[], void *services[]);


    void conf(string path) { this->path = path; }

    config *import_config(const char *config_string);


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

    string path;

    component *parent = NULL;

  };

};

#endif
