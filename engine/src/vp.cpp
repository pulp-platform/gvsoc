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

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <vp/vp.hpp>
#include <stdio.h>
#include "string.h"
#include <iostream>
#include <sstream>
#include <string>


char vp_error[VP_ERROR_SIZE];

vp::component::component(const char *config_string) : traces(*this), power(*this)
{
  comp_config = import_config(strdup(config_string));

  comp_js_config = js::import_config_from_string(strdup(config_string));
}


void vp::component::post_post_build()
{
  traces.post_post_build();
}



void vp::component_clock::clk_reg(component *_this, component *clock)
{
  _this->clock = (clock_engine *)clock;
}

void vp::component_clock::reset_sync(void *_this, bool active)
{
  printf("RESET SYNC\n");
}


void vp::component_clock::pre_build(component *comp) {
  clock_port.set_reg_meth((vp::clk_reg_meth_t *)&component_clock::clk_reg);
  comp->new_slave_port("clock", &clock_port);

  reset_port.set_sync_meth(&component_clock::reset_sync);
  comp->new_slave_port("reset", &reset_port);
}

void vp::time_engine::enqueue(time_engine_client *client, int64_t time)
{
  if (client->is_enqueued) return;

  client->is_enqueued = true;

  time_engine_client *current = first_client, *prev = NULL;
  client->next_event_time = get_time() + time;
  while (current && current->next_event_time < client->next_event_time)
  {
    prev = current;
    current = current->next;
  }
  if (prev) prev->next = client;
  else first_client = client;
  client->next = current;
}

void vp::clock_engine::update()
{
  int64_t diff = this->get_time() - this->stop_time;

  if (diff > 0)
  {
    int64_t cycles = (diff + this->period - 1) / this->period;
    this->stop_time += cycles * this->period;
    this->cycles += cycles;
  }
}

vp::clock_event *vp::clock_engine::enqueue_other(vp::clock_event *event, int64_t cycle)
{
  // Slow case where the engine is not running and we have to first
  // enqueue it to the global time engine.
  enqueue_to_engine(cycle*period);

  if (cycle < CLOCK_EVENT_QUEUE_SIZE)
  {
    enqueue_to_cycle(event, cycle);
  }
  else
  {
    vp::clock_event *current = delayed_queue, *prev = NULL;
    while (current && current->cycle < cycle)
    {
      prev = current;
      current = current->next;
    }
    if (prev) prev->next = event;
    else delayed_queue = event;
    event->next = current;
    event->cycle = cycle + get_cycles();
  }
  return event;
}

void vp::clock_engine::cancel(vp::clock_event *event)
{
  uint64_t cycle_diff = event->cycle - get_cycles();
  if (cycle_diff >= CLOCK_EVENT_QUEUE_SIZE)
  {
    vp::clock_event *current = delayed_queue, *prev = NULL;
    while (current != event)
    {
      prev = current;
      current = current->next;
    }
    if (prev)
      prev->next = event->next;
    else
      delayed_queue = event->next;
  }
  else
  {
    int cycle = (current_cycle + cycle_diff) & CLOCK_EVENT_QUEUE_MASK;
    vp::clock_event *current = event_queue[cycle], *prev = NULL;
    while (current != event)
    {
      prev = current;
      current = current->next;
    }
    if (prev)
      prev->next = event->next;
    else
      event_queue[cycle] = event->next;
  }
  event->enqueued = false;
}

int64_t vp::clock_engine::exec()
{
  // The clock engine has a circular buffer of events to be executed.
  // Events longer than the buffer as put temporarly in a queue.
  // Everytime we start again at the beginning of the buffer, we need
  // to check if events must be enqueued from the queue to the buffer
  // in case they fit the window.
  if (unlikely(current_cycle == 0))
  {
    clock_event *event = delayed_queue;
    while (event)
    {
      if (nb_enqueued_to_cycle == 0) cycles = event->cycle;

      uint64_t cycle_diff = event->cycle - get_cycles();
      if (cycle_diff >= CLOCK_EVENT_QUEUE_SIZE) break;

      clock_event *next = event->next;

      enqueue_to_cycle(event, cycle_diff);

      event = next;
      delayed_queue = event;
    }
  }

  // Now take all events available at the current cycle and execute them all without returning
  // to the main engine to execute them faster. 
  clock_event *current = event_queue[current_cycle];

  //printf("[%ld] %p ENGINE EXEC CYCLE %d first %p %d\n", get_time(), this, current_cycle, current, nb_enqueued_to_cycle);

  while (likely(current != NULL))
  {
    //printf("HANDLING EVENT %p meth %p\n", current, current->meth);
    clock_event *next = current->next;
    current->enqueued = false;
    nb_enqueued_to_cycle--;

    current->meth(current->_this, current);
    current = next;
  }
  event_queue[current_cycle] = NULL;

  // Now we need to tell the time engine when is the next event.
  // The most likely is that there is an event in the circular buffer, 
  // in which case we just return the clock period, as we will go through
  // each element of the circular buffer, even if the next event if further in
  // the buffer.
  if (likely(nb_enqueued_to_cycle))
  {
    cycles++;
    current_cycle = (current_cycle + 1) & CLOCK_EVENT_QUEUE_MASK;
    return period;
  }
  else
  {
    // Otherwise if there is an event in the delayed queue, return the time
    // to this event.
    // In both cases, reset the current cycle so that the next event to be
    // executed is moved to the circular buffer.
    current_cycle = 0;

    // Also remember the current time in order to resynchronize the clock engine
    // in case we enqueue and event from another engine.
    this->stop_time = this->get_time();

    if (delayed_queue)
    {
      return (delayed_queue->cycle - get_cycles()) * period;
    }
    else
    {
      // In case there is no more event to execute, returns -1 to tell the time
      // engine we are done.
      return -1;
    }
  }
}


