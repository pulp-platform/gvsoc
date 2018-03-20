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
#include <archi/eu/eu_v3.h>

class Core_event_unit;
class Event_unit;
class Dispatch_unit;



class Dispatch {
public:
  //Plp3_ckg *top;
  //DispatchUnit *dispatch;
  uint32_t value;
  uint32_t status_mask;     // Cores that must get the value before it can be written again
  uint32_t config_mask;     // Cores that will get a valid value
  uint32_t waiting_mask;
  vp::io_req *waiting_reqs[32];
//
//  //gv::ioSlave_ioReq stallRetryCallbackPtr;
//  //function<void (int)> sleepCancelCallback;
//
//  //bool stallRetryCallback(gv::ioReq *req);
  //void sleepCancel(int coreId);
};


class Dispatch_core
{
public:
  int tail;
};


class Dispatch_unit {
public:
  Dispatch_unit(Event_unit *top);

  vp::io_req_status_e req(vp::io_req *req, uint64_t offset, bool is_write, uint32_t *data, int core);
  void reset();

  vp::io_req_status_e enqueue_sleep(Dispatch *dispatch, vp::io_req *req, int core_id);

  //Plp3_ckg *top;
  //gv::trace trace;
  //Dispatch *dispatches;
  //unsigned int globalFifoId;
  //unsigned int fifoId[32];
  unsigned int config;
  //unsigned int teamConfig;
  //bool ioReq(gv::ioReq *req, uint32_t offset, bool isRead, uint32_t *data, int coreId);
  int dispatch_event;
private:
  Event_unit *top;
  Dispatch_core *core;
  Dispatch *dispatches;
  int size;
  int fifo_head;
};



class Barrier {
public:
  uint32_t core_mask;
  uint32_t status;
  uint32_t target_mask;
};


class Barrier_unit {
public:
  Barrier_unit(Event_unit *top);

  vp::io_req_status_e req(vp::io_req *req, uint64_t offset, bool is_write, uint32_t *data, int core);
  void reset();

private:
  void check_barrier(int barrier_id);

  Event_unit *top;
  vp::trace     trace;
  Barrier *barriers;
  int nb_barriers;
  int barrier_event;
};


typedef enum
{
  CORE_STATE_ACTIVE,
  CORE_STATE_WAITING_EVENT,
  CORE_STATE_WAITING_BARRIER
} Event_unit_core_state_e;

class Event_unit : public vp::component
{

  friend class Core_event_unit;
  friend class Dispatch_unit;
  friend class Barrier_unit;

public:

  Event_unit(const char *config);

  void build();
  void start();
  void reset();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);
  static vp::io_req_status_e demux_req(void *__this, vp::io_req *req, int core);
  static void irq_ack_sync(void *__this, int irq, int core);

protected:

  vp::trace     trace;

  vp::io_slave in;

  Core_event_unit *core_eu;
  Dispatch_unit *dispatch;
  Barrier_unit *barrier_unit;

  int nb_core;


  vp::io_req_status_e sw_events_req(vp::io_req *req, uint64_t offset, bool is_write, uint32_t *data);
  void trigger_event(int event, uint32_t core_mask);
  void send_event(int core, uint32_t mask);
  static void in_event_sync(void *__this, bool active, int id);

};



class Core_event_unit
{
public:
  static vp::io_req_status_e req(void *__this, vp::io_req *req);
  void build(Event_unit *top, int core_id);
  void set_status(uint32_t new_value);
  void reset();
  void check_state();
  vp::io_req_status_e req(vp::io_req *req, uint64_t offset, bool is_write, uint32_t *data);
  void check_wait_mask();
  void check_pending_req();
  vp::io_req_status_e wait_event(vp::io_req *req, Event_unit_core_state_e wait_state=CORE_STATE_WAITING_EVENT);
  Event_unit_core_state_e get_state() { return state; }
  void irq_ack_sync(int irq, int core);

  vp::io_slave demux_in;

  uint32_t status;
  uint32_t evt_mask;
  uint32_t irq_mask;
  uint32_t clear_evt_mask;

  int sync_irq;

private:
  Event_unit *top;
  int core_id;
  Event_unit_core_state_e state;
  vp::io_req *pending_req;

  vp::wire_master<bool> barrier_itf;
  vp::wire_slave<bool> in_event_itf[32];

  vp::wire_master<int>    irq_req_itf;
  vp::wire_slave<int>     irq_ack_itf;

};



