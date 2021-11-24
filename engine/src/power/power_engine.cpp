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

#include "vp/vp.hpp"
#include "vp/trace/trace.hpp"



vp::power::Linear_volt_table::Linear_volt_table(double volt, js::config *config)
{
  this->volt = volt;

  for (auto& x:config->get_childs())
  {
    if (x.first == "any")
    {
      this->any = std::stod(x.second->get_str());
    }
    else
    {
      throw std::logic_error("Only any frequency is allowed for now");
    }
  }
}

double vp::power::Linear_temp_table::get(double volt, double frequency)
{
  int low_index = -1, high_index = -1;

  // Go through the temperatures to find the one just below and the one
  // just above
  for (unsigned int i=0; i<this->volt_tables.size(); i++)
  {
    if (this->volt_tables[i]->volt == volt)
    {
      low_index = high_index = i;
      break;
    }

    if (this->volt_tables[i]->volt > volt)
    {
      high_index = i;
      break;
    }

    low_index = i;
  }

  if (high_index == -1)
    high_index = low_index;

  if (low_index == -1)
    low_index = high_index;

  double value;
  if (high_index == low_index)
  {
    value = this->volt_tables[low_index]->get(frequency);
  }
  else
  {
    double low_volt = this->volt_tables[low_index]->volt;
    double high_volt = this->volt_tables[high_index]->volt;

    double value_at_low_volt = this->volt_tables[low_index]->get(frequency);
    double value_at_high_volt = this->volt_tables[high_index]->get(frequency);

    double volt_ratio = (volt - low_volt) / (high_volt - low_volt);
    value = (value_at_high_volt - value_at_low_volt)*volt_ratio + value_at_low_volt;
  }
  return value;
}

vp::power::Linear_temp_table::Linear_temp_table(double temp, js::config *config)
{
  this->temp = temp;

  for (auto& x:config->get_childs())
  {
    volt_tables.push_back(new Linear_volt_table(std::stod(x.first), x.second));
  }
}

vp::power::Linear_table::Linear_table(js::config *config)
{
  for (auto& x:config->get_childs())
  {
    temp_tables.push_back(new Linear_temp_table(std::stod(x.first), x.second));
  }
}

double vp::power::Linear_table::get(double temp, double volt, double frequency)
{
  int low_temp_index = -1, high_temp_index = -1;

  // Go through the temperatures to find the one just below and the one
  // just above
  for (unsigned int i=0; i<this->temp_tables.size(); i++)
  {
    if (this->temp_tables[i]->temp == temp)
    {
      low_temp_index = high_temp_index = i;
      break;
    }

    if (this->temp_tables[i]->temp > temp)
    {
      high_temp_index = i;
      break;
    }

    low_temp_index = i;
  }

  if (high_temp_index == -1)
    high_temp_index = low_temp_index;

  if (low_temp_index == -1)
    low_temp_index = high_temp_index;

  double value;

  if (high_temp_index == low_temp_index)
  {
    value = this->temp_tables[low_temp_index]->get(volt, frequency);
  }
  else
  {
    double low_temp = this->temp_tables[low_temp_index]->temp;
    double high_temp = this->temp_tables[high_temp_index]->temp;

    double value_at_low_temp = this->temp_tables[low_temp_index]->get(volt, frequency);
    double value_at_high_temp = this->temp_tables[high_temp_index]->get(volt, frequency);

    double temp_ratio = (temp - low_temp) / (high_temp - low_temp);
    value = (value_at_high_temp - value_at_low_temp)*temp_ratio + value_at_low_temp;
  }
  return value;
}





void vp::power::engine::reg_trace(vp::power::power_trace *trace)
{
    this->traces.push_back(trace);
}



void vp::power::engine::start_capture()
{
    for (auto trace : this->traces)
    {
        trace->report_start();
    }
}



void vp::power::engine::stop_capture()
{
    FILE *file = fopen("power_report.csv", "w");
    if (file == NULL)
    {
        //vp_warning_always(&this->warning, "Failed to open power report file (path: %s)\n", "power_report.csv");
        return;
    }

    this->top->dump_traces_recursive(file);
}





vp::power::engine::engine(vp::component *top)
{
    this->top = top;
    top->new_service("power", this);
}



