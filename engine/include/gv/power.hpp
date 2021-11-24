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

#pragma once

#include "json.hpp"
#include "vp/vp_data.hpp"

/**
 * @brief Power framework
 * 
 * Power is modeled through 2 classes, one for declaring sources of power consumptions
 * and another one for collecting consumed power from sources.
 * A note on method visibility: all the public functions are there for the HW models
 * to model HW power consumption while protected methods are there for the classes
 * belonging to vp::power, to manage the overall power framework.
 * 
 */

namespace vp
{

    namespace power
    {

#define VP_POWER_DEFAULT_TEMP 25
#define VP_POWER_DEFAULT_VOLT 1.2
#define VP_POWER_DEFAULT_FREQ 50

        class Linear_table;
        class engine;
        class power_source;
        class power_trace;
        class component_power;

        /**
         * @brief Used to model a power source
         *
         * A power source is a piece of hardware which is consuming energy.
         * This class can be instantiated any time as needed to model all the ways
         * an IP is consuming energy.
         * It can be used for dynamic or leakage power, or for both at the same time, since
         * they are accounted separatly.
         * For dynamic power it can be used either as a quantum-based power source or
         * as background power source.
         * A quantum-based power source will consume energy only when an event is triggered.
         * A background power source will constantly consume power as soon as it is on.
         * A leakage power source is similar to a background power source, but is just accounted
         * differently in the power report.
         */
        class power_source
        {
            // Only classes from vp::power are allowed as friends
            friend class vp::power::component_power;

        public:
            /**
             * @brief Account the the current energy quantum
             * 
             * This should be used in quantum-based power source to trigger the consumption
             * of a quantum of energy.
             * The accounted quantum is the current one, estimated from the current temperature
             * and voltage.
             * This will just add the quantum of energy to the current consumed energy.
             */
            inline void account_energy_quantum();

            /**
             * @brief Start accounting background power
             * 
             * This should be used for a background dynamic power to start accounting the associated power.
             * The power is accounted until the background power is stopped.
             * The accounted power is the current one, estimated from the current temperature
             * and voltage.
             * This is actually converted to energy and accounted on the total of energy
             * anytime current power is changed or power is stopped.
             */
            inline void dynamic_power_start();

            /**
             * @brief Stop accounting background power
             * 
             * This will trigger the accounting of energy for the current windows
             * and stop accounting power until it is started again.
             */
            inline void dynamic_power_stop();

            /**
             * @brief Start accounting leakage
             * 
             * This should be used for leakage to start accounting the associated power.
             * The power is accounted until the leakage is stopped.
             * The accounted leakage is the current one, estimated from the current temperature
             * and voltage.
             * This is actually converted to energy and accounted on the total of energy
             * anytime current power is changed or leakage is stopped.
             */
            inline void leakage_power_start();

            /**
             * @brief Stop accounting leakage
             * 
             * This will trigger the accounting of energy for the current windows
             * and stop accounting leakage until it is started again.
             */
            inline void leakage_power_stop();

        protected:
            /**
             * @brief Initialize a power source
             *
             * This method is reserved for methods belonging to namespace vp::power.
             *
             * @param top Component containing the power source.
             * @param name Name of the power source, used in traces.
             * @param config Configuration of the power source, giving power numbers.
             * @param trace Power trace where this source should account power consumed.
             */
            int init(component *top, std::string name, js::config *config, power_trace *trace);
            
            /**
             * @brief Set temperature, voltage and frequency
             * 
             * The power source will adapt its power number according to the given characteristics.
             * This method is reserved for methods belonging to namespace vp::power.
             *
             * @param temp Temperature
             * @param volt Voltage
             * @param freq Frequency
             */
            void setup(double temp, double volt, double freq);

        private:
            Linear_table *table = NULL;  // Table of power values for all supported temperatures and voltages
                                        // imported from the json configuration given when trace was initialized.
            double quantum;          // Current quantumm of energy, for quantum-based power consumption.
                                     // The current value is estimated depending on voltage and temperature according
                                     // to the provided json configuration.
            double background_power; // Current background power, for background-based power consumption.
                                     // The current value is estimated depending on voltage and temperature according
                                     // to the provided json configuration.
            double leakage;          // Current leakage power, for leakage-based power consumption.
                                     // The current value is estimated depending on voltage and temperature according
                                     // to the provided json configuration.
            component *top;          // Top component containing the power source
            power_trace *trace;      // Power trace where the power consumption should be reported.
            bool is_on = false;      // True is the source is on and backgroun-power and leakage should be reported
        };


        /**
         * @brief Used for tracing power consumption
         * 
         * This class can be used to gather the power consumption of several power sources and
         * trace it through VCD traces and power reports.
         * Each power source must be associated to a power trace and a power trace can be associated
         * to one ore more power traces.
         * A power trace is automatically created with each component and is the default trace associated
         * to any power source created in this component. Additional traces can also be created
         * to give finer granularity to the power consumption in the reports.
         */
        class power_trace
        {
            // Only classes from vp::power are allowed as friends
            friend class vp::power::power_source;
            friend class vp::power::engine;
            friend class vp::power::component_power;

        public:
            /**
             * @brief Init the trace
             *
             * @param top Component containing the power trace.
             * @param name Name of the power trace. It will be used in traces and in the power report
             */
            int init(component *top, std::string name);

            /**
             * @brief Return if the trace is enabled
             * 
             * @return true if the trace is active and should account power
             * @return false if the trace is inactive and any activity should be ignored
             */
            inline bool get_active() { return trace.get_event_active(); }