Event_unit::Event_unit(const char *config)
: vp::component(config)
{
  nb_core = get_config_int("nb_core");
}

void Event_unit::reset()
{
  for (int i=0; i<nb_core; i++)
  {
    core_eu[i].reset();
  }
  dispatch->reset();
  barrier_unit->reset();
}

vp::io_req_status_e Event_unit::req(void *__this, vp::io_req *req)
{
  Event_unit *_this = (Event_unit *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("Event_unit access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  if (size != 4)
  {
    _this->trace.warning("Only 32 bits accesses are allowed\n");
    return vp::IO_REQ_INVALID;
  }

  if (offset >= EU_CORES_AREA_OFFSET && offset < EU_CORES_AREA_OFFSET + EU_CORES_AREA_SIZE)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //unsigned int coreId = EU_CORE_AREA_COREID_GET(offset - EU_CORES_AREA_OFFSET);
    //if (coreId >= nbPes) return true;
    //return coreEvt[coreId].ioReq(req, offset - EU_CORE_AREA_OFFSET_GET(coreId), isRead, data);
  }
  else if (offset >= EU_SOC_EVENTS_AREA_OFFSET && offset < EU_SOC_EVENTS_AREA_OFFSET + EU_SOC_EVENTS_AREA_SIZE)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //return socEventsUnit.ioReq(offset - EU_SOC_EVENTS_AREA_OFFSET, isRead, data);
  }
  else if (offset >= EU_EXT_EVENT_AREA_OFFSET && offset < EU_EXT_EVENT_AREA_OFFSET + EU_EXT_EVENT_AREA_SIZE)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
  }
  else if (offset >= EU_SW_EVENTS_AREA_BASE && offset < EU_SW_EVENTS_AREA_BASE + EU_SW_EVENTS_AREA_SIZE)
  {
    return _this->sw_events_req(req, offset - EU_SW_EVENTS_AREA_BASE, is_write, (uint32_t *)data);
  }
  else if (offset >= EU_BARRIER_AREA_OFFSET && offset < EU_BARRIER_AREA_OFFSET + EU_BARRIER_AREA_SIZE)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //return barrierUnit.barrierIoReq(req, offset - EU_BARRIER_AREA_OFFSET, isRead, data, -1, false);
  }

  return vp::IO_REQ_INVALID;

}



vp::io_req_status_e Event_unit::sw_events_req(vp::io_req *req, uint64_t offset, bool is_write, uint32_t *data)
{
  if (offset >= EU_CORE_TRIGG_SW_EVENT && offset <  EU_CORE_TRIGG_SW_EVENT_SIZE)
  {
    if (!is_write) return vp::IO_REQ_INVALID;

    int event = (offset - EU_CORE_TRIGG_SW_EVENT) >> 2;
    trace.msg("SW event trigger (event: %d, coreMask: 0x%x)\n", event, *data);
    trigger_event(1<<event, *data);
  }
  else if (offset >= EU_CORE_TRIGG_SW_EVENT_WAIT && offset <  EU_CORE_TRIGG_SW_EVENT_WAIT_SIZE)
  {
    trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) {
    //  trace.msg("Writing event trigger target core mask (coreMask: 0x%x)\n", *data);
    //  targetCoreMask = *data;
    //}
    //else
    //{
    //  int event = (offset - EU_CORE_TRIGG_SW_EVENT_WAIT) >> 2;
    //  trace.msg("Event trigger and wait (event: %d, coreMask: 0x%x)\n", event, *data);
    //  top->triggerEvent(1<<event, targetCoreMask);
    //  *data = waitEvent(req);
    //  return req->error != 0;
    //}
  }
  else if (offset >= EU_CORE_TRIGG_SW_EVENT_WAIT_CLEAR && offset <  EU_CORE_TRIGG_SW_EVENT_WAIT_CLEAR_SIZE)
  {
    trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) {
    //  trace.msg("Writing event trigger target core mask (coreMask: 0x%x)\n", *data);
    //  targetCoreMask = *data;
    //}
    //else
    //{
    //  int event = (offset - EU_CORE_TRIGG_SW_EVENT_WAIT) >> 2;
    //  trace.msg("Event trigger, wait and clear (event: %d, coreMask: 0x%x)\n", event, *data);
    //  top->triggerEvent(1<<event, targetCoreMask);
    //  clearEvtMask = evtMask;
    //  *data = waitEvent(req);
    //  return req->error != 0;
    //}
  }
  else
  {
    trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    return vp::IO_REQ_INVALID;    
  }

  return vp::IO_REQ_OK;
}


