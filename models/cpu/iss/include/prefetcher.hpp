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

#ifndef __CPU_ISS_PREFETCHER_HPP
#define __CPU_ISS_PREFETCHER_HPP

#include "types.hpp"
#include <stdio.h>
#include "platform_wrapper.hpp"

static inline void prefetcher_init(iss_t *iss);
static inline iss_opcode_t prefetcher_get_word(iss_t *iss, iss_addr_t addr);
static inline iss_opcode_t prefetcher_fill(iss_t *iss, iss_addr_t addr);



static inline iss_opcode_t prefetcher_fill(iss_t *iss, iss_addr_t addr)
{
  iss_prefetcher_t *prefetcher = &iss->cpu.prefetcher;
  // TODO this is a temporary work-around until the vp can split all fetch requests
  // There is currently a bug when we fetch a line which is partly in bank1 and
  // partly in shared banks on wolfe
#if 0
  iss_fetch_req(iss, addr, prefetcher->data, ISS_PREFETCHER_SIZE, false);
#else
  uint32_t addr_low = addr & ~(ISS_PREFETCHER_SIZE/2-1);
  iss_fetch_req(iss, addr_low, prefetcher->data, ISS_PREFETCHER_SIZE/2, false);
  iss_fetch_req(iss, addr_low + ISS_PREFETCHER_SIZE/2, ((uint8_t *)prefetcher->data) + ISS_PREFETCHER_SIZE/2, ISS_PREFETCHER_SIZE/2, false);
#endif
  prefetcher->addr = addr_low;
}

static inline iss_opcode_t prefetcher_get_word(iss_t *iss, iss_addr_t addr)
{
  iss_prefetcher_t *prefetcher = &iss->cpu.prefetcher;

  int index = addr - prefetcher->addr;
  if (index< 0 || index + ISS_OPCODE_MAX_SIZE > ISS_PREFETCHER_SIZE)
  {
    prefetcher_fill(iss, addr);
    index = addr - prefetcher->addr;
  }

  return *(iss_opcode_t *)&prefetcher->data[index];
}

static inline void prefetcher_flush(iss_t *iss)
{
  iss->cpu.prefetcher.addr = -1;
}

static inline void prefetcher_init(iss_t *iss)
{
  prefetcher_flush(iss);
}

#endif
