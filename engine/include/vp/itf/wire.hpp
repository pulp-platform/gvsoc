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

#ifndef __VP_ITF_WIRE_HPP__
#define __VP_ITF_WIRE_HPP__

#include "vp/vp.hpp"

namespace vp {

  class component;

  template<class T>
  class wire_slave;

  template<class T>
  class wire_master : public master_port
  {
    friend class wire_slave<T>;
  public:

    wire_master() {}

    inline void sync(T value)
    {
      if (next) next->sync(value);
      sync_meth((void *)comp, value);
    }

    inline void sync_back(T *value)
    {
      if (next) next->sync_back(value);
      sync_back_meth((void *)comp, value);
    }

    void bind_to(cm::port *port, cm::config *config);

    bool is_bound() { return slave_port != NULL; }

  private:
    static inline void sync_muxed(wire_master *_this, T value);
    static inline void sync_back_muxed(wire_master *_this, T *value);
    void (*sync_meth)(void *, T value);
    void (*sync_meth_mux)(void *, T value, int id);
    void (*sync_back_meth)(void *, T *value);
    void (*sync_back_meth_mux)(void *, T *value, int id);

    vp::component *comp_mux;
    int sync_mux;
    int sync_back_mux;
    wire_slave<T> *slave_port = NULL;
    wire_master<T> *next = NULL;

  };



  template<class T>
  class wire_slave : public slave_port
  {

    friend class wire_master<T>;

  public:

    inline wire_slave();

    void set_sync_meth(void (*)(void *_this, T value));
    void set_sync_meth_muxed(void (*)(void *_this, T value, int), int id);

    void set_sync_back_meth(void (*)(void *_this, T *value));
    void set_sync_back_meth_muxed(void (*)(void *_this, T *value, int), int id);

    inline void bind_to(cm::port *_port, cm::config *config);


  private:

    void (*sync)(void *comp, T value);
    void (*sync_mux)(void *comp, T value, int id);

    void (*sync_back)(void *comp, T *value);
    void (*sync_back_mux)(void *comp, T *value, int id);

    int sync_mux_id;

    int sync_back_mux_id;

  };



  template<class T>
  inline void wire_master<T>::bind_to(cm::port *_port, cm::config *config)
  {
    if (slave_port != NULL)
    {
      wire_master<T> *master = new wire_master<T>;
      master->bind_to(_port, config);
      master->next = this->next;
      this->next = master;
    }
    else
    {
      wire_slave<T> *port = (wire_slave<T> *)_port;
      if (port->sync_mux == NULL)
      {
        sync_meth = port->sync;
        comp = (vp::component *)port->get_comp();
      }
      else
      {
        sync_meth_mux = port->sync_mux;
        sync_meth = (void (*)(void *, T))&wire_master::sync_muxed;
        comp = (vp::component *)this;
        comp_mux = (vp::component *)port->get_comp();
        sync_mux = port->sync_mux_id;
      }
      if (port->sync_back_mux == NULL)
      {
        sync_back_meth = port->sync_back;
        comp = (vp::component *)port->get_comp();
      }
      else
      {
        sync_back_meth_mux = port->sync_back_mux;
        sync_back_meth = (void (*)(void *, T *))&wire_master::sync_back_muxed;
        comp = (vp::component *)this;
        comp_mux = (vp::component *)port->get_comp();
        sync_back_mux = port->sync_back_mux_id;
      }
    }
  }

  template<class T>
  inline void wire_master<T>::sync_muxed(wire_master<T> *_this, T value)
  {
    return _this->sync_meth_mux(_this->comp_mux, value, _this->sync_mux);
  }

  template<class T>
  inline void wire_master<T>::sync_back_muxed(wire_master<T> *_this, T *value)
  {
    return _this->sync_back_meth_mux(_this->comp_mux, value, _this->sync_back_mux);
  }




  template<class T>
  inline void wire_slave<T>::bind_to(cm::port *_port, cm::config *config)
  {
    slave_port::bind_to(_port, config);
    wire_master<T> *port = (wire_master<T> *)_port;

    if (port->next) port = port->next;

    port->slave_port = new wire_slave();
    port->slave_port->set_comp(port->get_comp());
    port->slave_port->comp = port->get_comp();
  }

  template<class T>
  inline void wire_slave<T>::set_sync_meth(void (*meth)(void *, T))
  {
    sync = meth;
    sync_mux = NULL;
  }

  template<class T>
  inline void wire_slave<T>::set_sync_meth_muxed(void (*meth)(void *, T, int), int id)
  {
    sync = NULL;
    sync_mux = meth;
    sync_mux_id = id;
  }

  template<class T>
  inline wire_slave<T>::wire_slave() : sync(NULL), sync_mux(NULL), sync_back(NULL), sync_back_mux(NULL) {
  }

  template<class T>
  inline void wire_slave<T>::set_sync_back_meth(void (*meth)(void *, T *))
  {
    sync_back = meth;
    sync_back_mux = NULL;
  }

  template<class T>
  inline void wire_slave<T>::set_sync_back_meth_muxed(void (*meth)(void *, T *, int), int id)
  {
    sync_back = NULL;
    sync_back_mux = meth;
    sync_back_mux_id = id;
  }

};

#endif