void Event_unit::trigger_event(int event_mask, uint32_t core_mask)
{
  for (unsigned int i=0; i<nb_core; i++) {
    if (core_mask == 0 || (core_mask & (1 << i))) {
      send_event(i, event_mask);
    }
  }
  }

void Event_unit::send_event(int core, uint32_t mask)
{
  trace.msg("Triggering event (core: %d, mask: 0x%x)\n", core, mask);
  Core_event_unit *eu = &core_eu[core];
  eu->set_status(eu->status | mask);
  eu->check_state();
}


void Core_event_unit::build(Event_unit *top, int core_id)
{
  this->top = top;
  this->core_id = core_id;
  demux_in.set_req_meth_muxed(&Event_unit::demux_req, core_id);
  top->new_slave_port("demux_in_" + std::to_string(core_id), &demux_in);

  top->new_master_port("irq_req_" + std::to_string(core_id), &irq_req_itf);

  irq_ack_itf.set_sync_meth_muxed(&Event_unit::irq_ack_sync, core_id);
  top->new_slave_port("irq_ack_" + std::to_string(core_id), &irq_ack_itf);

  for (int i=0; i<32; i++)
  {
    in_event_itf[i].set_sync_meth_muxed(&Event_unit::in_event_sync, (core_id << 16 | i));
    top->new_slave_port("in_event_" + std::to_string(i) + "_pe_" + std::to_string(core_id), &in_event_itf[i]);
  }

}

vp::io_req_status_e Core_event_unit::req(vp::io_req *req, uint64_t offset, bool is_write, uint32_t *data)
{
  if (offset == EU_CORE_BUFFER)
  {
    top->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //*data = status;
    //checkCoreState();
    return vp::IO_REQ_INVALID;
  }
  else if (offset == EU_CORE_BUFFER_MASKED)
  {
    top->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //*data = status & evtMask;
    return vp::IO_REQ_INVALID;
  }
  else if (offset == EU_CORE_BUFFER_CLEAR)
  {
    top->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (isRead) return true;
    //setStatus(req->getSecure(), status & (~*data));
    //trace.msg("Clearing buffer status (mask: 0x%x, newValue: 0x%x)\n", *data, status);
    //checkCoreState();
    return vp::IO_REQ_INVALID;
  }
  else if (offset == EU_CORE_MASK)
  {
    if (!is_write) *data = evt_mask;
    else {
      evt_mask = *data;
      top->trace.msg("Updating event mask (newValue: 0x%x)\n", *data);
      check_state();
    }
  }
  else if (offset == EU_CORE_MASK_AND)
  {
    if (!is_write) return vp::IO_REQ_INVALID;
    evt_mask &= ~*data;
    top->trace.msg("Clearing event mask (mask: 0x%x, newValue: 0x%x)\n", *data, evt_mask);
    check_state();
  }
  else if (offset == EU_CORE_MASK_OR)
  {
    if (!is_write) return vp::IO_REQ_INVALID;
    evt_mask |= *data;
    top->trace.msg("Setting event mask (mask: 0x%x, newValue: 0x%x)\n", *data, evt_mask);
    check_state();
  }
  else if (offset == EU_CORE_STATUS)
  {
    top->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //*data = active;
    return vp::IO_REQ_INVALID;
  }
  else if (offset == EU_CORE_EVENT_WAIT)
  {
    top->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //trace.msg("Wait\n");
    //*data = waitEvent(req);
    //return req->error != 0;
    return vp::IO_REQ_INVALID;
  }

  else if (offset == EU_CORE_EVENT_WAIT_CLEAR)
  {
    top->trace.msg("Wait and clear\n");
    clear_evt_mask = evt_mask;
    vp::io_req_status_e err = wait_event(req);
    *data = evt_mask & status;
    return err;
  }
  else if (offset == EU_CORE_MASK_IRQ)
  {
    if (!is_write) *data = irq_mask;
    else {
      top->trace.msg("Updating irq mask (newValue: 0x%x)\n", *data);
      irq_mask = *data;
      check_state();
    }
  }
  else if (offset == EU_CORE_BUFFER_IRQ_MASKED)
  {
    if (is_write) return vp::IO_REQ_INVALID;
    *data = status & irq_mask;
  }
  else if (offset == EU_CORE_MASK_IRQ_AND)
  {
    if (!is_write) return vp::IO_REQ_INVALID;
    irq_mask &= ~*data;
    top->trace.msg("Clearing irq mask (mask: 0x%x, newValue: 0x%x)\n", *data, irq_mask);
    check_state();
  }
  else if (offset == EU_CORE_MASK_IRQ_OR)
  {
    if (!is_write) return vp::IO_REQ_INVALID;
    irq_mask |= *data;
    top->trace.msg("Setting irq mask (mask: 0x%x, newValue: 0x%x)\n", *data, irq_mask);
    check_state();
  }
  else
  {
    return vp::IO_REQ_INVALID;
  }

  return vp::IO_REQ_OK;
}

