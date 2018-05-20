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

#ifndef __VP_ITF_IO_HPP__
#define __VP_ITF_IO_HPP__

#include "vp/vp.hpp"

namespace vp {

  class io_slave;
  class io_req;

  typedef enum
  {
    IO_REQ_OK,
    IO_REQ_INVALID,
    IO_REQ_DENIED,
    IO_REQ_PENDING
  } io_req_status_e;

  #define IO_REQ_PAYLOAD_SIZE 64
  #define IO_REQ_NB_ARGS 8

  typedef io_req_status_e (io_req_meth_t)(void *, io_req *);
  typedef io_req_status_e (io_req_meth_muxed_t)(void *, io_req *, int id);

  typedef void (io_resp_meth_t)(void *, io_req *);
  typedef void (io_grant_meth_t)(void *, io_req *);

  class io_req
  {
    friend class io_master;
    friend class io_slave;

  public:
    io_req() {}

    io_req(uint64_t addr, uint8_t *data, uint64_t size, bool is_write)
    : addr(addr), data(data), size(size), is_write(is_write)
    {
    }

    io_slave *get_resp_port() { return resp_port;}
    void set_next(io_req *req) { next = req; }
    io_req *get_next() { return next; }

    uint64_t get_addr() { return addr; }
    void set_addr(uint64_t value) { addr = value; }

    uint64_t get_is_write() { return is_write; }
    void set_is_write(bool is_write) { this->is_write = is_write; }

    void set_size(uint64_t size) { this->size = size; }
    uint64_t get_size() { return size; }

    inline void set_latency(uint64_t latency) { this->latency = latency; }
    inline uint64_t get_latency() { return this->latency; }
    inline void inc_latency(uint64_t incr) { this->latency += incr; }

    inline void set_duration(uint64_t duration) { this->duration = duration; }
    inline uint64_t get_duration() { return this->duration; }

    inline uint64_t get_full_latency() { return latency + duration; }

    uint8_t *get_data() { return data; }
    void set_data(uint8_t *data) { this->data = data; }

    inline int get_payload_size() { return IO_REQ_PAYLOAD_SIZE; }
    inline uint8_t *get_payload() { return payload; }

    inline int get_nb_args() { return IO_REQ_NB_ARGS; }
    inline void **get_args() { return args; }

    inline void set_int(int index, int value) { *(int *)&get_args()[index] = value; }
    inline int get_int(int index) { return *(int *)&get_args()[index]; }

    inline bool is_debug() { return false; }

    inline int arg_alloc() { return current_arg++; }
    inline void arg_free() { current_arg--; }

    inline void **arg_get(int index) { return &args[index]; }
    inline void **arg_get_last() { return &args[current_arg]; }

    inline void init() { latency = 0; duration=0; }

    uint64_t addr;
    uint8_t *data;
    uint64_t size;
    bool is_write;
    io_req_status_e status;


  private:
    io_slave *resp_port;
    io_req *next;
    int64_t latency;
    int64_t duration;
    uint8_t payload[IO_REQ_PAYLOAD_SIZE];
    void *args[IO_REQ_NB_ARGS];
    int current_arg = 0;
  };

  class io_master : public vp::master_port
  {
    friend class io_slave;

  public:

    inline io_master();

    inline io_req_status_e req(io_req *req)
    {
      req->resp_port = slave_port;
      return req_meth((void *)comp, req);
    }

    inline io_req_status_e req_forward(io_req *req)
    {
      return req_meth((void *)comp, req);
    }

    inline io_req_status_e req(io_req *req, io_slave *slave_port);

    virtual inline void bind_to(vp::port *port, vp::config *config);

    inline void set_resp_meth(io_resp_meth_t *meth);
    inline void set_grant_meth(io_grant_meth_t *meth);

    inline io_req *req_new(uint64_t addr, uint8_t *data, uint64_t size, bool is_write);
    inline void req_del(io_req *req);

    bool is_bound() { return slave_port != NULL; }

