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

extern iss_isa_set_t __iss_isa_set;

static int decode_item(iss *iss, iss_insn_t *insn, iss_opcode_t opcode, iss_decoder_item_t *item);

static uint64_t decode_ranges(iss *iss, iss_opcode_t opcode, iss_decoder_range_set_t *range_set, bool is_signed)
{
  int nb_ranges = range_set->nb_ranges;
  iss_decoder_range_t *ranges = range_set->ranges;;
  uint64_t result = 0;
  int bits = 0;
  for (int i=0; i<nb_ranges; i++)
  {
    iss_decoder_range_t *range = &ranges[i];
    result |= iss_get_field(opcode, range->bit, range->width) << range->shift;
    int last_bit = range->width + range->shift;
    if (last_bit > bits) bits = last_bit;
  }
  if (is_signed) result = iss_get_signed_value(result, bits);
  return result;
}


static int decode_info(iss *iss, iss_insn_t *insn, iss_opcode_t opcode, iss_decoder_arg_info_t *info, bool is_signed)
{
  if (info->type == ISS_DECODER_VALUE_TYPE_RANGE)
  {
    return decode_ranges(iss, opcode, &info->u.range_set, is_signed);
  }
  else if (info->type == ISS_DECODER_VALUE_TYPE_UIM)
  {
    return info->u.uim;
  }
  else if (info->type == ISS_DECODER_VALUE_TYPE_SIM)
  {
    return info->u.sim;
  }

  return 0;
}

static int decode_insn(iss *iss, iss_insn_t *insn, iss_opcode_t opcode, iss_decoder_item_t *item)
{
  insn->hwloop_handler = NULL;
  insn->handler = item->u.insn.handler;
  insn->decoder_item = item;
  insn->size = item->u.insn.size;

  for (int i=0; i<item->u.insn.nb_args; i++)
  {
    iss_decoder_arg_t *darg = &item->u.insn.args[i];
    iss_insn_arg_t *arg = &insn->args[i];
    arg->type = darg->type;
    arg->flags = darg->flags;

    switch (darg->type)
    {
      case ISS_DECODER_ARG_TYPE_IN_REG:
      case ISS_DECODER_ARG_TYPE_OUT_REG:
        arg->u.reg.index = decode_info(iss, insn, opcode, &darg->u.reg.info, false);
        if (darg->flags & ISS_DECODER_ARG_FLAG_COMPRESSED) arg->u.reg.index += 8;
        if (darg->flags & ISS_DECODER_ARG_FLAG_FREG) {
          arg->u.reg.index += ISS_NB_REGS;
        }
        if (darg->type == ISS_DECODER_ARG_TYPE_IN_REG) {
          insn->in_regs[darg->u.reg.id] = arg->u.reg.index;
        }
        else {
         insn->out_regs[darg->u.reg.id] = arg->u.reg.index;
        }
        break;

      case ISS_DECODER_ARG_TYPE_UIMM:
        arg->u.uim.value = decode_ranges(iss, opcode, &darg->u.uimm.info.u.range_set, darg->u.uimm.is_signed);
        insn->uim[darg->u.uimm.id] = arg->u.uim.value;
        break;

      case ISS_DECODER_ARG_TYPE_SIMM:
        arg->u.sim.value = decode_ranges(iss, opcode, &darg->u.simm.info.u.range_set, darg->u.simm.is_signed);
        insn->sim[darg->u.simm.id] = arg->u.sim.value;
        break;

      case ISS_DECODER_ARG_TYPE_INDIRECT_IMM:
        arg->u.indirect_imm.reg_index = decode_info(iss, insn, opcode, &darg->u.indirect_imm.reg.info, false);
        if (darg->u.indirect_imm.reg.flags & ISS_DECODER_ARG_FLAG_COMPRESSED) arg->u.indirect_imm.reg_index += 8;
        insn->in_regs[darg->u.indirect_imm.reg.id] = arg->u.indirect_imm.reg_index;
        arg->u.indirect_imm.imm = decode_info(iss, insn, opcode, &darg->u.indirect_imm.imm.info, darg->u.indirect_imm.imm.is_signed);
        insn->sim[darg->u.indirect_imm.imm.id] = arg->u.indirect_imm.imm;
        break;

      case ISS_DECODER_ARG_TYPE_INDIRECT_REG:
        arg->u.indirect_reg.base_reg_index = decode_info(iss, insn, opcode, &darg->u.indirect_reg.base_reg.info, false);
        if (darg->u.indirect_reg.base_reg.flags & ISS_DECODER_ARG_FLAG_COMPRESSED) arg->u.indirect_reg.base_reg_index += 8;
        insn->in_regs[darg->u.indirect_reg.base_reg.id] = arg->u.indirect_reg.base_reg_index;

        arg->u.indirect_reg.offset_reg_index = decode_info(iss, insn, opcode, &darg->u.indirect_reg.offset_reg.info, false);
        if (darg->u.indirect_reg.offset_reg.flags & ISS_DECODER_ARG_FLAG_COMPRESSED) arg->u.indirect_reg.offset_reg_index += 8;
        insn->in_regs[darg->u.indirect_reg.offset_reg.id] = arg->u.indirect_reg.offset_reg_index;

        break;
    }
  }

  insn->next = insn_cache_get(iss, insn->addr + insn->size);

  if (item->u.insn.decode != NULL)
  {
    item->u.insn.decode(iss, insn);
  }

  return 0;
}

