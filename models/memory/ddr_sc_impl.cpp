/* vim: set ts=2 sw=2 expandtab:*/
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
#include <systemc.h>
#include <tlm>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>
#include <tlm_utils/multi_passthrough_initiator_socket.h>

#include "ems_mm.h"
#include "ems_common.h"
#include "ems_at_bus.h"
#include "tlm2_base_protocol_checker.h"
#ifdef __VP_USE_SYSTEMC_GEM5
#include "ems_gem5_tlm_br.h"
#endif /* __VP_USE_SYSTEMC_GEM5 */
#ifdef __VP_USE_SYSTEMC_DRAMSYS
#include "DRAMSys.h"
#else
#include "ems_at_target.h"
#endif /* __VP_USE_SYSTEMC_DRAMSYS */


#define BYTES_PER_ACCESS            8
#define ACCEPT_DELAY_PS             1000
#define TARGET_LATENCY_PS           2000

class ddr;

class ddr_module: public sc_module {
  friend class ddr;
  tlm_utils::simple_initiator_socket<ddr_module> isocket;

  SC_HAS_PROCESS(ddr_module);
  ddr_module(sc_module_name nm, ddr *vp_module) :
    sc_module(nm),
    in_progress(NULL),
    vp_module(vp_module),
    bytes_per_access(BYTES_PER_ACCESS),
    curr_req(NULL),
    isocket("isocket"),
    peq(this, &ddr_module::peq_callback)
  {
    resp_accept_delay = sc_core::sc_time(ACCEPT_DELAY_PS, SC_PS);
    isocket.register_nb_transport_bw(this, &ddr_module::nb_transport_bw);
    SC_THREAD(run);
  }

  void run();
  void req_to_gp(vp::io_req *r, tlm::tlm_generic_payload *p, uint32_t tid, bool last);
  tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload &p, tlm::tlm_phase &phase, sc_core::sc_time &d);
  void peq_callback(tlm::tlm_generic_payload &p, const tlm::tlm_phase &phase);
  void inspect(tlm::tlm_generic_payload &p);

private:
  sc_event event;
  sc_event end_req_event;
  sc_event all_trans_completed;
  tlm::tlm_generic_payload *in_progress;
  tlm_utils::peq_with_cb_and_phase<ddr_module> peq;
  ddr *vp_module;
  ems::mm mm;
  uint32_t bytes_per_access;
  vp::io_req *curr_req;
  sc_core::sc_time resp_accept_delay;
};

class ddr : public vp::component
{

  friend class ddr_module;

public:

  ddr(const char *config);

  int build();
  void start();
  void elab();

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

protected:
  vp::io_req *first_pending_reqs = NULL;
  vp::io_req *first_stalled_req = NULL;

private:

  vp::trace     trace;
  vp::io_slave in;

  uint64_t size = 0;
  int max_reqs = 4;
  int current_reqs = 0;
  int count = 0;
  vp::io_req *last_pending_reqs = NULL;
  // Transactor module
  ddr_module *sc_module;
  // Interconnect component
  ems::at_bus *at_bus;
  // A protocol checker for each hop
  // pcib: protocol checker between initiator and bus
  tlm_utils::tlm2_base_protocol_checker<> *pcib;
  // pcbt: protocol checker between bus and target
  tlm_utils::tlm2_base_protocol_checker<> *pcbt;
#ifdef __VP_USE_SYSTEMC_GEM5
  // gem5-TLM bridge
  ems::gem5_tlm_br *g5tbr;
#endif /* __VP_USE_SYSTEMC_GEM5 */
#ifdef __VP_USE_SYSTEMC_DRAMSYS
  DRAMSys *dramsys;
#else
  ems::at_target *at_target;
#endif /* __VP_USE_SYSTEMC_DRAMSYS */

};

ddr::ddr(const char *config)
  : vp::component(config)
{

}

