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

#ifndef __CPU_ISS_IRQ_HPP
#define __CPU_ISS_IRQ_HPP

static inline void iss_irq_check(iss_t *iss)
{
  int req_irq = iss->cpu.irq.req_irq;
  if (req_irq != -1 && iss->cpu.irq.irq_enable)
  {
    iss->cpu.csr.epc = iss->cpu.current_insn->addr;
    iss->cpu.irq.saved_irq_enable = iss->cpu.irq.irq_enable;
    iss->cpu.irq.irq_enable = 0;
    iss->cpu.current_insn = iss->cpu.irq.vectors[req_irq];
    iss_irq_ack(iss, req_irq);
  }
}

static inline iss_insn_t *iss_irq_handle_mret(iss_t *iss)
{
  iss->cpu.irq.irq_enable = iss->cpu.irq.saved_irq_enable;
  return insn_cache_get(iss, iss->cpu.csr.epc);

}

static inline void iss_irq_enable(iss_t *iss, int enable)
{
  iss->cpu.irq.irq_enable = enable;
  iss->irq_check();
}

static inline void iss_irq_req(iss_t *iss, int irq)
{
  iss->cpu.irq.req_irq = irq;
}

static inline void iss_irq_set_vector_table(iss_t *iss, iss_addr_t base)
{
  iss->trace.msg("Setting vector table (addr: 0x%x)\n", base);
  for (int i=0; i<32; i++)
  {
    iss->cpu.irq.vectors[i] = insn_cache_get(iss, base + i * 4);
  }

  for (int i=32; i<35; i++)
  {
    iss->cpu.irq.vectors[i] = insn_cache_get(iss, base + i * 4);
  }
}

static inline void iss_irq_init(iss_t *iss)
{
  iss->cpu.irq.irq_enable = 0;
  iss->cpu.irq.req_irq = -1;
}

#endif