vp::clock_event::clock_event(component_clock *comp, clock_event_meth_t *meth) 
: comp(comp), _this((void *)static_cast<vp::component *>((vp::component_clock *)(comp))), meth(meth), enqueued(false)
{

}

void vp::component::new_master_port(std::string name, vp::master_port *port)
{
  port->set_owner(this);
  port->set_context(this);
  master_ports[name] = port;
}

void vp::component::new_slave_port(std::string name, vp::slave_port *port)
{
  port->set_owner(this);
  port->set_context(this);
  slave_ports[name] = port;
}

void vp::component::new_slave_port(void *comp, std::string name, vp::slave_port *port)
{
  port->set_owner(this);
  port->set_context(comp);
  slave_ports[name] = port;
}

void vp::component::new_service(std::string name, void *service)
{
  services[name] = service;
}


template<typename P> int vp::component::get_ports(std::map<std::string, P *> ports_map,
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

int vp::component::get_ports(bool master, int size, const char *names[], void *ports[])
{
  if (master)
  {
    return get_ports<vp::master_port>(master_ports, size, names, ports);
  }
  else
  {
    return get_ports<vp::slave_port>(slave_ports, size, names, ports);
  }
}

void *vp::component::get_service(string name)
{
  return all_services[name];
}

int vp::component::get_services(int size, const char *names[], void *res_services[])
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

std::vector<std::string> split_name(const std::string& s, char delimiter)
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

vp::config *vp::config::create_config(jsmntok_t *tokens, int *_size)
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

vp::config *vp::config_string::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;
  return NULL;
}

vp::config *vp::config_number::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;
  return NULL;
}

vp::config *vp::config_bool::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;
  return NULL;
}

vp::config *vp::config_array::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;
  return NULL;
}

vp::config *vp::config_object::get_from_list(std::vector<std::string> name_list)
{
  if (name_list.size() == 0) return this;

  vp::config *result = NULL;
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

vp::config *vp::config_object::get(std::string name)
{
  return get_from_list(split_name(name, '/'));
}

vp::config_string::config_string(jsmntok_t *tokens)
{
  value = tokens->str;
}

vp::config_number::config_number(jsmntok_t *tokens)
{
  value = atof(tokens->str);
}

vp::config_bool::config_bool(jsmntok_t *tokens)
{
  value = strcmp(tokens->str, "True") == 0 || strcmp(tokens->str, "true") == 0;
}

vp::config_array::config_array(jsmntok_t *tokens, int *_size)
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

vp::config_object::config_object(jsmntok_t *tokens, int *_size)
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

vp::config *vp::component::import_config(const char *config_string)
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



  return new vp::config_object(tokens);
}

void vp::component::set_services(int nb_services, const char *names[], void *services[])
{
  for (int i=0; i<nb_services; i++)
  {
    all_services[names[i]] = services[i];
  }
}

extern "C" int vp_comp_get_ports(void *comp, bool master, int size, const char *names[], void *ports[])
{
  return ((vp::component *)comp)->get_ports(master, size, names, ports);
}

extern "C" char *vp_get_error()
{
  return vp_error;
}

extern "C" int vp_comp_get_services(void *comp, int size, const char *names[], void *services[])
{
  return ((vp::component *)comp)->get_services(size, names, services);
}

extern "C" void vp_comp_set_services(void *comp, int nb_services, const char *names[], void *services[])
{
  ((vp::component *)comp)->set_services(nb_services, names, services);
}

extern "C" void vp_port_bind_to(void *_master, void *_slave, const char *config_str)
{
  vp::master_port *master = (vp::master_port *)_master;
  vp::slave_port *slave = (vp::slave_port *)_slave;

  vp::config *config = NULL;
  if (config_str != NULL)
  {
    config = master->get_comp()->import_config(config_str);
  }

  master->bind_to(slave, config);
  slave->bind_to(master, config);
}

extern "C" void vp_port_finalize(void *_master)
{
  vp::master_port *master = (vp::master_port *)_master;
  master->finalize();
}

extern "C" void vp_comp_conf(void *comp, const char *path)
{
  ((vp::component *)comp)->conf(path);
}

extern "C" void vp_pre_start(void *comp)
{
  ((vp::component *)comp)->pre_start();
}

extern "C" void vp_load(void *comp)
{
  ((vp::component *)comp)->load();
}

extern "C" void vp_start(void *comp)
{
  ((vp::component *)comp)->start();
}

extern "C" void vp_reset(void *comp)
{
  ((vp::component *)comp)->reset();
}

extern "C" void vp_stop(void *comp)
{
  ((vp::component *)comp)->stop();
}

extern "C" const char *vp_run(void *comp)
{
  return ((vp::component *)comp)->run().c_str();
}

extern "C" int vp_run_status(void *comp)
{
  return ((vp::component *)comp)->run_status();
}

extern "C" void vp_post_post_build(void *comp)
{
  ((vp::component *)comp)->post_post_build();
}

extern "C" int vp_build(void *comp)
{
  ((vp::component *)comp)->pre_build();
  return ((vp::component *)comp)->build();
}
