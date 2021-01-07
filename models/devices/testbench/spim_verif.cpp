/*
 * Copyright (C) 2020 GreenWaves Technologies, SAS, ETH Zurich and
 *                    University of Bologna
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
 * Authors: Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */

#include <vp/vp.hpp>
#include "spim_verif.hpp"

Spim_verif::Spim_verif(Testbench *top, vp::qspim_slave *itf, int itf_id, int cs, int mem_size)
{
    this->itf = itf;
    this->mem_size = mem_size;
    verbose = true; //config->get("verbose")->get_bool();
    //print("Creating SPIM VERIF model (mem_size: 0x%x)", this->mem_size);
    data = new unsigned char[mem_size];

    wait_cs = false;
    this->current_cs = 1;
    this->tx_file = NULL;

    top->traces.new_trace("spim_verif_itf" + std::to_string(itf_id) + "_cs" + std::to_string(cs), &trace, vp::DEBUG);

#if 0
  js::config *tx_file_config = config->get("tx_file");
  if (tx_file_config != NULL)
  {
    js::config *path_config = tx_file_config->get("path");
    js::config *qpi_config = tx_file_config->get("qpi");
    this->tx_dump_bits = 0;
    this->tx_file = fopen(path_config->get_str().c_str(), "wb");
    this->tx_dump_qpi = qpi_config != NULL && qpi_config->get_bool();
  }
#endif

    //this->trace = this->trace_new(config->get_child_str("name").c_str());
}

void Spim_verif::handle_read(uint64_t cmd, bool is_quad)
{
    this->is_quad = is_quad;
    int size = SPIM_VERIF_FIELD_GET(cmd, SPIM_VERIF_CMD_INFO_BIT, SPIM_VERIF_CMD_INFO_WIDTH);
    current_addr = SPIM_VERIF_FIELD_GET(cmd, SPIM_VERIF_CMD_ADDR_BIT, SPIM_VERIF_CMD_ADDR_WIDTH);
    command_addr = current_addr;

    this->trace.msg(vp::trace::LEVEL_INFO, "Handling read command (size: 0x%x, addr: 0x%x, is_quad: %d)\n", size, current_addr, is_quad);

    state = STATE_READ_CMD;
    current_size = size;
    nb_bits = 0;
    wait_cs = true;
    //dummy_cycles = 1;
}

void Spim_verif::handle_write(uint64_t cmd, bool is_quad)
{
    this->is_quad = is_quad;
    int size = SPIM_VERIF_FIELD_GET(cmd, SPIM_VERIF_CMD_INFO_BIT, SPIM_VERIF_CMD_INFO_WIDTH);
    current_write_addr = SPIM_VERIF_FIELD_GET(cmd, SPIM_VERIF_CMD_ADDR_BIT, SPIM_VERIF_CMD_ADDR_WIDTH);
    command_addr = current_write_addr;

    this->trace.msg(vp::trace::LEVEL_INFO, "Handling write command (size: 0x%x, addr: 0x%x, is_quad: %d)\n", size, current_write_addr, is_quad);

    state = STATE_WRITE_CMD;
    current_write_size = size;
    nb_write_bits = 0;
}

void Spim_verif::handle_full_duplex(uint64_t cmd)
{
    int size = SPIM_VERIF_FIELD_GET(cmd, SPIM_VERIF_CMD_INFO_BIT, SPIM_VERIF_CMD_INFO_WIDTH);
    current_addr = SPIM_VERIF_FIELD_GET(cmd, SPIM_VERIF_CMD_ADDR_BIT, SPIM_VERIF_CMD_ADDR_WIDTH);
    command_addr = current_addr;

    this->trace.msg(vp::trace::LEVEL_INFO, "Handling full duplex command (size: 0x%x, addr: 0x%x, is_quad: %d)\n", size, current_addr, is_quad);

    state = STATE_FULL_DUPLEX_CMD;
    current_write_addr = current_addr;
    current_size = size;
    current_write_size = size;
    nb_bits = 0;
    nb_write_bits = 0;
    wait_cs = true;
}

