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

#include <vp/comp-model/cm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>

#include "string.h"

std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

cm::config *cm::config::create_config(jsmntok_t *tokens, int *_size)
{
  jsmntok_t *current = tokens;
  config *config = NULL;

  switch (current->type)
  {
    case JSMN_PRIMITIVE:
      if (strcmp(current->str, "True") == 0 || strcmp(current->str, "False") == 0 || strcmp(current->str, "true") == 0 || strcmp(current->str, "false") == 0)
      {
        config = new config_bool(current);
      }
      else
      {
        config = new config_number(current);
      }
      current++;
      break;

    case JSMN_OBJECT: {
      int size;
      config = new config_object(current, &size);
      current += size;
      break;
    }

    case JSMN_ARRAY: {
      int size;
      config = new config_array(current, &size);
      current += size;
      break;
    }

    case JSMN_STRING:
      config = new config_string(current);
      current++;
      break;

    case JSMN_UNDEFINED:
      break;
  }

  if (_size) {
    *_size = current - tokens;
  }

  return config;
}

cm::config *cm::config_string::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;
  return NULL;
}

cm::config *cm::config_number::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;
  return NULL;
}

cm::config *cm::config_bool::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;
  return NULL;
}

cm::config *cm::config_array::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;
  return NULL;
}

cm::config *cm::config_object::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;

  cm::config *result = NULL;
  std::string name;
  int name_pos = 0;

  for (auto& x: name_list) {
    if (x != "*" && x != "**")
    {
      name = x;
      break;
    }
    name_pos++;
  }

  for (auto& x: childs) {

    if (name == x.first)
    {
      result = x.second->get_from_list(std::vector<std::string>(name_list.begin () + name_pos + 1, name_list.begin () + name_list.size()));
      if (name_pos == 0 || result != NULL) return result;

    }
    else if (name_list[0] == "*")
    {
      result = x.second->get_from_list(std::vector<std::string>(name_list.begin () + 1, name_list.begin () + name_list.size()));
      if (result != NULL) return result;
    }
    else if (name_list[0] == "**")
    {
      result = x.second->get_from_list(name_list);
      if (result != NULL) return result;
    }
  }

  return result;
}

cm::config *cm::config_object::get(std::string name)
{
  return get_from_list(split(name, '/'));
}

cm::config_string::config_string(jsmntok_t *tokens)
{
  value = tokens->str;
}

cm::config_number::config_number(jsmntok_t *tokens)
{
  value = atof(tokens->str);
}

cm::config_bool::config_bool(jsmntok_t *tokens)
{
  value = strcmp(tokens->str, "True") == 0 || strcmp(tokens->str, "true") == 0;
}

cm::config_array::config_array(jsmntok_t *tokens, int *_size)
{
  jsmntok_t *current = tokens;
  jsmntok_t *top = current++;
  
  for (int i=0; i<top->size; i++)
  {
    int child_size;
    elems.push_back(create_config(current, &child_size));
    current += child_size;
  }


  if (_size) {
    *_size = current - tokens;
  }
}

cm::config_object::config_object(jsmntok_t *tokens, int *_size)
{
  jsmntok_t *current = tokens;
  jsmntok_t *t = current++;

  for (int i=0; i<t->size; i++)
  {
    jsmntok_t *child_name = current++;
    int child_size;
    config *child_config = create_config(current, &child_size);
    current += child_size;

    if (child_config != NULL)
    {
      childs[child_name->str] = child_config;

    }
  }

  if (_size) {
    *_size = current - tokens;
  }
}

cm::config *cm::component::import_config(const char *config_string)
{
  if (config_string == NULL) return NULL;

  jsmn_parser parser;
  
  jsmn_init(&parser);
  int nb_tokens = jsmn_parse(&parser, config_string, strlen(config_string), NULL, 0);

  jsmntok_t tokens[nb_tokens];

  jsmn_init(&parser);
  nb_tokens = jsmn_parse(&parser, config_string, strlen(config_string), tokens, nb_tokens);

  char *str = strdup(config_string);
  for (int i=0; i<nb_tokens; i++)
  {
    jsmntok_t *tok = &tokens[i];
    tok->str = &str[tok->start];
    str[tok->end] = 0;
    //printf("%d %d %d %d: %s\n", tokens[i].type, tokens[i].start, tokens[i].end, tokens[i].size, tok->str);
  }



  return new cm::config_object(tokens);
}



