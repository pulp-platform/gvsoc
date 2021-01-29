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

#include "nina_b112.hpp"

#include <regex>

namespace {

    std::string wrap_response(std::string response)
    {
        return std::string("\r\n") + response + std::string("\r\n");
    }

    std::string ok_response(std::string response)
    {
        return ::wrap_response(response + std::string("OK"));
    }

    std::string error_response(std::string response)
    {
        return ::wrap_response(response + std::string("ERROR"));
    }
}

std::string Nina_b112::parse_cmd(std::string input_cmd)
{
    std::smatch match;

    /* empty command */
    if (input_cmd.empty())
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received Attention_AT command\n");
        return ::ok_response("");
    }

    /* echo On/Off command */
    std::regex cmd_echo_mode("E[01]");
    if(std::regex_match(input_cmd, match, cmd_echo_mode))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received echo_on_off command\n");
        //TODO
        return ok_response("");
    }

    /* factory reset command */
    /* Reboot required */
    std::regex cmd_factory_reset("\\+UFACTORY");
    if(std::regex_match(input_cmd, match, cmd_factory_reset))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received factory_reset command (reboot required)\n");
        //TODO
        return ok_response("");
    }

    /* operating mode change command */
    std::regex cmd_op_mode_data("O[012]?");
    if(std::regex_match(input_cmd, match, cmd_op_mode_data))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received enter_data_mode command\n");
        std::string cmd = match[0];
        if (cmd.back() == 2)
        {
            this->trace.msg(vp::trace::LEVEL_INFO, "Switching to extended data operating mode: NOT SUPPORTED\n");
        }
        else if (cmd.back() == 0)
        {
            this->trace.msg(vp::trace::LEVEL_INFO, "Switching to command mode\n");
            this->operating_mode = NINA_B112_OPERATING_MODE_COMMAND;
        }
        else
        {
            /* default */
            this->trace.msg(vp::trace::LEVEL_INFO, "Switching to data operating mode\n");
            this->operating_mode = NINA_B112_OPERATING_MODE_DATA;
        }
        return ok_response("");
    }

    /* unbond command */
    std::regex cmd_unbond("\\+UBTUB\\=.*");
    if(std::regex_match(input_cmd, match, cmd_unbond))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received unbond command\n");
        //TODO
        return ok_response("");
    }

    /* bluetooth low energy role command */
    std::regex cmd_ble_role("\\+UBTLE\\=[0123]");
    if(std::regex_match(input_cmd, match, cmd_ble_role))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received bluetooth_low_energy_role command\n");
        //TODO
        return ok_response("");
    }

    /* local name command */
    std::regex cmd_local_name("\\+UBTLN\\=.*");
    if(std::regex_match(input_cmd, match, cmd_local_name))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received local_name command\n");
        //TODO
        //TODO parse string (surrounded by double quotes ?), verify length (max 31 characters)
        return ok_response("");
    }

    /* local name read command */
    std::regex cmd_local_name_read("\\+UBTLN\\?");
    if(std::regex_match(input_cmd, match, cmd_local_name_read))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received local_name_read command\n");
        //TODO
        //TODO parse string (surrounded by double quotes ?), verify length (max 31 characters)
        return ok_response("+UBTLN:\"NOT_IMPLEMENTED\"");
    }

    /* RS232 parameters command */
    std::regex cmd_rs232_params("\\+UMRS\\=.*");
    if(std::regex_match(input_cmd, match, cmd_rs232_params))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received rs232_parameters command\n");
        //TODO
        //TODO parse parameters, modify UART configuration after sending response
        return ok_response("");
    }

    /* RS232 parameters read command */
    std::regex cmd_rs232_params_read("\\+UMRS\\?");
    if(std::regex_match(input_cmd, match, cmd_rs232_params_read))
    {
        this->trace.msg(vp::trace::LEVEL_INFO, "Received rs232_parameters_read command\n");
        //TODO
        //TODO parse parameters, modify UART configuration after sending response
        return ok_response("+UMRS:NOT_IMPLEMENTED");
    }

    this->trace.msg(vp::trace::LEVEL_INFO, "Received unknown command %s\n", input_cmd.c_str());
    return error_response("");
}