static int decode_opcode_group(iss *iss, iss_insn_t *insn, iss_opcode_t opcode, iss_decoder_item_t *item)
{
  iss_opcode_t group_opcode = (opcode >> item->u.group.bit) & ((1ULL << item->u.group.width) - 1);
  iss_decoder_item_t *group_item_other = NULL;

  for (int i=0; i<item->u.group.nb_groups; i++)
  {
    iss_decoder_item_t *group_item = item->u.group.groups[i];
    if (group_opcode == group_item->opcode && !group_item->opcode_others) return decode_item(iss, insn, opcode, group_item);
    if (group_item->opcode_others) group_item_other = group_item;
  }

  if (group_item_other) return decode_item(iss, insn, opcode, group_item_other);

  return -1;
}

static int decode_item(iss *iss, iss_insn_t *insn, iss_opcode_t opcode, iss_decoder_item_t *item)
{
  if (item->is_insn) return decode_insn(iss, insn, opcode, item);
  else return decode_opcode_group(iss, insn, opcode, item);
}

static int decode_opcode(iss *iss, iss_insn_t *insn, iss_opcode_t opcode)
{
  for (int i=0; i<__iss_isa_set.nb_isa; i++)
  {
    iss_isa_t *isa = &__iss_isa_set.isa_set[i];
    if (decode_item(iss, insn, opcode, isa->tree) == 0) return 0;
  }

  iss->decode_trace.msg("Unknown instruction\n");

  return -1;
}

static iss_insn_t *iss_exec_insn_illegal(iss *iss, iss_insn_t *insn)
{
  iss->decode_trace.msg("Executing illegal instruction\n");
  return iss_except_raise(iss, ISS_EXCEPT_ILLEGAL);
}

iss_insn_t *iss_decode_pc_noexec(iss *iss, iss_insn_t *insn)
{
  iss->decode_trace.msg("Decoding instruction (pc: 0x%lx)\n", insn->addr);

  iss_opcode_t opcode = prefetcher_get_word(iss, insn->addr);

  iss->decode_trace.msg("Got opcode (opcode: 0x%lx)\n", opcode);

  if (decode_opcode(iss, insn, opcode) == -1)
  {
    insn->handler = iss_exec_insn_illegal;
    return insn;
  }


  if (iss->insn_trace.get_active())
  {
    insn->saved_handler = insn->handler;
    insn->handler = iss_exec_insn_with_trace;
  }

  return insn;
}

iss_insn_t *iss_decode_pc(iss *iss, iss_insn_t *insn)
{
  return iss_exec_insn(iss, iss_decode_pc_noexec(iss, insn));
}
