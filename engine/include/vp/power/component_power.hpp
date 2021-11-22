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

#ifndef __VP_TRACE_COMPONENT_POWER_HPP__
#define __VP_TRACE_COMPONENT_POWER_HPP__

#include "vp/component.hpp"
#include "vp/power/power.hpp"
#include "json.hpp"

using namespace std;

namespace vp
{

    namespace power
    {
        class power_trace;
        class power_source;

        class component_power
        {

        public:
            component_power(component &top);

            power::engine *get_engine() { return power_manager; }

            void post_post_build();

            void pre_start();

            int new_power_source(std::string name, power_source *source, js::config *config, power_trace *trace=NULL);

            int new_power_trace(std::string name, power_trace *trace);

            vp::power::power_trace *power_get_power_trace() { return &this->power_trace; }
        //protected:
            void power_get_energy_from_childs(double *dynamic, double *leakage);

            void dump(FILE *file, double total);
            void dump_child_traces(FILE *file, double total);

        private:
            void power_get_energy_from_self_and_childs(double *dynamic, double *leakage);

            component &top;

            vp::power::power_trace power_trace;

            std::vector<vp::power::power_trace *> traces;

            power::engine *power_manager = NULL;
        };
    };

};

#endif
