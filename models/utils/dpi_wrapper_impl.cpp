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
#include "vp/itf/uart.hpp"
#include <vector>
#include <thread>
#include <unistd.h>

typedef struct {
  void *handle;
  vp::trace tx_trace;
} uart_handle_t;

static thread_local vp::clock_event *wait_evt;
static thread_local bool is_waiting = false;
static pthread_mutex_t dpi_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static vector<vp::qspim_slave *> qspim_slaves;
static vector<vp::jtag_master *> jtag_masters;
static vector<vp::uart_slave *> uart_slaves;
static vector<uart_handle_t *> uart_handles;

class dpi_wrapper : public vp::component
{

public:

  dpi_wrapper(const char *config);

  int build();
  void start();
  void create_task(int id);
  int wait(int64_t t);
  int wait_ps(int64_t t);
  void wait_event();
  void raise_event();
  vp::trace *get_trace() { return &trace; }

private:

  void task_thread_routine(int id);
  static void wait_handler(void *__this, vp::clock_event *event);
  static void jtag_sync(void *__this, int tdo, int id);
  static void uart_sync(void *__this, int data, int id);

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
  _this->get_trace()->msg("HANDLER\n");
  pthread_mutex_lock(&dpi_mutex);
  *(bool *)(event->get_args()[0]) = false;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&dpi_mutex);
  _this->get_clock()->get_engine()->lock_step();
}

void dpi_wrapper::task_thread_routine(int id)
{
  this->get_clock()->get_engine()->lock();
  this->get_clock()->retain();
  wait_evt = event_new(dpi_wrapper::wait_handler);
  wait_evt->get_args()[0] = &is_waiting;
  dpi_start_task(id);
  this->get_clock()->release();
}

int dpi_wrapper::wait(int64_t t)
{
  int64_t period = get_period();
  int64_t cycles = (t*1000 + period - 1) / period;
  event_enqueue(wait_evt, cycles);
  is_waiting = true;
  get_clock()->get_engine()->unlock();

  pthread_mutex_lock(&dpi_mutex);
  while(is_waiting)
  {
    get_clock()->get_engine()->lock_step_cancel();
    pthread_cond_wait(&cond, &dpi_mutex);
  }
  pthread_mutex_unlock(&dpi_mutex);

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

  pthread_mutex_lock(&dpi_mutex);
  while(is_waiting)
  {
    get_clock()->get_engine()->lock_step_cancel();
    pthread_cond_wait(&cond, &dpi_mutex);
  }
  pthread_mutex_unlock(&dpi_mutex);

  get_clock()->get_engine()->lock();
  return 0;
}

void dpi_wrapper::wait_event()
{
  get_clock()->get_engine()->unlock();
  pthread_mutex_lock(&dpi_mutex);
  while(!event_raised)
  {
    get_clock()->get_engine()->lock_step_cancel();
    pthread_cond_wait(&cond, &dpi_mutex);
  }
  event_raised = false;
  pthread_mutex_unlock(&dpi_mutex);
  get_clock()->get_engine()->lock();
}

void dpi_wrapper::raise_event()
{
  pthread_mutex_lock(&dpi_mutex);
  event_raised = true;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&dpi_mutex);
  this->get_clock()->get_engine()->lock_step();
}

void dpi_wrapper::create_task(int id)
{
  threads.push_back(new std::thread(&dpi_wrapper::task_thread_routine, this, id));
}

void dpi_wrapper::jtag_sync(void *__this, int tdo, int id)
{
  vp::jtag_master *itf = jtag_masters[id];
  itf->tdo = tdo;
}

void dpi_wrapper::uart_sync(void *__this, int data, int id)
{
  dpi_wrapper *_this = (dpi_wrapper *)__this;
  uart_handles[id]->tx_trace.event((uint8_t *)&data);
  dpi_uart_edge(uart_handles[id]->handle, _this->get_clock()->get_time(), data);
}