void Event_unit::irq_ack_sync(void *__this, int irq, int core)
{
  Event_unit *_this = (Event_unit *)__this;

  _this->trace.msg("Received IRQ acknowledgement (core: %d, irq: %d)\n", core, irq);

  _this->core_eu[core].irq_ack_sync(irq, core);
}

vp::io_req_status_e Event_unit::demux_req(void *__this, vp::io_req *req, int core)
{
  Event_unit *_this = (Event_unit *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("Demux event_unit access (core: %d, offset: 0x%x, size: 0x%x, is_write: %d)\n", core, offset, size, is_write);

  if (size != 4)
  {
    _this->trace.warning("Only 32 bits accesses are allowed\n");
    return vp::IO_REQ_INVALID;
  }

  if (offset >= EU_CORE_DEMUX_OFFSET && offset < EU_CORE_DEMUX_OFFSET + EU_CORE_DEMUX_SIZE)
  {
    return _this->core_eu[core].req(req, offset - EU_CORE_DEMUX_OFFSET, is_write, (uint32_t *)data);
  }
  else if (offset >= EU_MUTEX_DEMUX_OFFSET && offset < EU_MUTEX_DEMUX_OFFSET + EU_MUTEX_DEMUX_SIZE)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //return mutexUnit.ioReq(req, offset - EU_MUTEX_DEMUX_OFFSET, isRead, data, coreId);
  }
  else if (offset >= EU_DISPATCH_DEMUX_OFFSET && offset < EU_DISPATCH_DEMUX_OFFSET + EU_DISPATCH_DEMUX_SIZE)
  {
    return _this->dispatch->req(req, offset - EU_DISPATCH_DEMUX_OFFSET, is_write, (uint32_t *)data, core);
  }
  else if (offset >= EU_LOOP_DEMUX_OFFSET && offset < EU_LOOP_DEMUX_OFFSET + EU_LOOP_DEMUX_SIZE)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //return loopUnit.ioReq(req, offset - EU_LOOP_DEMUX_OFFSET, isRead, data, coreId);
  }
  else if (offset >= EU_SW_EVENTS_DEMUX_OFFSET && offset < EU_SW_EVENTS_DEMUX_OFFSET + EU_SW_EVENTS_DEMUX_SIZE)
  {
    return _this->sw_events_req(req, offset - EU_SW_EVENTS_DEMUX_OFFSET, is_write, (uint32_t *)data);
  }
  else if (offset >= EU_BARRIER_DEMUX_OFFSET && offset < EU_BARRIER_DEMUX_OFFSET + EU_BARRIER_DEMUX_SIZE)
  {
    return _this->barrier_unit->req(req, offset - EU_BARRIER_DEMUX_OFFSET, is_write, (uint32_t *)data, core);
  }

  return vp::IO_REQ_INVALID;
}

void Event_unit::in_event_sync(void *__this, bool active, int id)
{
  Event_unit *_this = (Event_unit *)__this;
  int core_id = id >> 16;
  int event_id = id & 0xffff;
  _this->trace.msg("Received input event (core: %d, event: %d, active: %d)\n", core_id, event_id, active);
  Core_event_unit *eu = &_this->core_eu[core_id];
  eu->set_status(eu->status | (1<<event_id));
  eu->check_state();
}

void Event_unit::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);

  in.set_req_meth(&Event_unit::req);
  new_slave_port("in", &in);

  core_eu = (Core_event_unit *)new Core_event_unit[nb_core];
  dispatch = new Dispatch_unit(this);
  barrier_unit = new Barrier_unit(this);

  for (int i=0; i<nb_core; i++)
  {
    core_eu[i].build(this, i);
  }

  reset();
}

void Event_unit::start()
{
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new Event_unit(config);
}



