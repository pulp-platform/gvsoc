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

#ifndef __VP_COMP_MODEL_PORTS_HPP__
#define __VP_COMP_MODEL_PORTS_HPP__

namespace cm {

  class port
  {

  public:

    virtual void bind_to(port *port, cm::config *config);

    void set_comp(cm::component *comp) { this->owner = comp; }
    cm::component *get_comp() { return owner; }

  protected:
    component *comp;

    component *owner;

  };

  class master_port : public port
  {
  public:

  };


  class slave_port : public port
  {
  public:


  };

  inline void port::bind_to(port *port, cm::config *config)
  {
    comp = (component *)port->get_comp();
  }

};

#endif
