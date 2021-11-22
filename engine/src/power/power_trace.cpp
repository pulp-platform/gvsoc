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


int vp::power::power_trace::init(component *top, std::string name)
{
  this->top = top;
  top->traces.new_trace_event_real(name, &this->trace);
  this->value = 0;
  this->total = 0;
  this->total_leakage = 0;
  this->timestamp = 0;
  this->trace.event_real(0);
  //this->top->power.get_engine()->reg_trace(this);

  this->current_power = 0;
  this->current_power_timestamp = 0;

  this->current_leakage_power = 0;
  this->current_leakage_power_timestamp = 0;

  return 0;
}


void vp::power::power_trace::clear()
{
  this->dumped = false;
  this->total = this->get_value();
  this->total_leakage = 0;
  this->last_clear_timestamp = this->top->get_time();
  this->current_power_timestamp = this->top->get_time();
  this->current_leakage_power_timestamp = this->top->get_time();
}

void vp::power::power_trace::get_energy(double *dynamic, double *leakage)
{
    *dynamic = this->get_total();
    *leakage = this->get_total_leakage();
}

void vp::power::power_trace::get_power(double *dynamic, double *leakage)
{
  this->dumped = true;

  double childs_dynamic=0, childs_leakage=0;
  this->top->power.power_get_energy_from_childs(&childs_dynamic, &childs_leakage);

  *dynamic = (childs_dynamic + this->get_total()) / (this->top->get_time() - this->last_clear_timestamp);

  *leakage = (childs_leakage + this->get_total_leakage()) / (this->top->get_time() - this->last_clear_timestamp);
}

void vp::power::power_trace::dump(FILE *file)
{
  fprintf(file, "Trace path; Dynamic power (W); Leakage power (W); Total (W); Percentage\n");

  double dynamic, leakage;
  this->get_power(&dynamic, &leakage);
  double total = dynamic + leakage;

  fprintf(file, "%s; %.12f; %.12f; %.12f; 1.0\n", this->trace.get_full_path().c_str(), dynamic, leakage, total);

  this->top->power.dump_child_traces(file, total);

  fprintf(file, "\n");

}

void vp::power::power_trace::incr(double quantum)
{
  this->get_value();

  this->value += quantum;
  this->total += quantum;

  if (this->top->get_clock())
  {
      this->trace.event_real_pulse(this->top->get_period(), this->value, 0);
  }
}


void vp::power::power_trace::account_power()
{
  // We need to compute the energy spent on the current windows.

  // First measure the duration of the windows
  int64_t diff = this->top->get_time() - this->current_power_timestamp;

  if (diff > 0)
  {
    // Then energy based on the current power. Note that this can work only if the 
    // power was constant over the period, which is the case, since this function is called
    // before any modification to the power.
    double energy = this->current_power * diff;

    this->total += energy;

    // And update the timestamp to the current one to start a new window
    this->current_power_timestamp = this->top->get_time();
  }
}


void vp::power::power_trace::account_leakage_power()
{
  int64_t diff = this->top->get_time() - this->current_leakage_power_timestamp;
  if (diff > 0)
  {
    double energy = this->current_leakage_power * diff;
    this->total_leakage += energy;
    this->current_leakage_power_timestamp = this->top->get_time();
  }
}

void vp::power::power_trace::incr_dynamic_power(double power_incr)
{
  // Leakage and dynamic are handled differently since they are reported separately,
  // In both cases, first compute the power on current period, start a new one,
  // and change the power so that it is constant over the period, to properly
  // compute the energy.
  this->account_power();
  this->current_power += power_incr;
}

void vp::power::power_trace::incr_leakage_power(double power_incr)
{
  // Leakage and dynamic are handled differently since they are reported separately,
  // In both cases, first compute the power on current period, start a new one,
  // and change the power so that it is constant over the period, to properly
  // compute the energy.
  this->account_leakage_power();
  this->current_leakage_power += power_incr;
}