void Core_event_unit::irq_ack_sync(int irq, int core)
{
  set_status(status & ~(1<<irq));
  sync_irq = -1;

  check_state();
}



void Core_event_unit::set_status(uint32_t new_value)
{
  status = new_value;
}


void Core_event_unit::check_pending_req()
{
  pending_req->get_resp_port()->resp(pending_req);
}



void Core_event_unit::check_wait_mask()
{
  if (clear_evt_mask)
  {
    set_status(status & (~clear_evt_mask));
    top->trace.msg("Clear event after wake-up (evtMask: 0x%x, status: 0x%x)\n", clear_evt_mask, status);
    clear_evt_mask = 0;
  }
}

vp::io_req_status_e Core_event_unit::wait_event(vp::io_req *req, Event_unit_core_state_e wait_state)
{
  top->trace.msg("Wait request (status: 0x%x, evt_mask: 0x%x)\n", status, evt_mask);

  if (evt_mask & status)
  {
    // Case where the core ask for clock-gating but the event status prevent him from doing so
    // In this case, don't forget to clear the status in case of wait and clear
    check_wait_mask();
    return vp::IO_REQ_OK;
  }
  else
  {
    state = wait_state;
    pending_req = req;
    return vp::IO_REQ_PENDING;
  }
}

vp::io_req_status_e Core_event_unit::req(void *__this, vp::io_req *req)
{
  Event_unit *_this = (Event_unit *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("Event_unit access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, is_write);

  if (offset == EU_CORE_BUFFER)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //*data = status;
    //checkCoreState();
  }
  else if (offset == EU_CORE_BUFFER_MASKED)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //*data = status & evtMask;
  }
  else if (offset == EU_CORE_BUFFER_CLEAR)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (isRead) return true;
    //setStatus(req->getSecure(), status & (~*data));
    //trace.msg("Clearing buffer status (mask: 0x%x, newValue: 0x%x)\n", *data, status);
    //checkCoreState();
  }
  else if (offset == EU_CORE_MASK)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (isRead) *data = evtMask;
    //else {
    //  evtMask = *data;
    //  trace.msg("Updating event mask (newValue: 0x%x)\n", *data);
    //  checkCoreState();
    //}
  }
  else if (offset == EU_CORE_MASK_AND)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (isRead) return true;
    //evtMask &= ~*data;
    //trace.msg("Clearing event mask (mask: 0x%x, newValue: 0x%x)\n", *data, evtMask);
    //checkCoreState();
  }
  else if (offset == EU_CORE_MASK_OR)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (isRead) return true;
    //evtMask |= *data;
    //trace.msg("Setting event mask (mask: 0x%x, newValue: 0x%x)\n", *data, evtMask);
    //checkCoreState();
  }
  else if (offset == EU_CORE_STATUS)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //*data = active;
  }
  else if (offset == EU_CORE_EVENT_WAIT)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //trace.msg("Wait\n");
    //*data = waitEvent(req);
    //return req->error != 0;
  }

  else if (offset == EU_CORE_EVENT_WAIT_CLEAR)
  {

    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //trace.msg("Wait and clear\n");
    //clearEvtMask = evtMask;
    //*data = waitEvent(req);
    //return req->error != 0;
  }
  else if (offset == EU_CORE_MASK_IRQ)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (isRead) *data = irqMask;
    //else {
    //  trace.msg("Updating irq mask (newValue: 0x%x)\n", *data);
    //  irqMask = *data;
    //  checkCoreState();
    //}
  }
  else if (offset == EU_CORE_BUFFER_IRQ_MASKED)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (!isRead) return true;
    //*data = status & irqMask;
  }
  else if (offset == EU_CORE_MASK_IRQ_AND)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (isRead) return true;
    //irqMask &= ~*data;
    //trace.msg("Clearing irq mask (mask: 0x%x, newValue: 0x%x)\n", *data, irqMask);
    //checkCoreState();
  }
  else if (offset == EU_CORE_MASK_IRQ_OR)
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
    //if (isRead) return true;
    //irqMask |= *data;
    //trace.msg("Setting irq mask (mask: 0x%x, newValue: 0x%x)\n", *data, irqMask);
    //checkCoreState();
  }
  else
  {
    _this->trace.warning("UNIMPLEMENTED at %s %d\n", __FILE__, __LINE__);
  }

  return vp::IO_REQ_INVALID;

  return vp::IO_REQ_OK;
}