int dpi_wrapper::build()
{
  traces.new_trace("trace", &trace, vp::DEBUG);

  void *config_handle = dpi_config_get_from_file(getenv("PULP_CONFIG_FILE"));

  if (config_handle == NULL) return 0;

  void *driver_handle = dpi_driver_set_config(config_handle);
  int nb_comp = dpi_driver_get_nb_comp(driver_handle);

  for(int i = 0; i < nb_comp; i++)
  {
    const char *comp_name = dpi_driver_get_comp_name(driver_handle, i);
    void *comp_config = dpi_driver_get_comp_config(driver_handle, i);
    const char *comp_type = dpi_config_get_str(dpi_config_get_config(comp_config, "type"));
    int nb_itf = dpi_driver_get_comp_nb_itf(comp_config, i);

    this->trace.msg("Found TB driver component (index: %d, name: %s, type: %s, nb_itf: %d)\n", i, comp_name, comp_type, nb_itf);



    if (strcmp(comp_type, "dpi") == 0)
    {
//      dpi_models::periph_wrapper i_comp = new();
//      int err;
//
//      $display("[TB] %t - Instantiating DPI component", $realtime, i);


      this->trace.msg("Instantiating DPI component\n");

      void *dpi_model = dpi_model_load(comp_config, (void *)this);
      if (dpi_model == NULL)
      {
        snprintf(vp_error, VP_ERROR_SIZE, "Failed to instantiate DPI model\n");
        return -1;
      }
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

        this->trace.msg("Got interface information (index: %d, name: %s, type: %s, id: %d, sub_id: %d)\n", j, itf_name, itf_type, itf_id, itf_sub_id);

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
          void *handle = dpi_jtag_bind(dpi_model, itf_name, jtag_masters.size()-1);
          if (handle == NULL)
          {
            snprintf(vp_error, VP_ERROR_SIZE, "Failed to bind JTAG interface (name: %s)\n", itf_name);
            return -1;
          }

        }
        else if (strcmp(itf_type, "UART") == 0)
        {
          vp::uart_slave *itf = new vp::uart_slave();
          itf->set_sync_meth_muxed(&dpi_wrapper::uart_sync, itf_id);
          new_slave_port(itf_name + std::to_string(itf_id), itf);
          uart_slaves.push_back(itf);
          uart_handle_t *handle = new uart_handle_t;
          handle->handle = dpi_uart_bind(dpi_model, itf_name, uart_handles.size());
          if (handle->handle == NULL)
          {
            snprintf(vp_error, VP_ERROR_SIZE, "Failed to bind UART interface (name: %s)\n", itf_name);
            return -1;
          }
          uart_handles.push_back(handle);
          traces.new_trace_event(itf_name + std::to_string(itf_id) + "/tx", &handle->tx_trace, 1);

        }
        else if (strcmp(itf_type, "CTRL") == 0)
        {
//            i_comp.ctrl_bind(itf_name, ctrl_infos[itf_id].itf);
        }
      }
    }
  }

  return 0;
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
  if (itf->is_bound())
  {
    itf->sync(tck, tdi, tms, trst);
    *tdo = itf->tdo;
  }
}

extern "C" void dpi_uart_rx_edge(void *handle, int data)
{
  vp::uart_slave *itf = uart_slaves[(int)(long)handle];
  itf->sync(data);
}

extern "C" void dpi_print(void *data, const char *msg)
{
  dpi_wrapper *_this = (dpi_wrapper *)data;
  int len = strlen(msg);
  char buff[len + 2];
  strcpy(buff, msg);
  buff[len] = '\n';
  buff[len + 1] = 0;
  _this->get_trace()->msg(buff);
}

extern "C" void dpi_fatal(void *data, const char *msg)
{
  dpi_wrapper *_this = (dpi_wrapper *)data;
  int len = strlen(msg);
  char buff[len + 2];
  strcpy(buff, msg);
  buff[len] = '\n';
  buff[len + 1] = 0;
  _this->get_trace()->fatal(buff);
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

extern "C" void dpi_create_task(void *handle, int id)
{
  dpi_wrapper *dpi = (dpi_wrapper *)handle;
  dpi->create_task(id);
}

extern "C" void *vp_constructor(const char *config)
{
  return (void *)new dpi_wrapper(config);
}
