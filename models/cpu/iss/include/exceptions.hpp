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

#ifndef __CPU_ISS_EXCEPTIONS_HPP
#define __CPU_ISS_EXCEPTIONS_HPP

static inline iss_insn_t *iss_except_raise(iss_t *iss, int id)
{
  iss->cpu.csr.epc = iss->cpu.current_insn->addr;
  iss->cpu.irq.saved_irq_enable = iss->cpu.irq.irq_enable;
  iss->cpu.irq.irq_enable = 0;
  iss->cpu.csr.mcause = 0xb;
  return iss->cpu.irq.vectors[32 + id];
}

#endif