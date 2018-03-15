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

#include "iss.hpp"

int iss_open(iss_t *iss)
{
  insn_cache_init(iss);
  prefetcher_init(iss);

  iss->cpu.regfile.regs[0] = 0;

  iss->cpu.state.hwloop_count[0] = 0;
  iss->cpu.state.hwloop_count[1] = 0;

  iss_irq_init(iss);

  iss_csr_init(iss);

  iss->io_req.set_data(new uint8_t[sizeof(iss_reg_t)]);

  return 0;
}



void iss_start(iss_t *iss)
{
}



void iss_pc_set(iss_t *iss, iss_addr_t value)
{
  iss->cpu.current_insn = insn_cache_get(iss, value);
}