void Spim_verif::exec_read()
{
    if (dummy_cycles)
    {
        dummy_cycles--;
        return;
    }

    if (nb_bits == 0)
    {
        if (this->current_addr >= this->mem_size)
        {
            //this->fatal("Trying to read outside memory range (addr: 0x%x, mem size: 0x%x)\n", this->current_addr, this->mem_size);
        }
        else
        {
            byte = data[current_addr];
            this->trace.msg(vp::trace::LEVEL_DEBUG, "Read byte from memory (value: 0x%x, rem_size: 0x%x)\n", byte, current_size);
        }
        nb_bits = 8;
        current_addr++;
    }

    if (this->is_quad)
    {
        int data_0 = (byte >> 4) & 1;
        int data_1 = (byte >> 5) & 1;
        int data_2 = (byte >> 6) & 1;
        int data_3 = (byte >> 7) & 1;

        byte <<= 4;

        this->itf->sync(data_0, data_1, data_2, data_3, 0xf);

        nb_bits -= 4;
        current_size -= 4;
    }
    else
    {
        int bit = (byte >> 7) & 1;
        byte <<= 1;

        this->itf->sync(2, bit, 2, 2, 0x2);

        nb_bits--;
        current_size--;
    }
    if (current_size == 0)
    {
        if (state == STATE_READ_CMD)
        {
            wait_cs = true;
            state = STATE_GET_CMD;
        }
    }
}

void Spim_verif::exec_write(int sdio0, int sdio1, int sdio2, int sdio3)
{
    if (this->is_quad)
    {
        pending_write = (pending_write << 4) | (sdio3 << 3) | (sdio2 << 2) | (sdio1 << 1) | (sdio0 << 0);;
        nb_write_bits +=4;
        current_write_size -= 4;
    }
    else
    {
        pending_write = (pending_write << 1) | sdio0;
        nb_write_bits++;
        current_write_size--;
    }
    if (nb_write_bits == 8)
    {
        if (this->current_write_addr >= this->mem_size)
        {
            //this->fatal("Trying to write outside memory range (addr: 0x%x, mem size: 0x%x)\n", this->current_write_addr, this->mem_size);
        }
        else
        {
            data[current_write_addr] = pending_write;

            this->trace.msg(vp::trace::LEVEL_DEBUG, "Wrote byte to memory (addr: 0x%x, value: 0x%x, rem_size: 0x%x)\n", current_write_addr, data[current_write_addr], current_write_size-1);
        }

        nb_write_bits = 0;
        current_write_addr++;
    }

    if (current_write_size == 0)
    {
        if (nb_write_bits != 0)
        {
            int shift = 8 - nb_write_bits;
            pending_write = (data[current_write_addr] & ~((1 << shift) - 1)) | (pending_write << shift);
            this->trace.msg(vp::trace::LEVEL_DEBUG, "Wrote byte to memory (value: 0x%x)\n", data[current_write_addr]);
        }
        wait_cs = true;
        state = STATE_GET_CMD;
    }
}

void Spim_verif::handle_command(uint64_t cmd)
{
   this->trace.msg(vp::trace::LEVEL_INFO, "Handling command %x\n", current_cmd);

    int cmd_id = SPIM_VERIF_FIELD_GET(cmd, SPIM_VERIF_CMD_BIT, SPIM_VERIF_CMD_WIDTH);

    switch (cmd_id)
    {
    case SPIM_VERIF_CMD_WRITE:
        handle_write(cmd, false);
        break;
    case SPIM_VERIF_CMD_READ:
        handle_read(cmd, false);
        break;
    case SPIM_VERIF_CMD_FULL_DUPLEX:
        handle_full_duplex(cmd);
        break;
    case SPIM_VERIF_CMD_WRITE_QUAD:
        handle_write(cmd, true);
        break;
    case SPIM_VERIF_CMD_READ_QUAD:
        handle_read(cmd, true);
        break;
    default: //print("WARNING: received unknown command (raw: 0x%llx, id: 0x%x)", cmd, cmd_id);
        break;
    }
}

