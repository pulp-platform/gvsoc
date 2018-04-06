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

#ifndef __VP_TIME_ENGINE_HPP__
#define __VP_TIME_ENGINE_HPP__

#include "vp/vp.hpp"
#include "vp/component.hpp"


namespace vp {

  class time_engine_client;

  class time_engine : public component {
  public:
    time_engine(const char *config);

    void start();

    void run_loop();

    string run();

    int run_status() { return stop_status; }

    inline void lock();

    inline void unlock();

    inline void stop();

    inline void stop(int status);

    void enqueue(time_engine_client *client, int64_t time);

    int64_t get_time() { return time; }

    inline void retain() { retain_count++; }
    inline void release() { retain_count--; }

  private:
    time_engine_client *first_client = NULL;
    bool locked = false;
    bool locked_run_req;
    bool run_req;
    bool stop_req;
    bool finished = false;




    bool running;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t run_thread;

    int64_t time = 0;
    int stop_status = -1;
    int retain_count = 0;
  };

  class time_engine_client : public component {

    friend class time_engine;

  public:
    time_engine_client(const char *config)
      : vp::component(config)
    {
    }

    inline bool is_running() { return running; }

    inline void enqueue_to_engine(int64_t time)
    {
      engine->enqueue(this, time);
    }

    inline int64_t get_time() { return engine->get_time(); }

    virtual int64_t exec() = 0;

  protected:
    time_engine_client *next;
    int64_t next_event_time = 0;
    vp::time_engine *engine;
    bool running = false;
  };


  // This can be called from anywhere so just propagate the stop request
  // to the main python thread which will take care of stopping the engine.
  inline void vp::time_engine::stop()
  {
    pthread_mutex_lock(&mutex);
    stop_req = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
  }

  inline void vp::time_engine::stop(int status)
  {
    stop_status = status;
    stop();
  }

  inline void vp::time_engine::lock()
  {
    pthread_mutex_lock(&mutex);
    locked = true;
    locked_run_req = run_req;
    run_req = false;
    pthread_cond_broadcast(&cond);
    while (running) pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
  }

  inline void vp::time_engine::unlock()
  {
    pthread_mutex_lock(&mutex);
    run_req = locked_run_req;
    locked = false;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
  }


};

#endif
