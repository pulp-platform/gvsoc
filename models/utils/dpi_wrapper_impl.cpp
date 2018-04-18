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
#include <vp/itf/io.hpp>
#include <stdio.h>
#include <string.h>
#include "dpi/tb_driver.h"
#include "vp/itf/qspim.hpp"
#include "vp/itf/jtag.hpp"
#include <vector>
#include <thread>
#include <unistd.h>

static thread_local vp::clock_event *wait_evt;
static thread_local bool is_waiting = false;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static vector<vp::qspim_slave *> qspim_slaves;
static vector<vp::jtag_master *> jtag_masters;

class dpi_wrapper : public vp::component
{

public:

  dpi_wrapper(const char *config);

  void build();
  void start();
  void create_task(void *arg1, void *arg2);
  int wait(int64_t t);
  int wait_ps(int64_t t);
  void wait_event();
  void raise_event();

private:

  void task_thread_routine(void *arg1, void *arg2);
  static void wait_handler(void *__this, vp::clock_event *event);
  static void jtag_sync(void *__this, int tdo, int id);

  vp::trace     trace;

  vector<std::thread *>threads;
  vector<void *>models;

  bool event_raised = false;
};

dpi_wrapper::dpi_wrapper(const char *config)
: vp::component(config)
{

}

void dpi_wrapper::wait_handler(void *__this, vp::clock_event *event)
{
  dpi_wrapper *_this = (dpi_wrapper *)__this;
  pthread_mutex_lock(&mutex);
  *(bool *)(event->get_args()[0]) = false;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);
}

void dpi_wrapper::task_thread_routine(void *arg1, void *arg2)
{
  this->get_clock()->get_engine()->lock();
  this->get_clock()->retain();
  wait_evt = event_new(dpi_wrapper::wait_handler);
  wait_evt->get_args()[0] = &is_waiting;
  ((void (*)(void *))arg1)(arg2);  
  this->get_clock()->release();
}

int dpi_wrapper::wait(int64_t t)
{
  int64_t period = get_period();
  int64_t cycles = (t*1000 + period - 1) / period;
  event_enqueue(wait_evt, cycles);
  is_waiting = true;
  get_clock()->get_engine()->unlock();

  pthread_mutex_lock(&mutex);
  while(is_waiting)
    pthread_cond_wait(&cond, &mutex);
  pthread_mutex_unlock(&mutex);

  get_clock()->get_engine()->lock();
  return 0;
}

int dpi_wrapper::wait_ps(int64_t t)
{
  int64_t period = get_period();
  int64_t cycles = (t + period - 1) / period;
  event_enqueue(wait_evt, cycles);
  is_waiting = true;
  get_clock()->get_engine()->unlock();

  pthread_mutex_lock(&mutex);
  while(is_waiting)
    pthread_cond_wait(&cond, &mutex);
  pthread_mutex_unlock(&mutex);

  get_clock()->get_engine()->lock();
  return 0;
}

void dpi_wrapper::wait_event()
{
  get_clock()->get_engine()->unlock();
  pthread_mutex_lock(&mutex);
  while(!event_raised)
    pthread_cond_wait(&cond, &mutex);
  event_raised = false;
  pthread_mutex_unlock(&mutex);
  get_clock()->get_engine()->lock();
}

void dpi_wrapper::raise_event()
{
  pthread_mutex_lock(&mutex);
  event_raised = true;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);
}

void dpi_wrapper::create_task(void *arg1, void *arg2)
{
  threads.push_back(new std::thread(&dpi_wrapper::task_thread_routine, this, arg1, arg2));
}

void dpi_wrapper::jtag_sync(void *__this, int tdo, int id)
{
  vp::jtag_master *itf = jtag_masters[id];
  itf->tdo = tdo;
}

