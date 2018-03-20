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
#include <stdio.h>
#include "string.h"
#include <iostream>
#include <sstream>
#include <string>

vp::component::component(const char *config_string) : cm::component(config_string), traces(*this)
{
}

void vp::component::post_post_build()
{
  traces.post_post_build();
}





void vp::component_clock::clk_reg(component *_this, component *clock) {
  _this->clock = (clock_engine *)clock;
}



void vp::component_clock::pre_build(component *comp) {
  clock_port.set_reg_meth((vp::clk_reg_meth_t *)&component_clock::clk_reg);
  comp->new_slave_port("clock", &clock_port);
}

void vp::time_engine::enqueue(time_engine_client *client, int64_t time)
{
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

vp::clock_event *vp::clock_engine::enqueue_other(vp::clock_event *event, int64_t cycle)
{
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
    vp::clock_event *current = event_queue[cycle_diff], *prev = NULL;
    while (current != event)
    {
      prev = current;
      current = current->next;
    }
    if (prev)
      prev->next = event->next;
    else
      event_queue[cycle_diff] = event->next;
  }
}

int64_t vp::clock_engine::exec()
{

  if (unlikely(current_cycle == 0))
  {
    clock_event *event = delayed_queue;
    while (event)
    {
      if (nb_enqueued_to_cycle == 0) cycles = event->cycle;

      uint64_t cycle_diff = event->cycle - get_cycles();
      if (cycle_diff >= CLOCK_EVENT_QUEUE_SIZE) break;

      enqueue_to_cycle(event, cycle_diff);

      event = event->next;
      delayed_queue = event;
    }
  }

  clock_event *current = event_queue[current_cycle];

  //printf("[%ld] %p ENGINE EXEC CYCLE %d first %p %d\n", get_time(), this, current_cycle, current, nb_enqueued_to_cycle);

  while (likely(current != NULL))
  {
    //printf("HANDLING EVENT %p meth %p\n", current, current->meth);
    clock_event *next = current->next;
    current->enqueued = false;
    nb_enqueued_to_cycle--;
    current->meth(static_cast<vp::component *>(current->comp), current);
    current = next;
  }
  event_queue[current_cycle] = NULL;

  if (likely(nb_enqueued_to_cycle))
  {
    cycles++;
    current_cycle = (current_cycle + 1) & CLOCK_EVENT_QUEUE_MASK;
    return period;
  }
  else
  {
    current_cycle = 0;
    if (delayed_queue)
    {
      return (delayed_queue->cycle - get_cycles()) * period;
    }
    else
    {
      return -1;
    }
  }
}
