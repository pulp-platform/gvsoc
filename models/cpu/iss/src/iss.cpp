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

static int iss_parse_isa(iss_t *iss)
{
  const char *current = iss->cpu.config.isa;
  int len = strlen(current);

  if (strncmp(current, "rv32", 4) == 0)
  {
    current += 4;
    len -= 4;
  }
  else
  {
    iss->trace.warning("Unsupported ISA: %s\n", current);
    return -1;
  }

  while (len)
  {
    switch (*current)
    {
      case 'x':
        if (strncmp(current, "xpulpv2", 7) == 0)
        {
          iss_isa_pulpv2_activate(iss);
          current += 6;
          len -= 6;
        }
        break;
      case 'i':
      case 'm':
      case 'c':
      case 'f':
      case 'd':
        break;
      default:
        iss->trace.warning("Unknwon ISA descriptor: %c\n", *current);
        return -1;
    }
    current++;
    len--;
  }

  return 0;
}

int iss_open(iss_t *iss)
{
  iss_isa_pulpv2_init(iss);

  if (iss_parse_isa(iss)) return -1;

  insn_cache_init(iss);
  prefetcher_init(iss);

  iss->cpu.regfile.regs[0] = 0;
  iss->cpu.prev_insn = NULL;
  iss->cpu.state.hw_counter_en = 0;

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