vp::io_req_status_e ddr::req(void *__this, vp::io_req *req)
{
  ddr *_this = (ddr *)__this;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();

  _this->trace.msg("ddr access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, req->get_is_write());

  if (offset + size > _this->size) {
    _this->warning.warning("Received out-of-bound request (reqAddr: 0x%llx, reqSize: 0x%llx, memSize: 0x%llx)\n", offset, size, _this->size);
    return vp::IO_REQ_INVALID;
  }

  if (_this->first_pending_reqs)
    _this->last_pending_reqs->set_next(req);
  else
    _this->first_pending_reqs = req;
  _this->last_pending_reqs = req;
  req->set_next(NULL);

  _this->current_reqs++;

  if (_this->current_reqs > _this->max_reqs)
  {
    if (_this->first_stalled_req == NULL)
      _this->first_stalled_req = _this->last_pending_reqs;
    _this->trace.msg("ddr access (offset: 0x%x, size: 0x%x DENIED)\n", offset, size);
    return vp::IO_REQ_DENIED;
  }
  else
  {
    _this->sc_module->event.notify();
    _this->trace.msg("ddr access (offset: 0x%x, size: 0x%x sc_module notified)\n", offset, size);
    return vp::IO_REQ_PENDING;
  }
}

int ddr::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);

  in.set_req_meth(&ddr::req);
  new_slave_port("input", &in);

  return 0;
}


void ddr::elab()
{
  sc_module = new ddr_module("wrapper2", this);
  at_bus = new ems::at_bus("at_bus");
  pcib = new tlm_utils::tlm2_base_protocol_checker<>("pcib");
  pcbt = new tlm_utils::tlm2_base_protocol_checker<>("pcbt");

  sc_module->isocket.bind(pcib->target_socket);
  pcib->initiator_socket.bind(at_bus->tsocket);
  at_bus->isocket.bind(pcbt->target_socket);

#ifdef __VP_USE_SYSTEMC_GEM5
  // Instantiate gem5_tlm_br
  std::string cfg = std::string(__GEM5_PATH) + std::string("/config.ini");
  std::string cmd = "grep port_data= " + cfg + " | wc -l";
  auto nports = std::stoul(ems::sh_exec(cmd.c_str()));
  g5tbr = new ems::gem5_tlm_br("g5tbr", cfg, nports);
  assert(g5tbr->transactors.size() == nports);
  for (auto t : g5tbr->transactors) {
      t->socket.bind(at_bus->tsocket);
  }
#endif /* __VP_USE_SYSTEMC_GEM5 */
#ifdef __VP_USE_SYSTEMC_DRAMSYS
  std::string resources = std::string(__DRAMSYS_PATH) + std::string("/DRAMSys/library/resources/");
  std::string simulation_xml = resources + "simulations/ddr3-example.xml";
  dramsys = new DRAMSys("DRAMSys", simulation_xml, resources);
  pcbt->initiator_socket.bind(dramsys->tSocket);
#else
  at_target = new ems::at_target("at_target", ACCEPT_DELAY_PS, TARGET_LATENCY_PS, BYTES_PER_ACCESS);
  pcbt->initiator_socket.bind(at_target->tsocket);
#endif /* __VP_USE_SYSTEMC_DRAMSYS */
}


void ddr::start()
{
  size = get_config_int("size");

  trace.msg("Building ddr (size: 0x%lx)\n", size);

}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new ddr(config);
}

void ddr_module::req_to_gp(vp::io_req *r, tlm::tlm_generic_payload *p, uint32_t tid, bool last)
{
  assert(r);
  assert(p);
  assert(p->has_mm());

  tlm::tlm_command cmd;
  cmd = r->get_is_write() ? tlm::TLM_WRITE_COMMAND : tlm::TLM_READ_COMMAND;
  uint32_t offset = tid * bytes_per_access;
  sc_dt::uint64 addr = r->get_addr() + offset;
  unsigned char *data = r->get_data();
  unsigned char *dptr = &data[offset];
  unsigned int size = bytes_per_access;

  p->set_command(cmd);
  p->set_address(addr);
  p->set_data_ptr(dptr);
  p->set_data_length(size);
  p->set_streaming_width(size);
  p->set_byte_enable_ptr(NULL);
  p->set_dmi_allowed(false);
  p->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  ems::req_extension *ext = new ems::req_extension(r, tid, last);
  p->set_auto_extension(ext);
}