void Core_event_unit::reset()
{
  status = 0;
  evt_mask = 0;
  irq_mask = 0;
  clear_evt_mask = 0;
  sync_irq = -1;
  state = CORE_STATE_ACTIVE;
}

void Core_event_unit::check_state()
{
  //top->trace.msg("Checking core state (coreId: %d, active: %d, waitingEvent: %d, status: 0x%llx, evtMask: 0x%llx, irqMask: 0x%llx)\n", coreId, active, waitingEvent, status, evtMask, irqMask);

  uint32_t status_masked = status & irq_mask;
  int irq = status_masked ? 31 - __builtin_clz(status_masked) : -1;

  if (irq != sync_irq) {
    top->trace.msg("Updating irq req (core: %d, irq: %d)\n", core_id, irq);
    sync_irq = irq;
    irq_req_itf.sync(irq);
  }

  switch (state)
  {
    case CORE_STATE_ACTIVE:
    break;

    case CORE_STATE_WAITING_EVENT:
    case CORE_STATE_WAITING_BARRIER:
    if (status & evt_mask)
    {
      top->trace.msg("Activating clock (core: %d)\n", core_id);
      state = CORE_STATE_ACTIVE;
      check_wait_mask();
      check_pending_req();
    }
    break;
  }

}





/****************
 * DISPATCH UNIT
 ****************/


vp::io_req_status_e Dispatch_unit::enqueue_sleep(Dispatch *dispatch, vp::io_req *req, int core_id) {
  Core_event_unit *core_eu = &top->core_eu[core_id];

  // Enqueue the request so that the core can be unstalled when a value is pushed
  dispatch->waiting_reqs[core_id] = req;
  dispatch->waiting_mask |= 1<<core_id;

  // Don't forget to remember to clear the event after wake-up by the dispatch event
  core_eu->clear_evt_mask = 1<<dispatch_event;

  return core_eu->wait_event(req);
}