cm::component::component(const char *config_string)
{
  comp_config = import_config(config_string);
}


void cm::component::new_master_port(std::string name, cm::master_port *port)
{
  port->set_comp(this);
  master_ports[name] = port;
}

void cm::component::new_slave_port(std::string name, cm::slave_port *port)
{
  port->set_comp(this);
  slave_ports[name] = port;
}

void cm::component::new_slave_port(void *comp, std::string name, cm::slave_port *port)
{
  port->set_comp((cm::component *)comp);
  slave_ports[name] = port;
}

void cm::component::new_service(std::string name, void *service)
{
  services[name] = service;
}


template<typename P> int cm::component::get_ports(std::map<std::string, P *> ports_map,
  int size, const char *names[], void *ports[])
{
  if (size != 0)
  {   
    int i = 0;    
    for (auto& x: ports_map) {
      names[i] = strdup(x.first.c_str());
      ports[i] = x.second;
      i++;
    }
  }
  return ports_map.size();
}

int cm::component::get_ports(bool master, int size, const char *names[], void *ports[])
{
  if (master)
  {
    return get_ports<cm::master_port>(master_ports, size, names, ports);
  }
  else
  {
    return get_ports<cm::slave_port>(slave_ports, size, names, ports);
  }
}

void *cm::component::get_service(string name)
{
  return all_services[name];
}

int cm::component::get_services(int size, const char *names[], void *res_services[])
{
  if (size != 0)
  {   
    int i = 0;    
    for (auto& x: services) {
      names[i] = x.first.c_str();
      res_services[i] = x.second;
      i++;
    }
  }
  return services.size();
}

void cm::component::set_services(int nb_services, const char *names[], void *services[])
{
  for (int i=0; i<nb_services; i++)
  {
    all_services[names[i]] = services[i];
  }
}

extern "C" void vp_comp_conf(void *comp, const char *path)
{
  ((cm::component *)comp)->conf(path);
}

extern "C" void vp_pre_start(void *comp)
{
  ((cm::component *)comp)->pre_start();
}

extern "C" void vp_load(void *comp)
{
  ((cm::component *)comp)->load();
}

extern "C" void vp_start(void *comp)
{
  ((cm::component *)comp)->start();
}

extern "C" void vp_reset(void *comp)
{
  ((cm::component *)comp)->reset();
}

extern "C" void vp_stop(void *comp)
{
  ((cm::component *)comp)->stop();
}

extern "C" const char *vp_run(void *comp)
{
  return ((cm::component *)comp)->run().c_str();
}

extern "C" int vp_run_status(void *comp)
{
  return ((cm::component *)comp)->run_status();
}

extern "C" void vp_post_post_build(void *comp)
{
  ((cm::component *)comp)->post_post_build();
}

extern "C" void vp_build(void *comp)
{
  ((cm::component *)comp)->pre_build();
  ((cm::component *)comp)->build();
}

extern "C" int vp_comp_get_ports(void *comp, bool master, int size, const char *names[], void *ports[])
{
  return ((cm::component *)comp)->get_ports(master, size, names, ports);
}

extern "C" int vp_comp_get_services(void *comp, int size, const char *names[], void *services[])
{
  return ((cm::component *)comp)->get_services(size, names, services);
}

extern "C" void vp_comp_set_services(void *comp, int nb_services, const char *names[], void *services[])
{
  ((cm::component *)comp)->set_services(nb_services, names, services);
}

extern "C" void vp_port_bind_to(void *_master, void *_slave, const char *config_str)
{
  cm::master_port *master = (cm::master_port *)_master;
  cm::slave_port *slave = (cm::slave_port *)_slave;

  cm::config *config = NULL;
  if (config_str != NULL)
  {
    config = master->get_comp()->import_config(config_str);
  }

  master->bind_to(slave, config);
  slave->bind_to(master, config);
}
