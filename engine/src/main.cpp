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

#include <vp/vp.hpp>
#include <algorithm>
#include <dlfcn.h>




int main(int argc, char *argv[])
{
    char *config_path = NULL;
    for (int i=1; i<argc; i++)
    {
        if (strncmp(argv[i], "--config=", 9) == 0)
        {
            config_path = &argv[i][9];
        }
    }

    if (config_path == NULL)
    {
        fprintf(stderr, "No configuration specified, please specify through option --config=<config path>.\n");
        return -1;
    }

    js::config *js_config = js::import_config_from_file(config_path);
    if (js_config == NULL)
    {
        fprintf(stderr, "Invalid configuration.");
        return -1;
    }

    js::config *gv_config = js_config->get("**/gvsoc");

    std::string module_name = "vp.trace_domain_impl";

    if (gv_config->get_child_bool("debug-mode"))
    {
        module_name = "debug." + module_name;
    }

    std::replace(module_name.begin(), module_name.end(), '.', '/');

    std::string path = std::string(getenv("GVSOC_PATH")) + "/" + module_name + ".so";

    void *module = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL | RTLD_DEEPBIND);
    if (module == NULL)
    {
        throw std::invalid_argument("ERROR, Failed to open periph model (module: " + module_name + ", error: " + std::string(dlerror()) + ")");
    }

    vp::component *(*constructor)(js::config *) = (vp::component * (*)(js::config *)) dlsym(module, "vp_constructor");
    if (constructor == NULL)
    {
        throw std::invalid_argument("ERROR, couldn't find vp_constructor in loaded module (module: " + module_name + ")");
    }

    vp::component *instance = constructor(js_config);

    instance->set_vp_config(gv_config);

    instance->pre_pre_build();
    instance->pre_build();
    instance->build();
    instance->build_new();

    std::string status = instance->run();

    instance->stop();

    if (status == "killer")
    {
        fprintf(stderr, "The top engine was not responding and was killed.\n");
        return -1;
    }
    else if (status == "error")
    {
        return -1;
    }
    else
    {
        return instance->run_status();
    }
}