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
#include "i2s_verif.hpp"

class Slot
{
public:
    Slot(Testbench *top, int itf, int id);
    void setup(pi_testbench_i2s_verif_slot_config_t *config);
    void start(pi_testbench_i2s_verif_slot_start_config_t *config);
    void start_frame();
    int get_data();
    void send_data(int sdo);
    int pdm_sync(int sck, int ws, int sd);

private:
    Testbench *top;
    int id;
    vp::trace trace;
    pi_testbench_i2s_verif_slot_config_t config_rx;
    pi_testbench_i2s_verif_slot_config_t config_tx;
    pi_testbench_i2s_verif_slot_start_config_t start_config_rx;
    pi_testbench_i2s_verif_slot_start_config_t start_config_tx;
    bool rx_started;
    int rx_pending_bits;
    int rx_current_value;
    uint32_t rx_pending_value;

    bool tx_started;
    int tx_pending_bits;
    uint32_t tx_pending_value;
    FILE *outfile;
    FILE *infile;
};


I2s_verif::I2s_verif(Testbench *top, vp::i2s_master *itf, int itf_id, pi_testbench_i2s_verif_config_t *config)
{
    ::memcpy(&this->config, config, sizeof(pi_testbench_i2s_verif_config_t));

    this->itf = itf;
    this->prev_ws = 0;
    this->frame_active = false;
    this->ws_delay = 1;
    this->current_ws_delay = 0;
    this->is_pdm = config->is_pdm;
    if (this->is_pdm)
    {
        this->config.nb_slots = 4;
    }

    top->traces.new_trace("i2s_verif_itf" + std::to_string(itf_id), &trace, vp::DEBUG);

    for (int i=0; i<this->config.nb_slots; i++)
    {
        this->slots.push_back(new Slot(top, itf_id, i));
    }
}


void I2s_verif::slot_setup(pi_testbench_i2s_verif_slot_config_t *config)
{
    if (config->slot >= this->config.nb_slots)
    {
        this->trace.fatal("Trying to configure invalid slot (slot: %d, nb_slot: %d)", config->slot, this->config.nb_slots);
        return;
    }

    this->slots[config->slot]->setup(config);

}


void I2s_verif::slot_start(pi_testbench_i2s_verif_slot_start_config_t *config)
{
    this->trace.msg(vp::trace::LEVEL_INFO, "Starting (nb_samples: %d, incr_start: 0x%x, incr_end: 0x%x, incr_value: 0x%x)\n",
        config->rx_iter.nb_samples, config->rx_iter.incr_start, config->rx_iter.incr_end, config->rx_iter.incr_value);

    int slot = config->slot;

    if (slot >= this->config.nb_slots)
    {
        this->trace.fatal("Trying to configure invalid slot (slot: %d, nb_slot: %d)", slot, this->config.nb_slots);
        return;
    }

    this->slots[slot]->start(config);

}


void I2s_verif::sync(int sck, int ws, int sdio)
{
    int sd = sdio >> 2;

    this->trace.msg(vp::trace::LEVEL_TRACE, "I2S edge (sck: %d, ws: %d, sdo: %d)\n", sck, ws, sd);

    if (this->is_pdm)
    {
        if (!sck)
        {
            int val0 = this->slots[0]->pdm_sync(sck, ws, sdio & 3);
            int val1 = this->slots[2]->pdm_sync(sck, ws, sdio >> 2);
            this->itf->sync(2, 2, val0 | (val1 << 2));
        }
        else
        {
            int val0 = this->slots[1]->pdm_sync(sck, ws, sdio & 3);
            int val1 = this->slots[3]->pdm_sync(sck, ws, sdio >> 2);
            this->itf->sync(2, 2, val0 | (val1 << 2));
        }
    }
    else
    {
        if (sck)
        {
            // The channel is the one of this microphone
            if (this->prev_ws != ws && ws == 1)
            {
                this->trace.msg(vp::trace::LEVEL_DEBUG, "Detected frame start\n");

                // If the WS just changed, apply the delay before starting sending
                this->current_ws_delay = this->ws_delay;
                if (this->current_ws_delay == 0)
                {
                    this->frame_active = true;
                    this->pending_bits = this->config.word_size;
                    this->active_slot = 0;
                }
            }

            // If there is a delay, decrease it
            if (this->current_ws_delay > 0)
            {
                this->current_ws_delay--;
                if (this->current_ws_delay == 0)
                {
                    this->frame_active = true;
                    this->pending_bits = this->config.word_size;
                    this->active_slot = 0;
                }
            }

            if (this->frame_active)
            {
                this->slots[this->active_slot]->send_data(sd);
            }

            if (this->frame_active)
            {
                if (this->pending_bits == this->config.word_size)
                {
                    this->slots[this->active_slot]->start_frame();
                }
                int data = this->slots[this->active_slot]->get_data();

                this->itf->sync(2, 2, data | (2 << 2));

                this->pending_bits--;
                if (this->pending_bits == 0)
                {
                    this->pending_bits = this->config.word_size;
                    this->active_slot++;
                    if (this->active_slot == this->config.nb_slots)
                    {
                        this->frame_active = false;
                    }
                }
            }

            this->prev_ws = ws;
        }
    }
}


Slot::Slot(Testbench *top, int itf, int id) : top(top), id(id)
{
    top->traces.new_trace("i2s_verif_itf" + std::to_string(itf) + "_slot" + std::to_string(id), &trace, vp::DEBUG);

    this->config_rx.enabled = false;
    this->rx_started = false;
    this->tx_started = false;
    this->infile = NULL;
    this->outfile = NULL;
}


