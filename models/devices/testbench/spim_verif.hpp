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

#ifndef __TESTBENCH_SPIM_VERIF_HPP__
#define __TESTBENCH_SPIM_VERIF_HPP__

#include "testbench.hpp"

#define SPIM_VERIF_CMD_BIT 56
#define SPIM_VERIF_CMD_WIDTH 8

#define SPIM_VERIF_CMD_INFO_BIT 32
#define SPIM_VERIF_CMD_INFO_WIDTH 24

#define SPIM_VERIF_CMD_ADDR_BIT 0
#define SPIM_VERIF_CMD_ADDR_WIDTH 32

#define SPIM_VERIF_FIELD_GET(value, bit, width) (((value) >> (bit)) & ((1UL << (width)) - 1))

typedef enum
{
    STATE_GET_CMD,
    STATE_WRITE_CMD,
    STATE_FULL_DUPLEX_CMD,
    STATE_READ_CMD
} Spim_verif_state_e;

typedef enum
{
    SPIM_VERIF_CMD_WRITE = 1,
    SPIM_VERIF_CMD_READ = 2,
    SPIM_VERIF_CMD_FULL_DUPLEX = 3,
    SPIM_VERIF_CMD_WRITE_QUAD = 4,
    SPIM_VERIF_CMD_READ_QUAD = 5,
} spim_cmd_e;

class Testbench;

class Spim_verif
{
public:
    Spim_verif(Testbench *top, vp::qspim_slave *itf, int itf_id, int cs, int mem_size);
    void cs_sync(int cs);
    void sync(int sck, int sdio0, int sdio1, int sdio2, int sdio3, int mask);

protected:
    void handle_clk_high(int sdio0, int sdio1, int sdio2, int sdio3, int mask);
    void handle_clk_low(int sdio0, int sdio1, int sdio2, int sdio3, int mask);

private:
    void handle_read(uint64_t cmd, bool is_quad);
    void handle_write(uint64_t cmd, bool is_quad);
    void handle_full_duplex(uint64_t cmd);

    void exec_write(int sdio0, int sdio1, int sdio2, int sdio3);
    void exec_read();
    void exec_dump_single(int sdio0);
    void exec_dump_qpi(int sdio0, int sdio1, int sdio2, int sdio3);

    void handle_command(uint64_t cmd);

    vp::trace trace;
    vp::qspim_slave *itf;
    Spim_verif_state_e state = STATE_GET_CMD;
    uint64_t current_cmd = 0;
    int prev_sck = 0;
    int cmd_count = 0;
    int dummy_cycles = 0;
    bool wait_cs;
    int command_addr;
    int current_addr;
    int current_write_addr;
    int current_size;
    int current_write_size;
    unsigned char *data;
    int nb_bits;
    int nb_write_bits;
    uint32_t byte;
    bool verbose;
    unsigned int pending_write;
    int current_cs;
    FILE *tx_file;
    int tx_dump_bits;
    bool tx_dump_qpi;
    int tx_dump_byte;
    int mem_size;
    bool is_quad;
};

#endif