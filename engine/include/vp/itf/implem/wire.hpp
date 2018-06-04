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

#ifndef __VP_ITF_IMPLEMEN_WIRE_HPP__
#define __VP_ITF_IMPLEMEN_WIRE_HPP__

namespace vp {

  template<class T>
  inline void wire_master<T>::bind_to(vp::port *_port, vp::config *config)
  {
    this->remote_port = _port;

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
      if (port->sync_mux == NULL && port->sync_back_mux == NULL)
      {
        sync_back_meth = port->sync_back;
        sync_meth = port->sync;
        set_remote_context(port->get_context());
      }
      else
      {
        sync_meth_mux = port->sync_mux;
        sync_back_meth_mux = port->sync_back_mux;
        sync_meth = (void (*)(void *, T))&wire_master::sync_muxed;
        sync_back_meth = (void (*)(void *, T *))&wire_master::sync_back_muxed;
        set_remote_context(this);
        comp_mux = (vp::component *)port->get_context();
        sync_mux = port->sync_mux_id;
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
  inline void wire_master<T>::sync_freq_cross_stub(wire_master<T> *_this, T value)
  {
    // The normal callback was tweaked in order to get there when the master is sending a
    // request. 
    // First synchronize the target engine in case it was left behind,
    // and then generate the normal call with the mux ID using the saved handler
    _this->remote_port->get_owner()->get_clock()->sync();
    return _this->sync_meth_freq_cross((component *)_this->slave_context_for_freq_cross, value);
  }

  template<class T>
  inline void wire_master<T>::sync_back_freq_cross_stub(wire_master<T> *_this, T *value)
  {
    // The normal callback was tweaked in order to get there when the master is sending a
    // request. 
    // First synchronize the target engine in case it was left behind,
    // and then generate the normal call with the mux ID using the saved handler
    _this->remote_port->get_owner()->get_clock()->sync();
    return _this->sync_back_meth_freq_cross((component *)_this->slave_context_for_freq_cross, value);
  }

  template<class T>
  inline void wire_master<T>::finalize()
  {
    // We have to instantiate a stub in case the binding is crossing different
    // frequency domains in order to resynchronize the target engine.
    if (this->get_owner()->get_clock() != this->remote_port->get_owner()->get_clock())
    {
      // Just save the normal handler and tweak it to enter the stub when the
      // master is pushing the request.
      this->sync_meth_freq_cross = this->sync_meth;
      this->sync_meth = (void (*)(void *, T))&wire_master<T>::sync_freq_cross_stub;

      this->sync_back_meth_freq_cross = this->sync_back_meth;
      this->sync_back_meth = (void (*)(void *, T *))&wire_master<T>::sync_back_freq_cross_stub;

      this->slave_context_for_freq_cross = this->get_remote_context();
      this->set_remote_context(this);
    }
  }



  template<class T>
  inline void wire_slave<T>::bind_to(vp::port *_port, vp::config *config)
  {
    slave_port::bind_to(_port, config);
    wire_master<T> *port = (wire_master<T> *)_port;

    if (port->next) port = port->next;

    port->slave_port = new wire_slave();
    port->slave_port->set_context(port->get_context());
    port->slave_port->set_remote_context(port->get_context());
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