Dispatch_unit::Dispatch_unit(Event_unit *top)
: top(top)
{
  dispatch_event = top->get_config_int("**/properties/events/dispatch");
  size = top->get_config_int("**/properties/dispatch/size");
  core = new Dispatch_core[top->nb_core];
  dispatches = new Dispatch[size];
}

  void Dispatch_unit::reset()
  {
    fifo_head = 0;
    config = 0;
    for (int i=0; i<top->nb_core; i++)
    {
      core[i].tail = 0;
    }
    for (int i=0; i<size; i++)
    {
      dispatches[i].value = 0;
      dispatches[i].status_mask = 0;
      dispatches[i].config_mask = 0;
      dispatches[i].waiting_mask = 0;
    }
  }

  vp::io_req_status_e Dispatch_unit::req(vp::io_req *req, uint64_t offset, bool is_write, uint32_t *data, int core_id)
  {
    if (offset == EU_DISPATCH_FIFO_ACCESS)
    {
      if (is_write)
      {
        unsigned int id = fifo_head++;
        if (fifo_head == size) fifo_head = 0;

        Dispatch *dispatch = &dispatches[id];

        // When pushing to the FIFO, the global config is pushed to the elected dispatcher
        dispatch->config_mask = config;     // Cores that will get a valid value

        top->trace.msg("Pushing dispatch value (dispatch: %d, value: 0x%x, coreMask: 0x%x)\n", id, *data, dispatch->config_mask);

        // Case where the master push a value
        dispatch->value = *data;
        // Reinitialize the status mask to notify a new value is ready
        dispatch->status_mask = -1;
        // Then wake-up the waiting cores
        unsigned int mask = dispatch->waiting_mask & dispatch->status_mask;
        for (int i=0; i<32 && mask; i++)
        {
          if (mask & (1<<i))
          {
            // Only wake-up the core if he's actually involved in the team
            if (dispatch->config_mask & (1<<i))
            {
              top->trace.msg("Waking-up core waiting for dispatch value (coreId: %d)\n", i);
              vp::io_req *waiting_req = dispatch->waiting_reqs[i];

              // Clear the status bit as the waking core takes the data
              dispatch->status_mask &= ~(1<<i);
              dispatch->waiting_mask &= ~(1<<i);

              // Store the dispatch value into the pending request and reply to the
              // initiator
              *(uint32_t *)waiting_req->get_data() = dispatch->value;
              waiting_req->get_resp_port()->resp(waiting_req);

              // Update the core fifo
              core[i].tail++;
              if (core[i].tail == size) core[i].tail = 0;

              // Clear the mask to stop iterating early
              mask &= ~(1<<i);

              // And trigger the event to the core
              top->trigger_event(1<<dispatch_event, 1<<i); 
            }
            // Otherwise keep him sleeping and increase its index so that he will bypass this entry when he wakes up
            else
            {
              // Cancel current dispatch sleep
              dispatch->status_mask &= ~(1<<i);
              dispatch->waiting_mask &= ~(1<<i);
              vp::io_req *pending_req = dispatch->waiting_reqs[i];

              // Bypass the current entry
              core[i].tail++;
              if (core[i].tail == size) core[i].tail = 0;

              // And reenqueue to the next entry
              id = core[i].tail;
              enqueue_sleep(&dispatches[id], pending_req, i);
              top->trace.msg("Incrementing core counter to bypass entry (coreId: %d, newIndex: %d)\n", i, id);
            }
          }
        }

        return vp::IO_REQ_OK;        
      }
      else
      {
        int id = core[core_id].tail;
        Dispatch *dispatch = &dispatches[id];

        top->trace.msg("Trying to get dispatch value (dispatch: %d)\n", id);

        // In case we found ready elements where this core is not involved, bypass them all
        while ((dispatch->status_mask & (1<<core_id)) && !(dispatch->config_mask & (1<<core_id))) {
          dispatch->status_mask &= ~(1<<core_id);
          core[core_id].tail++;
          if (core[core_id].tail == size) core[core_id].tail = 0;
          id = core[core_id].tail;
          dispatch = &dispatches[id];
          top->trace.msg("Incrementing core counter to bypass entry (coreId: %d, newIndex: %d)\n", core_id, id);
        }

        // Case where a slave tries to get a value
        if (dispatch->status_mask & (1<<core_id))
        {
          // A value is ready. Get it and clear the status bit to not read it again the next time
          // In case the core is not involved in this dispatch, returns 0
          if (dispatch->config_mask & (1<<core_id)) *data = dispatch->value;
          else *data = 0;
          dispatch->status_mask &= ~(1<<core_id);
          top->trace.msg("Getting ready dispatch value (dispatch: %d, value: %x, dispatchStatus: 0x%x)\n", id, dispatch->value, dispatch->status_mask);
          core[core_id].tail++;
          if (core[core_id].tail == size) core[core_id].tail = 0;
        }
        else
        {
          // Nothing is ready, go to sleep
          top->trace.msg("No ready dispatch value, going to sleep (dispatch: %d, value: %x, dispatchStatus: 0x%x)\n", id, dispatch->value, dispatch->status_mask);
          return enqueue_sleep(dispatch, req, core_id);
        }
      }

      return vp::IO_REQ_INVALID;
    }
    else if (offset == EU_DISPATCH_TEAM_CONFIG)
    {
      config = *data;
      return vp::IO_REQ_OK;
    }
    else
    {
      return vp::IO_REQ_INVALID;
    }
  }






/****************
 * BARRIER UNIT
 ****************/


Barrier_unit::Barrier_unit(Event_unit *top)
: top(top)
{
  top->traces.new_trace("barrier/trace", &trace, vp::DEBUG);
  nb_barriers = top->get_config_int("**/properties/barriers/nb_barriers");
  barrier_event = top->get_config_int("**/properties/events/barrier");
  barriers = new Barrier[nb_barriers];
}

void Barrier_unit::check_barrier(int barrier_id)
{
  Barrier *barrier = &barriers[barrier_id];

  if (barrier->status == barrier->core_mask) 
  {
    trace.msg("Barrier reached, triggering event (barrier: %d, coreMask: 0x%x, targetMask: 0x%x)\n", barrier_id, barrier->core_mask, barrier->target_mask);
    barrier->status = 0;
    top->trigger_event(1<<barrier_event, barrier->target_mask);
  }
}


