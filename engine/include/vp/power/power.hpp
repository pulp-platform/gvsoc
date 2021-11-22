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
         * they are accounted separetly.
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
             * @brief Account the event
             * 
             * This should be used in quantum-based power source to trigger the consumption
             * of a quantum of energy.
             * This will just add the quantum of energy to the current consumed energy.
             */
            inline void account_event();

            /**
             * @brief Start accounting power
             * 
             * This should be used either for a background dynamic power or for leakage
             * to start accounting the associated power.
             * The power is accounted until the power source is turned off.
             * This is actually converted to energy and accounted on the total of energy
             * anytime current power is changed or source is turned off.
             */
            inline void leakage_power_start();
            inline void dynamic_power_start();

            /**
             * @brief Stop accounting power
             * 
             * This will trigger the accounting of energy for the current windows
             * and stop accounting power until it is turned on again.
             */
            inline void leakage_power_stop();
            inline void dynamic_power_stop();

        protected:
            /**
             * @brief Initialize a power source
             *
             * @param top Component containing the power source.
             * @param name Name of the power source, used in traces.
             * @param data Configuration of the power source, giving power numbers.
             * @param trace Power trace where this source should account power consumed.
             */
            int init(component *top, std::string name, js::config *config, power_trace *trace);
            
            /**
             * @brief Set temperature, voltage and frequency
             * 
             * The power source will adapt its power number according to the given characteristics.
             *
             * @param temp Temperature
             * @param volt Voltage
             * @param freq Frequency
             */
            void setup(double temp, double volt, double freq);

        private:
            Linear_table *table = NULL;  // Table of power values for all supported temperatures and voltages
                                        // imported from the json configuration given when trace was initialized.
            double quantum;        // Current quantumm of energy. Only valid in quantum-based power sources.
                                // The current value is estimated depending on voltage and temperature according
                                // to the provided json configuration.
            component *top;        // Top component containing the power source
            power_trace *trace;    // Power trace where the power consumption should be reported.
            bool is_on = false;    // True is the source is on and backgroun-power and leakage should be reported
        };


        /**
         * @brief Used for tracing power consumption
         * 
         * This class can be used to gather the power consumption of several power sources and
         * trace it through VCD traces and power reports.
         * Each power source must be associated to a power trace and a power trace can be associated
         * to one ore more power traces.
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

            void dump(FILE *file);

        protected:

            inline double get_value();

            inline double get_total();

            inline double get_total_leakage();

            bool is_dumped() { return this->dumped; }

            void clear();

            void get_power(double *dynamic, double *leakage);
            void get_energy(double *dynamic, double *leakage);

            vp::trace trace;

            void incr_dynamic_power(double power_incr);
            void incr_leakage_power(double power_incr);

            inline void account_quantum(double quantum);

        private:
            // Compute the energy spent on the current windows and account it to the total amount of energy.
            // This should be called everytime the current power is updated or before it is dumped.
            void account_power();
            void account_leakage_power();
            void incr(double quantum);


            component *top;
            double value;
            double total;
            double total_leakage;
            int64_t timestamp;
            int64_t last_clear_timestamp;

            double current_power;            // Power of the current power. This is used to account the energy over the period
                                            // being measured. Everytime it is updated, the energy should be computed
                                            // and the timestamp updated.
            int64_t current_power_timestamp; // Indicate the timestamp of the last time the energy was accounted
                                            // This is used everytime power is updated or dumped to compute the energy spent
                                            // over the period.
            double current_leakage_power;
            int64_t current_leakage_power_timestamp;

            bool dumped;
        };

    };

};
