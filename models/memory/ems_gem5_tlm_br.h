/* vim: set ts=2 sw=2 expandtab: */
/*
 * Copyright (C) 2019 TU Kaiserslautern
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
 * Author: Éder F. Zulian, TUK (zulian@eit.uni-kl.de)
 */

#ifndef __EMS_GEM5_TLM_BR_H__
#define __EMS_GEM5_TLM_BR_H__

#include <regex>
#include <iostream>
#include <fstream>

#include <tlm.h>

#include "ems_common.h"

#include "report_handler.hh"
#include "sc_target.hh"
#include "sim_control.hh"
#include "slave_transactor.hh"
#include "stats.hh"

namespace ems {

class gem5_tlm_br
{
public:
  gem5_tlm_br(std::string name, std::string cfg, unsigned int nports)
  {
    sctrl = new gem5_sim_ctrl(name, cfg);
    Gem5SystemC::Gem5SlaveTransactor *t;
    std::string line;
    std::ifstream file(cfg);

    if (!file.is_open()) {
      debug("Unable to open file" << cfg);
      throw std::logic_error("Unable to open file");
    }

    std::regex rgx("port_data=(\\w+)");
    std::smatch match;
    while (std::getline(file, line)) {
      if (std::regex_search(line, match, rgx)) {
        t = new Gem5SystemC::Gem5SlaveTransactor(match.str(1).c_str(), match.str(1).c_str());
        t->sim_control.bind(*sctrl);
        transactors.push_back(t);
      }
    }
    file.close();
    if (match.empty()) {
      debug("No matches found for regex");
      throw std::logic_error("No matches found for regex");
    }
  }

  ~gem5_tlm_br()
  {
    delete sctrl;
    for (auto t : transactors) {
       delete t;
    }
  }

  // External bindings
  std::vector<Gem5SystemC::Gem5SlaveTransactor *> transactors;

private:
  class gem5_sim_ctrl : public Gem5SystemC::Gem5SimControl
  {
  public:
    // name:  SystemC module name
    // g5cfg: gem5 configuration file
    // ticks: zero to simulate until workload is finished, otherwise the
    //        specified amount of time
    // g5dbg: gem5 debug flags
    gem5_sim_ctrl(std::string name, std::string g5cfg,
                  uint64_t ticks=0, std::string g5dbg="MemoryAccess") :
      Gem5SystemC::Gem5SimControl(name.c_str(), g5cfg, ticks, g5dbg)
    {
      debug(name << " config file: " << g5cfg);
      debug(name << " ticks: " << ticks);
      debug(name << " debug flags: " << g5dbg);
    }

    void afterSimulate()
    {
      debug("gem5 simulation finished");
    }
  };

  gem5_sim_ctrl *sctrl;
};

} // namespace ems

#endif /* __EMS_GEM5_TLM_BR_H__ */