void Slot::setup(pi_testbench_i2s_verif_slot_config_t *config)
{
    if (config->is_rx)
    {
        ::memcpy(&this->config_rx, config, sizeof(pi_testbench_i2s_verif_slot_config_t));
    }
    else
    {
        ::memcpy(&this->config_tx, config, sizeof(pi_testbench_i2s_verif_slot_config_t));
    }

    this->trace.msg(vp::trace::LEVEL_INFO, "Slot setup (is_rx: %d, enabled: %d, word_size: %d)\n",
        config->is_rx, config->enabled, config->word_size);
}


void Slot::start(pi_testbench_i2s_verif_slot_start_config_t *config)
{
    if (config->type == PI_TESTBENCH_I2S_VERIF_RX_ITER)
    {
        ::memcpy(&this->start_config_rx, config, sizeof(pi_testbench_i2s_verif_slot_start_config_t));

        this->rx_started = true;
        this->rx_current_value = this->start_config_rx.rx_iter.incr_start;
        this->rx_pending_bits = 0;
    }
    else if (config->type == PI_TESTBENCH_I2S_VERIF_TX_FILE_DUMPER)
    {
        ::memcpy(&this->start_config_tx, config, sizeof(pi_testbench_i2s_verif_slot_start_config_t));

        this->tx_started = true;

        char *filepath = (char *)config + sizeof(pi_testbench_i2s_verif_slot_start_config_t);
        this->outfile = fopen(filepath, "w");
        if (this->outfile == NULL)
        {
            this->trace.fatal("Unable to open file (file: %s, error: %s)\n", filepath, strerror(errno));
        }
    }
    else if (config->type == PI_TESTBENCH_I2S_VERIF_PDM_RX_FILE_READER)
    {
        ::memcpy(&this->start_config_tx, config, sizeof(pi_testbench_i2s_verif_slot_start_config_t));

        this->rx_started = true;

        char *filepath = (char *)config + sizeof(pi_testbench_i2s_verif_slot_start_config_t);
        this->infile = fopen(filepath, "r");
        if (this->infile == NULL)
        {
            this->trace.fatal("Unable to open file (file: %s, error: %s)\n", filepath, strerror(errno));
        }
    }
}


void Slot::start_frame()
{
    if (this->rx_started && (this->start_config_rx.rx_iter.nb_samples > 0 || this->start_config_rx.rx_iter.nb_samples == -1))
    {
        this->rx_pending_bits = this->config_rx.word_size;
        this->rx_pending_value = this->rx_current_value;
        this->trace.msg(vp::trace::LEVEL_DEBUG, "Starting RX sample (sample: 0x%x)\n", this->rx_pending_value);
        if (this->start_config_rx.rx_iter.incr_end - this->rx_current_value <= this->start_config_rx.rx_iter.incr_value)
        {
            this->rx_current_value = this->start_config_rx.rx_iter.incr_start;
        }
        else
        {
            this->rx_current_value += this->start_config_rx.rx_iter.incr_value;
        }
        if (this->start_config_rx.rx_iter.nb_samples > 0)
        {
            this->start_config_rx.rx_iter.nb_samples--;
        }
    }

    if (this->tx_started && (this->start_config_tx.tx_file_dumper.nb_samples > 0 || this->start_config_tx.tx_file_dumper.nb_samples == -1))
    {
        this->tx_pending_value = 0;
        this->tx_pending_bits = this->config_tx.word_size;
        this->trace.msg(vp::trace::LEVEL_DEBUG, "Starting TX sample\n");

    }
}

void Slot::send_data(int sd)
{
    if (this->tx_started)
    {
        if (this->tx_pending_bits > 0)
        {
            this->tx_pending_bits--;
            this->tx_pending_value = (this->tx_pending_value << 1) | (sd == 1);

            if (this->tx_pending_bits == 0)
            {
                this->trace.msg(vp::trace::LEVEL_DEBUG, "Writing sample (value: 0x%lx)\n", this->tx_pending_value);
                fprintf(this->outfile, "0x%x\n", this->tx_pending_value);
                this->tx_pending_bits = this->config_tx.word_size;
            }
        }
    }
}


int Slot::get_data()
{
    if (this->rx_started)
    {
        int data;

        if (this->rx_pending_bits > 0)
        {
            data = (this->rx_pending_value >> (this->config_rx.word_size - 1)) & 1;
            this->rx_pending_bits--;
            this->rx_pending_value <<= 1;
        }
        else
        {
            if (this->start_config_rx.rx_iter.nb_samples == 0)
            {
                this->rx_started = false;
            }
            data = 3;   // Return X in case there is no more data
        }

        this->trace.msg(vp::trace::LEVEL_TRACE, "Getting data (data: %d)\n", data);

        return data;
    }

    return 3;
}


int Slot::pdm_sync(int sck, int ws, int sd)
{
    if (this->infile == NULL)
        return 3;


    char line [16];

    if (fgets(line, 16, this->infile) == NULL)
    {
        fseek(this->infile, 0, SEEK_SET);
        if (fgets(line, 16, this->infile))
        {
            this->trace.fatal("Unable to get sample from file (error: %s)\n", strerror(errno));
            return 3;
        }
    }

    int data = atoi(line);

    return data;
}