vp::io_req_status_e Barrier_unit::req(vp::io_req *req, uint64_t offset, bool is_write, uint32_t *data, int core)
{
  unsigned int barrier_id = EU_BARRIER_AREA_BARRIERID_GET(offset);
  offset = offset - EU_BARRIER_AREA_OFFSET_GET(barrier_id);
  if (barrier_id >= nb_barriers) return vp::IO_REQ_INVALID;
  Barrier *barrier = &barriers[barrier_id];

  if (offset == EU_HW_BARR_TRIGGER_MASK)
  {
    if (!is_write) *data = barrier->core_mask;
    else {
      trace.msg("Setting barrier core mask (barrier: %d, mask: 0x%x)\n", barrier_id, *data);
      barrier->core_mask = *data;
      check_barrier(barrier_id);
    }
  }

  else if (offset == EU_HW_BARR_TARGET_MASK)
  {
    if (!is_write) *data = barrier->target_mask;
    else {
      trace.msg("Setting barrier target mask (barrier: %d, mask: 0x%x)\n", barrier_id, *data);
      barrier->target_mask = *data;
      check_barrier(barrier_id);
    }
  }
  else if (offset == EU_HW_BARR_STATUS)
  {
    if (!is_write) *data = barrier->status;
    else {
      trace.msg("Setting barrier status (barrier: %d, status: 0x%x)\n", barrier_id, *data);
      barrier->status = *data;
      check_barrier(barrier_id);
    }
  }
  else if (offset == EU_HW_BARR_TRIGGER)
  {
    if (!is_write) return vp::IO_REQ_INVALID;
    else {
      barrier->status |= *data;
      trace.msg("Barrier mask trigger (barrier: %d, mask: 0x%x, newStatus: 0x%x)\n", barrier_id, *data, barrier->status);
    }

    check_barrier(barrier_id);
  }
  else if (offset == EU_HW_BARR_TRIGGER_SELF)
  {
    // The access is valid only through the demux
    if (core == -1) return vp::IO_REQ_INVALID;
    barrier->status |= 1 << core
    ;
    trace.msg("Barrier trigger (barrier: %d, coreId: %d, newStatus: 0x%x)\n", barrier_id, core, barrier->status);

    check_barrier(barrier_id);
  }
  else if (offset == EU_HW_BARR_TRIGGER_WAIT)
  {
    // The access is valid only through the demux
    if (core == -1) return vp::IO_REQ_INVALID;

    Core_event_unit *core_eu = &top->core_eu[core];
    if (core_eu->get_state() == CORE_STATE_WAITING_BARRIER)
    {
      // The core was already waiting for the barrier which means it was interrupted
      // by an interrupt. Just resume the barrier by going to sleep
      trace.msg("Resuming barrier trigger and wait (barrier: %d, coreId: %d, newStatus: 0x%x)\n", barrier_id, core, barrier->status);
    }
    else
    {
      barrier->status |= 1 << core;
      trace.msg("Barrier trigger and wait (barrier: %d, coreId: %d, newStatus: 0x%x)\n", barrier_id, core, barrier->status);
    }

    check_barrier(barrier_id);

    return core_eu->wait_event(req, CORE_STATE_WAITING_BARRIER);
  }
  else if (offset == EU_HW_BARR_TRIGGER_WAIT_CLEAR)
  {
    // The access is valid only through the demux
    if (core == -1) return vp::IO_REQ_INVALID;

    Core_event_unit *core_eu = &top->core_eu[core];
    if (core_eu->get_state() == CORE_STATE_WAITING_BARRIER)
    {
      // The core was already waiting for the barrier which means it was interrupted
      // by an interrupt. Just resume the barrier by going to sleep
      trace.msg("Resuming barrier trigger and wait (barrier: %d, coreId: %d, mask: 0x%x, newStatus: 0x%x)\n", barrier_id, core, barrier->core_mask, barrier->status);
    }
    else
    {
      barrier->status |= 1 << core;
      trace.msg("Barrier trigger, wait and clear (barrier: %d, coreId: %d, newStatus: 0x%x)\n", barrier_id, core, barrier->status);
    }
    core_eu->clear_evt_mask = core_eu->evt_mask;

    check_barrier(barrier_id);

    return core_eu->wait_event(req, CORE_STATE_WAITING_BARRIER);
  }
  else if (offset == EU_HW_BARR_STATUS_SUMMARY)
  {
    if (is_write) return vp::IO_REQ_INVALID;
    uint32_t status = 0;
    for (unsigned int i=1; i<nb_barriers; i++) status |= barriers[i].status;
    *data = status;
  }
  else return vp::IO_REQ_INVALID;

  return vp::IO_REQ_OK;
}

void Barrier_unit::reset()
{
  for (int i=0; i<nb_barriers; i++)
  {
    Barrier *barrier = &barriers[i];
    barrier->core_mask = 0;
    barrier->status = 0;
    barrier->target_mask = 0;
  }
}