            /**
             * @brief Dump the trace
             *
             * This allowss dumping the power consumption of this trace to a file.
             * This also dumps the consumption of the traces of the child components
             * to get the hierarchical distribution.
             *
             * @param file File descriptor where the trace should.
             */
            void dump(FILE *file);

            void set_parent(power_trace *parent);

        protected:

            /**
             * @brief Start monitoring power for report generation.
             *
             * This will clear all pending power values so that a new report starts
             * to be generating within a window which start when this method is
             * called.
             * It can be called several time to generate several reports, each one
             * on a different time window.
             * This method is reserved for methods belonging to namespace vp::power.
             *
             * @param file File descriptor where the trace should.
             */
            void report_start();

            inline double get_power() { return this->current_power; }

            /**
             * @brief Report the average power consumed on the active window.
             *
             * The time duration of the active window (starting at the time where
             * report_start was called) is computed, the average power estimated
             * on this window and returned (both dynamic and leakage).
             * This method is reserved for methods belonging to namespace vp::power.
             *
             * @param dynamic Dynamic power consumed on the time window is reported here.
             * @param leakage Leakage power consumed on the time window is reported here.
             */
            void get_report_power(double *dynamic, double *leakage);

            /**
             * @brief Report the energy consumed on the active window.
             *
             * The time duration of the active window (starting at the time where
             * report_start was called) is computed, the total amount of energy consumed
             * estimated on this window and returned (both dynamic and leakage).
             * This method is reserved for methods belonging to namespace vp::power.
             *
             * @param dynamic Dynamic power consumed on the time window is reported here.
             * @param leakage Leakage power consumed on the time window is reported here.
             */
            void get_report_energy(double *dynamic, double *leakage);

            /**
             * @brief Increment the current dynamic background power.
             *
             * The dynamic background power is incremented by the specified value
             * to reflect a change in power consumption.
             * Note that this power is consumed constantly until it is modified again.
             * This can be used to model some power consumed in background activity
             * (e.g. a core in idle mode).
             * This method is reserved for methods belonging to namespace vp::power.
             *
             * @param power_inc Power increase. Can be negative to reflect a decrease.
             */
            void inc_dynamic_power(double power_inc);

            /**
             * @brief Increment the current leakage power.
             *
             * The leakage power is incremented by the specified value
             * to reflect a change in power consumption.
             * Note that this power is consumed constantly until it is modified again.
             * This can be used to model the leakage power which is consumed in background
             * as soon as an IP is on.
             * This method is reserved for methods belonging to namespace vp::power.
             *
             * @param power_inc Power increase. Can be negative to reflect a decrease.
             */
            void inc_leakage_power(double power_incr);

            /**
             * @brief Increment the current energy consumed.
             *
             * The amount of energy consumed is increased by the specified amount of
             * energy (in joule) to reflect that an event occured and consumed some
             * energy (like a memory access).
             * This method is reserved for methods belonging to namespace vp::power.
             *
             * @param power_inc Power increase. Can be negative to reflect a decrease.
             */
            void inc_dynamic_energy(double energy);

        private:
            // Regularly, current power consumption is converted into energy and added
            // to the total amount of energy consumed, for example when the current power
            // consumption is modified.
            // Calling this function will do this conversion for the dynamic part of the
            // power consumed.
            void account_dynamic_power();

            // Regularly, current power consumption is converted into energy and added
            // to the total amount of energy consumed, for example when the current power
            // consumption is modified.
            // Calling this function will do this conversion for the leakage part of the
            // power consumed.
            void account_leakage_power();

            inline double get_dynamic_energy_for_cycle();
            inline void flush_dynamic_energy_for_cycle();

            inline double get_dynamic_energy();

            inline double get_leakage_energy();

            // Dump VCD trace reporting the powr consumption
            // This should be called everytime the energy consumed in the current cycle
            // or the current backgroun or leakage power is modified.
            void dump_vcd_trace();

            static void trace_handler(void *__this, vp::clock_event *event);


            component *top;
            power_trace *parent;
            vp::clock_event *trace_event;

            vp::trace trace;                    // Trace used for reporting power in VCD traces
            double dynamic_energy_for_cycle;
            double total_dynamic_energy;
            double total_leakage_energy;
            int64_t timestamp;
            int64_t report_start_timestamp;

            double current_dynamic_power;            // Power of the current power. This is used to account the energy over the period
                                            // being measured. Everytime it is updated, the energy should be computed
                                            // and the timestamp updated.
            int64_t current_dynamic_power_timestamp; // Indicate the timestamp of the last time the energy was accounted
                                            // This is used everytime power is updated or dumped to compute the energy spent
                                            // over the period.
            double current_leakage_power;
            int64_t current_leakage_power_timestamp;

            double current_power;
        };


        class component_power
        {
            friend class power_trace;

        public:
            component_power(component &top);

            power::engine *get_engine() { return engine; }

            void build();

            int new_power_source(std::string name, power_source *source, js::config *config, power_trace *trace=NULL);

            int new_power_trace(std::string name, power_trace *trace);

            vp::power::power_trace *get_power_trace() { return &this->power_trace; }

        protected:
            void get_energy_from_childs(double *dynamic, double *leakage);
            double get_power_from_childs();

            void dump(FILE *file, double total);
            void dump_child_traces(FILE *file, double total);

        private:
            void get_energy_from_self_and_childs(double *dynamic, double *leakage);
            double get_power_from_self_and_childs();

            component &top;

            vp::power::power_trace power_trace;

            std::vector<vp::power::power_trace *> traces;

            power::engine *engine = NULL;
        };
    };

};
