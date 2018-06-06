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

#include "vp/vp.hpp"
#include "vp/trace/trace.hpp"

namespace vp {

  class Linear_volt_table
  {
  public:
    Linear_volt_table(double temp, js::config *config);
    inline double get(double frequency) { return any; }

    double volt;

  private:
    double any;
  };



  class Linear_temp_table
  {
  public:
    Linear_temp_table(double temp, js::config *config);
    double get(double volt, double frequency);

    double temp;

  private:
    std::vector<Linear_volt_table *> volt_tables;
  };



  class Linear_table
  {
  public:
    Linear_table(js::config *config);
    double get(double temp, double volt, double frequency);

  private:
    std::vector<Linear_temp_table *> temp_tables;
  };

};



vp::Linear_volt_table::Linear_volt_table(double volt, js::config *config)
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

double vp::Linear_temp_table::get(double volt, double frequency)
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

vp::Linear_temp_table::Linear_temp_table(double temp, js::config *config)
{
  this->temp = temp;

  for (auto& x:config->get_childs())
  {
    volt_tables.push_back(new Linear_volt_table(std::stod(x.first), x.second));
  }
}

vp::Linear_table::Linear_table(js::config *config)
{
  for (auto& x:config->get_childs())
  {
    temp_tables.push_back(new Linear_temp_table(std::stod(x.first), x.second));
  }
}

double vp::Linear_table::get(double temp, double volt, double frequency)
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

vp::component_power::component_power(vp::component &top)
: top(top)
{
}



int vp::component_power::new_trace(std::string name, power_trace *trace)
{
  if (trace->init(&top, name))
    return -1;

  this->traces.push_back(trace);

  return 0;
}


int vp::component_power::new_event(std::string name, power_source *source, js::config *config, power_trace *trace)
{
  if (source->init(&top, name, config, trace))
    return -1;

  source->setup(VP_POWER_DEFAULT_TEMP, VP_POWER_DEFAULT_VOLT, VP_POWER_DEFAULT_FREQ);

  return 0;
}


int vp::power_trace::init(component *top, std::string name)
{
  this->top = top;
  top->traces.new_trace_event_real(name, &this->trace);
  this->value = 0;
  this->timestamp = 0;
  return 0;
}


void vp::power_trace::collect()
{
  this->top->power.reg_top_trace(this);
}

void vp::power_trace::reg_top_trace(vp::power_trace *trace)
{
  this->top_traces.push_back(trace);
}


void vp::power_source::setup(double temp, double volt, double freq)
{
  this->quantum = this->table->get(temp, volt, freq);
}



int vp::power_source::init(component *top, std::string name, js::config *config, vp::power_trace *trace)
{

  this->top = top;
  this->trace = trace;

  try
  {
    if (config == NULL)
    {
      snprintf(vp_error, VP_ERROR_SIZE, "Didn't find power trace (name: %s)",  name.c_str());
      return -1;
    }

    js::config *type_cfg = config->get("type");
    if (type_cfg == NULL)
    {
      snprintf(vp_error, VP_ERROR_SIZE, "Didn't find power trace type (name: %s)", name.c_str());
      return -1;
    }

    if (type_cfg->get_str() == "linear")
    {
      js::config *values = config->get("values");
      if (values == NULL)
      {
        snprintf(vp_error, VP_ERROR_SIZE, "Didn't find any value for linear power model");
        return -1;
      }

      this->table = new Linear_table(values);
    }
    else
    {
      snprintf(vp_error, VP_ERROR_SIZE, "%s", type_cfg->get_str().c_str());
      return -1;
    }
  }
  catch (std::logic_error e)
  {
    snprintf(vp_error, VP_ERROR_SIZE, "%s", e.what());
    return -1;
  }

  return 0;  
}


void vp::component_power::reg_top_trace(vp::power_trace *trace)
{
  for (auto& x: this->top.get_childs())
  {
    x->power.reg_top_trace(trace);
  }

  for (auto& x: this->traces)
  {
    if (x != trace)
      x->reg_top_trace(trace);
  }

}