  private:

    static inline io_req_status_e req_muxed(io_master *_this, io_req *req);
    void (*resp)(void *comp, io_req *req);
    void (*grant)(void *comp, io_req *req);
    io_req_status_e (*req_meth)(void *, io_req *);
    io_req_status_e (*req_meth_mux)(void *, io_req *, int mux);
    static inline void resp_default(void *, io_req *);
    static inline void grant_default(void *, io_req *);


    vp::component *comp_mux;
    int req_mux;
    io_slave *slave_port = NULL;
  };



  class io_slave : public vp::slave_port
  {

    friend class io_master;

  public:

    inline io_slave();

    inline void resp(io_req *req) { resp_meth((void *)comp, req); }
    inline void grant(io_req *req) { grant_meth((void *)comp, req); }

    inline void set_req_meth(io_req_meth_t *meth);
    inline void set_req_meth_muxed(io_req_meth_muxed_t *meth, int id);

    inline void bind_to(vp::port *_port, vp::config *config);

  private:

    void (*resp_meth)(void *, io_req *);
    void (*grant_meth)(void *, io_req *);
    io_req_status_e (*req)(void *comp, io_req *);
    io_req_status_e (*req_mux)(void *comp, io_req *, int mux);
    static inline io_req_status_e req_default(io_slave *, io_req *);

    int req_mux_id;


  };

  inline io_req_status_e io_master::req(io_req *req, io_slave *port)
  {
    req->resp_port = port;
    return port->req((void *)port->get_comp(), req);
  }


  inline void io_slave::bind_to(vp::port *_port, vp::config *config)
  {
    slave_port::bind_to(_port, config);
    io_master *port = (io_master *)_port;
    port->slave_port = new io_slave();
    port->slave_port->resp_meth = port->resp;
    port->slave_port->grant_meth = port->grant;
    port->slave_port->set_comp(port->get_comp());
    port->slave_port->comp = port->get_comp();
  }


  inline io_master::io_master() {
    resp = &io_master::resp_default;
    grant = &io_master::grant_default;
  }

  inline io_req *io_master::req_new(uint64_t addr, uint8_t *data, uint64_t size, bool is_write)
  {
    return new io_req(addr, data, size, is_write);
  }

  inline void io_master::req_del(io_req *req)
  {
    delete req;
  }

  inline io_req_status_e io_master::req_muxed(io_master *_this, io_req *req)
  {
    return _this->req_meth_mux(_this->comp_mux, req, _this->req_mux);
  }


  inline void io_master::bind_to(vp::port *_port, vp::config *config)
  {
    io_slave *port = (io_slave *)_port;
    if (port->req_mux == NULL)
    {
      req_meth = port->req;
      comp = (vp::component *)port->get_comp();
    }
    else
    {
      req_meth_mux = port->req_mux;
      req_meth = (io_req_meth_t *)&io_master::req_muxed;
      comp = (vp::component *)this;
      comp_mux = (vp::component *)port->get_comp();
      req_mux = port->req_mux_id;
    }
  }

  inline void io_master::set_resp_meth(io_resp_meth_t *meth)
  {
    resp = meth;
  }

  inline void io_master::set_grant_meth(io_grant_meth_t *meth)
  {
    grant = meth;
  }

  inline void io_master::resp_default(void *, io_req *)
  {
  }

  inline void io_master::grant_default(void *, io_req *)
  {
  }



  inline io_slave::io_slave() : req(NULL), req_mux(NULL) {
    req = (io_req_meth_t *)&io_slave::req_default;
  }

  inline void io_slave::set_req_meth(io_req_meth_t *meth)
  {
    req = meth;
    req_mux = NULL;
  }

  inline void io_slave::set_req_meth_muxed(io_req_meth_muxed_t *meth, int id)
  {
    req_mux = meth;
    req = NULL;
    req_mux_id = id;
  }

  inline io_req_status_e io_slave::req_default(io_slave *, io_req *)
  {
  }



};

#endif