void Spim_verif::cs_sync(int cs)
{
    if (this->current_cs == cs)
        return;

    this->current_cs = cs;

    this->trace.msg(vp::trace::LEVEL_DEBUG, "CS edge (cs: %d)\n", cs);
    if (cs == 1)
    {
        // Reset pending addresses to detect CS edge during command
        current_write_addr = command_addr;
        current_addr = command_addr;

        this->wait_cs = false;
        this->itf->sync(2, 2, 2, 2, 0x1);
    }

    if (cs == 0)
    {
        if (state == STATE_READ_CMD || state == STATE_FULL_DUPLEX_CMD)
        {
            exec_read();
        }
    }
}

void Spim_verif::exec_dump_single(int sdio0)
{
    this->tx_dump_byte = (this->tx_dump_byte << 1) | sdio0;
    this->tx_dump_bits++;
    if (this->tx_dump_bits == 8)
    {
        this->tx_dump_bits = 0;
        fwrite((void *)&this->tx_dump_byte, 1, 1, this->tx_file);
    }
}

void Spim_verif::exec_dump_qpi(int sdio0, int sdio1, int sdio2, int sdio3)
{
    this->tx_dump_byte = (this->tx_dump_byte << 4) | (sdio3 << 3) | (sdio2 << 2) | (sdio1 << 1) | sdio0;
    this->tx_dump_bits += 4;
    if (this->tx_dump_bits == 8)
    {
        this->tx_dump_bits = 0;
        fwrite((void *)&this->tx_dump_byte, 1, 1, this->tx_file);
    }
}

void Spim_verif::handle_clk_high(int sdio0, int sdio1, int sdio2, int sdio3, int mask)
{
    if (wait_cs)
        return;

    if (this->tx_file != NULL)
    {
        if (this->tx_dump_qpi)
            exec_dump_qpi(sdio0, sdio1, sdio2, sdio3);
        else
            exec_dump_single(sdio0);
        // TODO properly destroy the model and close the logfile instead of flushing
        fflush(NULL);
    }

    if (state == STATE_GET_CMD)
    {
        current_cmd = (current_cmd << 1) | sdio0;
        this->trace.msg(vp::trace::LEVEL_DEBUG, "Received command bit (count: %d, pending: %x, bit: %d)\n", cmd_count, current_cmd, sdio0);
        cmd_count++;
        if (cmd_count == 64)
        {
            cmd_count = 0;
            handle_command(current_cmd);
        }
    }
    else if (state == STATE_WRITE_CMD || state == STATE_FULL_DUPLEX_CMD)
    {
        exec_write(sdio0, sdio1, sdio2, sdio3);
    }
}

void Spim_verif::handle_clk_low(int sdio0, int sdio1, int sdio2, int sdio3, int mask)
{
    if (wait_cs)
        return;

    if (state == STATE_READ_CMD || state == STATE_FULL_DUPLEX_CMD)
    {
        exec_read();
    }
}

void Spim_verif::sync(int sck, int sdio0, int sdio1, int sdio2, int sdio3, int mask)
{
    this->trace.msg(vp::trace::LEVEL_TRACE, "SCK edge (sck: %d, data_0: %d, data_1: %d, data_2: %d, data_3: %d, mask: 0x%x)\n", sck, sdio0, sdio1, sdio2, sdio3, mask);

    if (prev_sck == 1 && !sck)
    {
        handle_clk_low(sdio0, sdio1, sdio2, sdio3, mask);
    }
    else if (prev_sck == 0 && sck)
    {
        handle_clk_high(sdio0, sdio1, sdio2, sdio3, mask);
    }
    prev_sck = sck;
}