// Called on receiving BEGIN_RESP or TLM_COMPLETED
void ddr_module::inspect(tlm::tlm_generic_payload &p)
{
  if (p.is_response_error()) {
    SC_REPORT_ERROR(name(), p.get_response_string().c_str());
  }

  ems::req_extension *re;
  p.get_extension(re);
  assert(re->req == curr_req);

  if (re->last)
    all_trans_completed.notify();
}

void ddr_module::peq_callback(tlm::tlm_generic_payload &p, const tlm::tlm_phase &phase)
{
  if (phase == tlm::END_REQ || (&p == in_progress && phase == tlm::BEGIN_RESP)) {
    in_progress = NULL;
    end_req_event.notify();
  } else if (phase == tlm::BEGIN_REQ || phase == tlm::END_RESP) {
    SC_REPORT_FATAL(name(), " Illegal phase received by initiator");
  }

  if (phase == tlm::BEGIN_RESP) {
    inspect(p);
    // Send final phase transition to target
    tlm::tlm_phase ph = tlm::END_RESP;
    isocket->nb_transport_fw(p, ph, resp_accept_delay);
    // Allow the memory manager to free the transaction object
    p.release();
  }
}

// Backward non-blocking transport (from target side)
tlm::tlm_sync_enum ddr_module::nb_transport_bw(tlm::tlm_generic_payload &p, tlm::tlm_phase &phase, sc_core::sc_time &d)
{
  peq.notify(p, phase, d);
  return tlm::TLM_ACCEPTED;
}

void ddr_module::run()
{
  tlm::tlm_generic_payload *p;
  tlm::tlm_phase phase;
  sc_core::sc_time delay;
  tlm::tlm_sync_enum status;

  while (1) {
    while (vp_module->current_reqs == 0) {
      wait (event);
    }

    vp::io_req *req = vp_module->first_pending_reqs;
    vp_module->first_pending_reqs = vp_module->first_pending_reqs->get_next();

    curr_req = req;
    // A request corresponds to one or more transactions
    uint32_t n_trans = req->get_size() / bytes_per_access;
    assert(n_trans);

    for (auto t = 0; t < n_trans; t++) {
      // Get a generic payload from memory manager
      p = mm.palloc();
      p->acquire();
      // Convert packet to tlm generic payload
      bool last = (t == (n_trans - 1));
      req_to_gp(req, p, t, last);
      // Honor BEGIN_REQ/END_REQ exclusion rule
      if (in_progress)
        wait(end_req_event);
      // Non-blocking transport call on the forward path (send to target)
      in_progress = p;
      phase = tlm::BEGIN_REQ;
      delay = sc_core::SC_ZERO_TIME;
      status = isocket->nb_transport_fw(*p, phase, delay);

      switch (status) {
        case tlm::TLM_ACCEPTED:
          // Backward path is being used
          // Nothing to do, but expect an incoming call to nb_transport_bw
          break;
        case tlm::TLM_UPDATED:
          // Return path is being used
          // The callee may have updated phase and delay
          // Honor the timing annotation
          peq.notify(*p, phase, delay);
          break;
        case tlm::TLM_COMPLETED:
          // Return path is being used
          // Early completion
          // The target has terminated the transaction
          in_progress = NULL;
          inspect(*p);
          p->release();
          break;
        default:
          SC_REPORT_FATAL(name(), "Invalid status");
          break;
      }
    }

    wait(all_trans_completed);

    vp_module->current_reqs--;

    req->get_resp_port()->resp(req);

    if (vp_module->current_reqs >= vp_module->max_reqs) {
      vp::io_req *stalled_req = vp_module->first_stalled_req;
      vp_module->first_stalled_req = vp_module->first_stalled_req->get_next();
      stalled_req->get_resp_port()->grant(stalled_req);
    }
  }
}