void dpi_wrapper::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);

  void *config_handle = dpi_config_get_from_file(getenv("PULP_CONFIG_FILE"));

  if (config_handle == NULL) return;

  void *driver_handle = dpi_driver_set_config(config_handle);
  int nb_comp = dpi_driver_get_nb_comp(driver_handle);

  for(int i = 0; i < nb_comp; i++)
  {
    const char *comp_name = dpi_driver_get_comp_name(driver_handle, i);
    void *comp_config = dpi_driver_get_comp_config(driver_handle, i);
    const char *comp_type = dpi_config_get_str(dpi_config_get_config(comp_config, "type"));
    int nb_itf = dpi_driver_get_comp_nb_itf(comp_config, i);

    //printf("Found TB driver component (index: %d, name: %s, type: %s)\n", i, comp_name, comp_type);

    if (strcmp(comp_type, "dpi") == 0)
    {
//      dpi_models::periph_wrapper i_comp = new();
//      int err;
//
//      $display("[TB] %t - Instantiating DPI component", $realtime, i);

      void *dpi_model = dpi_model_load(comp_config, (void *)this);
//      err = i_comp.load_model(comp_config);
//      if (err != 0) $fatal(1, "[TB] %t - Failed to instantiate periph model", $realtime);
//
      models.push_back(dpi_model);

      for(int j = 0; j < nb_itf; j++)
      {
        const char *itf_type;
        const char *itf_name;
        int itf_id;
        int itf_sub_id;
        dpi_driver_get_comp_itf_info(comp_config, i, j, &itf_name, &itf_type, &itf_id, &itf_sub_id);
        //printf("Got interface information (index: %d, name: %s, type: %s, id: %d, sub_id: %d)\n", i, itf_name, itf_type, itf_id, itf_sub_id);

        if (strcmp(itf_type, "QSPIM") == 0)
        {
//            i_comp.qpim_bind(itf_name, qspi_infos[itf_id].itf, qspi_infos[itf_id].cs[itf_sub_id]);
        }
        else if (strcmp(itf_type, "JTAG") == 0)
        {
          vp::jtag_master *itf = new vp::jtag_master();
          itf->set_sync_meth_muxed(&dpi_wrapper::jtag_sync, itf_id);
          new_master_port(itf_name + std::to_string(itf_id), itf);
          jtag_masters.push_back(itf);
          dpi_jtag_bind(dpi_model, itf_name, jtag_masters.size()-1);

        }
        else if (strcmp(itf_type, "CTRL") == 0)
        {
//            i_comp.ctrl_bind(itf_name, ctrl_infos[itf_id].itf);
        }
      }
    }
  }
}

void dpi_wrapper::start()
{
  for (int i=0; i<models.size(); i++)
  {
    dpi_model_start(models[i]);
  }
}

extern "C" void dpi_ctrl_reset_edge(void *handle, int reset)
{
}

extern "C" void dpi_jtag_tck_edge(void *handle, int tck, int tdi, int tms, int trst, int *tdo)
{
  vp::jtag_master *itf = jtag_masters[(int)(long)handle];
  itf->sync(tck, tdi, tms, trst);
  *tdo = itf->tdo;
}

extern "C" void dpi_print(void *data, const char *msg)
{
  fwrite(msg, 1, strlen(msg), stdout);
  printf("\n");
}

extern "C" void dpi_wait_event(void *handle)
{
  dpi_wrapper *dpi = (dpi_wrapper *)handle;
  return dpi->wait_event();
}

extern "C" void dpi_raise_event(void *handle)
{
  dpi_wrapper *dpi = (dpi_wrapper *)handle;
  return dpi->raise_event();
}

extern "C" int dpi_wait(void *handle, int64_t t)
{
  dpi_wrapper *dpi = (dpi_wrapper *)handle;
  return dpi->wait(t);
}

extern "C" int dpi_wait_ps(void *handle, int64_t t)
{
  dpi_wrapper *dpi = (dpi_wrapper *)handle;
  return dpi->wait_ps(t);
}


extern "C" void dpi_qspim_set_data(void *handle, int data_0, int data_1, int data_2, int data_3)
{
  printf("UNIMPLEMENTED AT %s %d\n", __FILE__, __LINE__);
  exit(-1);
}

extern "C" void dpi_create_task(void *handle, void *arg1, void *arg2)
{
  dpi_wrapper *dpi = (dpi_wrapper *)handle;
  dpi->create_task(arg1, arg2);
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new dpi_wrapper(config);
}