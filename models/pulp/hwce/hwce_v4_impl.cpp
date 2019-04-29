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
#include <archi/hwce/hwce_v4.h>

#define NB_MASTER_PORTS 4

#define HWCE_STATE_IDLE         0
#define HWCE_STATE_CONTEXT_COPY 1
#define HWCE_STATE_LOCKED       2

#define HWCE_JOBQUEUE_IDLE             0
#define HWCE_JOBQUEUE_FETCH_WEIGHTS    1
#define HWCE_JOBQUEUE_EXEC_CONV        3


static string get_state_name(int state) {
  switch (state) {
    case HWCE_STATE_IDLE: return "idle";
    case HWCE_STATE_CONTEXT_COPY: return "contextCopy";
    case HWCE_STATE_LOCKED: return "locked";
  }
  return "unknown";
}

class hwce;

class hwce_job_t {

public:
  hwce_job_t *next;
  int id;
  int run_id;

  vp_hwce_y_trans_size              r_y_trans_size;
  vp_hwce_y_line_stride_length      r_y_line_stride_length;
  vp_hwce_y_feat_stride_length      r_y_feat_stride_length;
  vp_hwce_y_out_3_base_addr         r_y_out_3_base_addr;
  vp_hwce_y_out_2_base_addr         r_y_out_2_base_addr;
  vp_hwce_y_out_1_base_addr         r_y_out_1_base_addr;
  vp_hwce_y_out_0_base_addr         r_y_out_0_base_addr;
  vp_hwce_y_in_3_base_addr          r_y_in_3_base_addr;
  vp_hwce_y_in_2_base_addr          r_y_in_2_base_addr;
  vp_hwce_y_in_1_base_addr          r_y_in_1_base_addr;
  vp_hwce_y_in_0_base_addr          r_y_in_0_base_addr;
  vp_hwce_x_trans_size              r_x_trans_size;
  vp_hwce_x_line_stride_length      r_x_line_stride_length;
  vp_hwce_x_feat_stride_length      r_x_feat_stride_length;
  vp_hwce_x_in_base_addr            r_x_in_base_addr;
  vp_hwce_w_base_addr               r_w_base_addr;
  vp_hwce_job_config0               r_job_config0;
  vp_hwce_job_config1               r_job_config1;
  vp_hwce_y_trans_size_ctx0         r_y_trans_size_ctx0;
  vp_hwce_y_line_stride_length_ctx0 r_y_line_stride_length_ctx0;
  vp_hwce_y_feat_stride_length_ctx0 r_y_feat_stride_length_ctx0;
  vp_hwce_y_out_3_base_addr_ctx0    r_y_out_3_base_addr_ctx0;
  vp_hwce_y_out_2_base_addr_ctx0    r_y_out_2_base_addr_ctx0;
  vp_hwce_y_out_1_base_addr_ctx0    r_y_out_1_base_addr_ctx0;
  vp_hwce_y_out_0_base_addr_ctx0    r_y_out_0_base_addr_ctx0;
  vp_hwce_y_in_3_base_addr_ctx0     r_y_in_3_base_addr_ctx0;
  vp_hwce_y_in_2_base_addr_ctx0     r_y_in_2_base_addr_ctx0;
  vp_hwce_y_in_1_base_addr_ctx0     r_y_in_1_base_addr_ctx0;
  vp_hwce_y_in_0_base_addr_ctx0     r_y_in_0_base_addr_ctx0;
  vp_hwce_x_trans_size_ctx0         r_x_trans_size_ctx0;
  vp_hwce_x_line_stride_length_ctx0 r_x_line_stride_length_ctx0;
  vp_hwce_x_feat_stride_length_ctx0 r_x_feat_stride_length_ctx0;
  vp_hwce_x_in_base_addr_ctx0       r_x_in_base_addr_ctx0;
  vp_hwce_w_base_addr_ctx0          r_w_base_addr_ctx0;
  vp_hwce_job_config0_ctx0          r_job_config0_ctx0;
  vp_hwce_job_config1_ctx0          r_job_config1_ctx0;
  vp_hwce_y_trans_size_ctx1         r_y_trans_size_ctx1;
  vp_hwce_y_line_stride_length_ctx1 r_y_line_stride_length_ctx1;
  vp_hwce_y_feat_stride_length_ctx1 r_y_feat_stride_length_ctx1;
  vp_hwce_y_out_3_base_addr_ctx1    r_y_out_3_base_addr_ctx1;
  vp_hwce_y_out_2_base_addr_ctx1    r_y_out_2_base_addr_ctx1;
  vp_hwce_y_out_1_base_addr_ctx1    r_y_out_1_base_addr_ctx1;
  vp_hwce_y_out_0_base_addr_ctx1    r_y_out_0_base_addr_ctx1;
  vp_hwce_y_in_3_base_addr_ctx1     r_y_in_3_base_addr_ctx1;
  vp_hwce_y_in_2_base_addr_ctx1     r_y_in_2_base_addr_ctx1;
  vp_hwce_y_in_1_base_addr_ctx1     r_y_in_1_base_addr_ctx1;
  vp_hwce_y_in_0_base_addr_ctx1     r_y_in_0_base_addr_ctx1;
  vp_hwce_x_trans_size_ctx1         r_x_trans_size_ctx1;
  vp_hwce_x_line_stride_length_ctx1 r_x_line_stride_length_ctx1;
  vp_hwce_x_feat_stride_length_ctx1 r_x_feat_stride_length_ctx1;
  vp_hwce_x_in_base_addr_ctx1       r_x_in_base_addr_ctx1;
  vp_hwce_w_base_addr_ctx1          r_w_base_addr_ctx1;
  vp_hwce_job_config0_ctx1          r_job_config0_ctx1;
  vp_hwce_job_config1_ctx1          r_job_config1_ctx1;

  int wstride;
};


class Hwce_base {
public:
  Hwce_base(hwce *top);
  void init(uint32_t base, int lineLen, int lineStride, int featLen, int featStride, int nbFeatures, int featLoop);
  uint32_t get();
  void update();
  bool reachedEof();
  void startFeature();

private:
  hwce *top;
  uint32_t baseStart;
  uint32_t baseLoop;
  uint32_t base;
  int lineLen;
  int lineStride;
  int featLen;
  int featStride;
  int wordCount;
  int lineCount;
  int featCount;
  int featLoopCount;
  int nbFeatures;
  int featLoop;
  bool pendingEof;
};


class hwce : public vp::component
{
  friend class Hwce_base;


public:

  hwce(const char *config);

  int build();
  void start();
  void reset(bool active);

private:

  static vp::io_req_status_e req(void *__this, vp::io_req *req);

  void set_state(int new_state);
  void enqueue_job();
  int alloc_job();
  static void job_queue_handle(void *__this, vp::clock_event *event);
  static void ctrl_handle(void *__this, vp::clock_event *event);
  vp::io_req_status_e req_trigger(int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_acquire(int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_finished_jobs(int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_status(int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_running_job(int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_soft_clear(int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_gen_config0(int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_gen_config1(int reg_offset, int size, uint8_t *data, bool is_write);

  vp::io_req_status_e req_y_trans_size(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_line_stride_length(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_feat_stride_length(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_out_3_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_out_2_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_out_1_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_out_0_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_in_3_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_in_2_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_in_1_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_y_in_0_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_x_trans_size(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_x_line_stride_length(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_x_feat_stride_length(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_x_in_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_w_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_job_config0(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);
  vp::io_req_status_e req_job_config1(int job_id, int reg_offset, int size, uint8_t *data, bool is_write);


  void start_job(hwce_job_t *job);

  vp::trace     trace;

  vp::io_slave          in;
  vp::io_master         out[NB_MASTER_PORTS];
  vp::wire_master<bool> irq;

  vp::clock_event *job_queue_event;
  vp::clock_event *ctrl_event;
  
  vp_hwce_trigger                   r_trigger;
  vp_hwce_acquire                   r_acquire;
  vp_hwce_finished_jobs             r_finished_jobs;
  vp_hwce_status                    r_status;
  vp_hwce_running_job               r_running_job;
  vp_hwce_soft_clear                r_soft_clear;
  vp_hwce_gen_config0               r_gen_config0;
  vp_hwce_gen_config1               r_gen_config1;

  hwce_job_t jobs[2];

  int current_job;
  int state;
  int job_queue_state;
  unsigned int free_jobs;
  int job_id;

  uint32_t weights_base;
  uint32_t x_in_size;
  uint32_t y_in_size;
  uint32_t x_out_size;
  uint32_t x_size;
  uint32_t x_base;
  int      y_in_take_first;
  int      y_out_take_first;

  hwce_job_t *first_job;
  hwce_job_t *last_job;

  hwce_job_t *pending_job;
  hwce_job_t *prev_job;
  hwce_job_t *allocated_job;

  Hwce_base *xinBase;
  Hwce_base *youtBase[4];
  Hwce_base *yinBase[4];

};

hwce::hwce(const char *config)
: vp::component(config)
{

}

void hwce::reset(bool active)
{
  if (active)
  {
    this->current_job = 0;
    this->state = HWCE_STATE_IDLE;

    this->first_job = NULL;
    this->last_job = NULL;
    this->prev_job = NULL;
    this->pending_job = NULL;
    this->allocated_job = NULL;

    this->free_jobs = 0x3;
    this->job_id = 0;
  }
}



int hwce::alloc_job()
{
  if (this->free_jobs == 0) return -1;

  for (int i=0; i<32; i++)
  {
    if ((this->free_jobs >> i) & 1)
    {
      this->trace.msg("Allocated job (jobId: %d)\n", i);
      this->free_jobs &= ~(1<<i);
      this->current_job = i;
      this->jobs[i].run_id = this->job_id++;
      if (this->job_id == 256) this->job_id = 0;
      return i;
    }
  }
  return -1;
}



void hwce::start_job(hwce_job_t *job)
{
  int busy = 1;
  //gv_vcd_dump(&vcdBusy, (uint8_t *)&busy);

  job->wstride = this->r_gen_config0.wstride_get();

  this->pending_job = job;
  this->job_queue_state = HWCE_JOBQUEUE_FETCH_WEIGHTS;

  this->weights_base = job->r_w_base_addr.addr_get();
  this->x_in_size = job->r_x_trans_size.size_get();
  this->x_out_size = job->r_y_trans_size.size_get()*(1<<this->r_gen_config0.vect_get());
  this->y_in_size = job->r_y_trans_size.size_get()*(1<<this->r_gen_config0.vect_get());
  this->y_in_take_first = 1;
  this->y_out_take_first = 1;

  //this->xinBase->init(job->xinBase, job->xinLineLength, job->xinLineStride, job->xinFeatLength, job->xinFeatStride, job->nif, 0);

#if 0
  for (int i=0; i<genConfig0_nbVect; i++) {
    yinBase[i]->init(job->yinBase[i], job->yLineLength, job->yLineStride, job->yFeatLength, job->yFeatStride, job->wof, job->nif);
    youtBase[i]->init(job->youtBase[i], job->yLineLength, job->yLineStride, job->yFeatLength, job->yFeatStride, job->wof, job->nif);
  }

  nbYoutValue = 0;
  youtValid = 0;

  lineBufferCurrentWidth = lineBufferWidth;
  if (lineBufferCurrentWidth > job->xinLineLength) lineBufferCurrentWidth = job->xinLineLength;
  nbReadyLines = 0;
  nbReadyWords = 0;
  convCurrentPosition = 0;
  gv_trace_dumpMsg(&trace, "Starting job (xoutSize: 0x%x, xinBase: 0x%x, youtBase: 0x%x, xinSize: 0x%x, xinLineStride: 0x%x, xinLineLen: 0x%x, xinFeatStride: 0x%x, xinFeatLen: 0x%x, )\n", xoutSize, xinBase->get(), youtBase[0]->get(), xinSize, job->xinLineStride, job->xinLineLength, job->xinFeatStride, job->xinFeatLength);  

  if (xoutSize == 0) gv_trace_dumpWarning(&warning, "Trying to start job with 0 output size\n");
  if (xinSize == 0) gv_trace_dumpWarning(&warning, "Trying to start job with 0 input size\n");

  if (!jobQueueEvent->enqueued) jobQueueEvent->enqueue(getEngine()->getCycles() + 1);
#endif
}



void hwce::set_state(int new_state)
{
  this->trace.msg("Setting new state (new_state: %s)\n", get_state_name(new_state).c_str());
  this->state = new_state;
}



vp::io_req_status_e hwce::req_trigger(int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->r_trigger.access(reg_offset, size, data, is_write);

  if (is_write)
  {
    if (this->state != HWCE_STATE_LOCKED)
    {
      this->warning.force_warning("Trying to trigger job while it is not locked (current_state: %s)\n", get_state_name(state).c_str());
      return vp::IO_REQ_INVALID;
    }

    this->enqueue_job();
    this->set_state(HWCE_STATE_IDLE);
  }

  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_acquire(int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->r_acquire.access(reg_offset, size, data, is_write);

  if (!is_write)
  {
    #if 0
    switch (this->state) {
      case HWCE_STATE_IDLE: {
        int job = this->alloc_job();
        if (job == -1) {
          *data = HWCE_ACQUIRE_QUEUE_FULL;
          gv_trace_dumpMsg(&trace, "Trying to acquire job while job queue is full\n");
        } else {
          *data = jobs[job].runId;
          setState(HWCE_STATE_LOCKED);
          prevJob = allocatedJob;
          allocatedJob = &jobs[job];
          // If this is enabled perform the context copy from the old one to the new one
          // This will change the state bacl to LOCKED in a few cycles (one register copy per cycle)
          if (!genConfig0_ncp) {
            setState(HWCE_STATE_CONTEXT_COPY);
            event->enqueue(getEngine()->getCycles() + HWCE_NB_IO_REGS + 1);
          }
        }
      }
      break;
      case HWCE_STATE_CONTEXT_COPY:
      *data = HWCE_ACQUIRE_CONTEXT_COPY;
      break;
      case HWCE_STATE_LOCKED:
      *data = HWCE_ACQUIRE_LOCKED;
      break;
    }
    #endif
  }

  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_finished_jobs(int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->r_finished_jobs.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_status(int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->r_status.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_running_job(int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->r_running_job.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_soft_clear(int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->r_soft_clear.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_gen_config0(int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->r_gen_config0.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_gen_config1(int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->r_gen_config1.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_trans_size(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_trans_size.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_line_stride_length(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_line_stride_length.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_feat_stride_length(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_feat_stride_length.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_out_3_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_out_3_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_out_2_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_out_2_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_out_1_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_out_1_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_out_0_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_out_0_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_in_3_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_in_3_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_in_2_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_in_2_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_in_1_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_in_1_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_y_in_0_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_y_in_0_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_x_trans_size(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_x_trans_size.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_x_line_stride_length(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_x_line_stride_length.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_x_feat_stride_length(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_x_feat_stride_length.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_x_in_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_x_in_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_w_base_addr(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_w_base_addr.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_job_config0(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_job_config0.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req_job_config1(int job_id, int reg_offset, int size, uint8_t *data, bool is_write)
{
  this->jobs[job_id].r_job_config1.access(reg_offset, size, data, is_write);
  return vp::IO_REQ_OK;
}



vp::io_req_status_e hwce::req(void *__this, vp::io_req *req)
{
  hwce *_this = (hwce *)__this;
  vp::io_req_status_e err = vp::IO_REQ_INVALID;

  uint64_t offset = req->get_addr();
  uint8_t *data = req->get_data();
  uint64_t size = req->get_size();
  bool is_write = req->get_is_write();

  _this->trace.msg("HWCE access (offset: 0x%x, size: 0x%x, is_write: %d)\n", offset, size, req->get_is_write());

  if (offset < HWCE_Y_TRANS_SIZE_OFFSET)
  {

    // Global registers access

    int reg_id = offset / 4;
    int reg_offset = offset % 4;

    switch (reg_id)
    {
      case HWCE_TRIGGER_OFFSET/4:
        err = _this->req_trigger(reg_offset, size, data, is_write);
        break;

      case HWCE_ACQUIRE_OFFSET/4:
        err = _this->req_acquire(reg_offset, size, data, is_write);
        break;

      case HWCE_FINISHED_JOBS_OFFSET/4:
        err = _this->req_finished_jobs(reg_offset, size, data, is_write);
        break;

      case HWCE_STATUS_OFFSET/4:
        err = _this->req_status(reg_offset, size, data, is_write);
        break;

      case HWCE_RUNNING_JOB_OFFSET/4:
        err = _this->req_running_job(reg_offset, size, data, is_write);
        break;

      case HWCE_SOFT_CLEAR_OFFSET/4:
        err = _this->req_soft_clear(reg_offset, size, data, is_write);
        break;

      case HWCE_GEN_CONFIG0_OFFSET/4:
        err = _this->req_gen_config0(reg_offset, size, data, is_write);
        break;

      case HWCE_GEN_CONFIG1_OFFSET/4:
        err = _this->req_gen_config1(reg_offset, size, data, is_write);
        break;

    }
  }
  else
  {
    int job;
    if (offset < HWCE_Y_TRANS_SIZE_CTX0_OFFSET)
    {
      job = _this->current_job;
    }
    else if (offset < HWCE_Y_TRANS_SIZE_CTX1_OFFSET)
    {
      job = 0;
      offset = offset - HWCE_Y_TRANS_SIZE_CTX0_OFFSET + HWCE_Y_TRANS_SIZE_OFFSET;
    }
    else
    {
      job = 1;
      offset = offset - HWCE_Y_TRANS_SIZE_CTX1_OFFSET + HWCE_Y_TRANS_SIZE_OFFSET;
    }

    int reg_id = offset / 4;
    int reg_offset = offset % 4;

    switch (reg_id)
    {
      case HWCE_Y_TRANS_SIZE_OFFSET/4:
        err = _this->req_y_trans_size(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_LINE_STRIDE_LENGTH_OFFSET/4:
        err = _this->req_y_line_stride_length(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_FEAT_STRIDE_LENGTH_OFFSET/4:
        err = _this->req_y_feat_stride_length(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_OUT_3_BASE_ADDR_OFFSET/4:
        err = _this->req_y_out_3_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_OUT_2_BASE_ADDR_OFFSET/4:
        err = _this->req_y_out_2_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_OUT_1_BASE_ADDR_OFFSET/4:
        err = _this->req_y_out_1_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_OUT_0_BASE_ADDR_OFFSET/4:
        err = _this->req_y_out_0_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_IN_3_BASE_ADDR_OFFSET/4:
        err = _this->req_y_in_3_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_IN_2_BASE_ADDR_OFFSET/4:
        err = _this->req_y_in_2_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_IN_1_BASE_ADDR_OFFSET/4:
        err = _this->req_y_in_1_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_Y_IN_0_BASE_ADDR_OFFSET/4:
        err = _this->req_y_in_0_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_X_TRANS_SIZE_OFFSET/4:
        err = _this->req_x_trans_size(job, reg_offset, size, data, is_write);
        break;

      case HWCE_X_LINE_STRIDE_LENGTH_OFFSET/4:
        err = _this->req_x_line_stride_length(job, reg_offset, size, data, is_write);
        break;

      case HWCE_X_FEAT_STRIDE_LENGTH_OFFSET/4:
        err = _this->req_x_feat_stride_length(job, reg_offset, size, data, is_write);
        break;

      case HWCE_X_IN_BASE_ADDR_OFFSET/4:
        err = _this->req_x_in_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_W_BASE_ADDR_OFFSET/4:
        err = _this->req_w_base_addr(job, reg_offset, size, data, is_write);
        break;

      case HWCE_JOB_CONFIG0_OFFSET/4:
        err = _this->req_job_config0(job, reg_offset, size, data, is_write);
        break;

      case HWCE_JOB_CONFIG1_OFFSET/4:
        err = _this->req_job_config1(job, reg_offset, size, data, is_write);
        break;
    }
  }

  return err;
}



void hwce::job_queue_handle(void *__this, vp::clock_event *event)
{
  hwce *_this = (hwce *)__this;

  if (_this->job_queue_state == HWCE_JOBQUEUE_EXEC_CONV)
  {
#if 0
    // The order is important as the FIFOs are not modeled
    // The output is first flushed to let convolution executes and stores a new result
    youtFlush();
    // The convolution is executed before fetching data to compute only the cycle after the data has been received
    execConvolution();
    fetchXin();
    fetchYin();

    bool reachedEof = true;
    for (int i=0; i<genConfig0_nbVect; i++) {
      reachedEof &= youtBase[i]->reachedEof();
    }

    if (reachedEof && xoutSize) {
      // In case we are waiting for convolotion termination, change the state once there is nothing
      gv_trace_dumpMsg(&trace, "Detected end of feature, switching to weight fetch\n");
      // to execute and everything is flushed
      for (int i=0; i<genConfig0_nbVect; i++) {
        youtBase[i]->startFeature();
        yinBase[i]->startFeature();
      }
      xinBase->startFeature();
      jobQueueState = HWCE_JOBQUEUE_FETCH_WEIGHTS;
      nbReadyLines = 0;
      weightsBase = weightsBase - 4*13 + pendingJob->wstride;
    }

    // TODO include latency from access
    if (!jobQueueEvent->enqueued) jobQueueEvent->enqueue(getEngine()->getCycles() + 1);
#endif
  }
  else if (_this->job_queue_state == HWCE_JOBQUEUE_FETCH_WEIGHTS)
  {
  #if 0
    gv_trace_dumpMsg(&trace, "Fetching weight (addr: 0x%x)\n", weightsBase);

    uint32_t data;
    gv::ioReq req(this, weightsBase, (uint8_t *)&data, 4, 1);
    if (outPorts[0]->ioReq(&req)) {
      gv_trace_dumpWarning(&warning, "Got bus error while fetching weights\n");
    }

    ((uint32_t *)weights)[nbValidWeights/2] = data;
    gv_trace_dumpMsg(&trace, "Fetched weight (value: 0x%x, latency: %d)\n", data, req.getLatency());

    nbValidWeights += 2;
    weightsBase += 4;
    if (nbValidWeights == expectedWeights) {
      // The HWCE is using flipped kernels, do it now to keep clean Convolution
      if (!genConfig0_nf) {
        int16_t tmp[filterSizeX*filterSizeY];
        memcpy(tmp, weights, filterSizeX*filterSizeY*2);
        for (int i=0; i<filterSizeX*filterSizeY; i++) {
          weights[i] = tmp[filterSizeX*filterSizeY-1 - i];
        }
      }
      nbValidWeights = 0;
      nbValidXin = 0;
      jobQueueState = HWCE_JOBQUEUE_EXEC_CONV;
    }
    // TODO include latency from access
    if (!jobQueueEvent->enqueued) jobQueueEvent->enqueue(getEngine()->getCycles() + 1);
    #endif
  }
  else if (_this->job_queue_state == HWCE_JOBQUEUE_IDLE && _this->first_job)
  {
    hwce_job_t *job = _this->first_job;
    _this->trace.msg("Popping job from queue (job: %d)\n", job->id);
    _this->first_job = job->next;
    if (_this->first_job == NULL)
      _this->last_job = NULL;
    _this->start_job(job);
  }
}



void hwce::enqueue_job()
{
  hwce_job_t *job = &jobs[this->current_job];
  this->trace.msg("Enqueuing job (job_id: %d)\n", job->id);

  if (this->last_job) this->last_job->next = job;
  else this->first_job = job;
  job->next = NULL;
  if (!this->job_queue_event->is_enqueued())
  this->event_enqueue(this->job_queue_event, 1);
}



void hwce::ctrl_handle(void *__this, vp::clock_event *event)
{
  hwce *_this = (hwce *)__this;
  
  switch (_this->state)
  {
    case HWCE_STATE_CONTEXT_COPY:
    _this->trace.msg("Finished context copy\n");
    _this->set_state(HWCE_STATE_LOCKED);
    if (_this->prev_job) {
      //memcpy(_this->allocated_job->regs, _this->prev_job->regs, sizeof(_this->prev_job->regs));
      //for (int i=0; i<HWCE_NB_IO_REGS; i++) {
       // updateJobReg(allocatedJob, i, allocatedJob->regs[i]);
      //}
    }
    break;
  }
}



int hwce::build()
{
  this->traces.new_trace("trace", &this->trace, vp::DEBUG);

  this->in.set_req_meth(&hwce::req);
  this->new_slave_port("input", &this->in);

  for (int i=0; i<NB_MASTER_PORTS; i++)
  {
    this->new_master_port("out" + std::to_string(i), &this->out[i]);
  }

  this->new_master_port("irq", &this->irq);

  this->ctrl_event = this->event_new(&hwce::ctrl_handle);
  this->job_queue_event = this->event_new(&hwce::job_queue_handle);

  this->new_reg("trigger", &this->r_trigger, 0);
  this->new_reg("acquire", &this->r_acquire, 0);
  this->new_reg("finished_jobs", &this->r_finished_jobs, 0);
  this->new_reg("status", &this->r_status, 0);
  this->new_reg("running_job", &this->r_running_job, 0);
  this->new_reg("soft_clear", &this->r_soft_clear, 0);
  this->new_reg("gen_config0", &this->r_gen_config0, 0);
  this->new_reg("gen_config1", &this->r_gen_config1, 0);

  for (int i=0; i<2; i++)
  {
    this->new_reg("y_trans_size_" + std::to_string(i), &this->jobs[i].r_y_trans_size, 0);
    this->new_reg("y_line_stride_length_" + std::to_string(i), &this->jobs[i].r_y_line_stride_length, 0);
    this->new_reg("y_feat_stride_length_" + std::to_string(i), &this->jobs[i].r_y_feat_stride_length, 0);
    this->new_reg("y_out_3_base_addr_" + std::to_string(i), &this->jobs[i].r_y_out_3_base_addr, 0);
    this->new_reg("y_out_2_base_addr_" + std::to_string(i), &this->jobs[i].r_y_out_2_base_addr, 0);
    this->new_reg("y_out_1_base_addr_" + std::to_string(i), &this->jobs[i].r_y_out_1_base_addr, 0);
    this->new_reg("y_out_0_base_addr_" + std::to_string(i), &this->jobs[i].r_y_out_0_base_addr, 0);
    this->new_reg("y_in_3_base_addr_" + std::to_string(i), &this->jobs[i].r_y_in_3_base_addr, 0);
    this->new_reg("y_in_2_base_addr_" + std::to_string(i), &this->jobs[i].r_y_in_2_base_addr, 0);
    this->new_reg("y_in_1_base_addr_" + std::to_string(i), &this->jobs[i].r_y_in_1_base_addr, 0);
    this->new_reg("y_in_0_base_addr_" + std::to_string(i), &this->jobs[i].r_y_in_0_base_addr, 0);
    this->new_reg("x_trans_size_" + std::to_string(i), &this->jobs[i].r_x_trans_size, 0);
    this->new_reg("x_line_stride_length_" + std::to_string(i), &this->jobs[i].r_x_line_stride_length, 0);
    this->new_reg("x_feat_stride_length_" + std::to_string(i), &this->jobs[i].r_x_feat_stride_length, 0);
    this->new_reg("x_in_base_addr_" + std::to_string(i), &this->jobs[i].r_x_in_base_addr, 0);
    this->new_reg("w_base_addr_" + std::to_string(i), &this->jobs[i].r_w_base_addr, 0);
    this->new_reg("job_config0_" + std::to_string(i), &this->jobs[i].r_job_config0, 0);
    this->new_reg("job_config1_" + std::to_string(i), &this->jobs[i].r_job_config1, 0);
  }

  for (int i=0; i<4; i++) {
    this->yinBase[i] = new Hwce_base(this);
    this->youtBase[i] = new Hwce_base(this);
  }

  this->xinBase = new Hwce_base(this);

  for (int i=0; i<2; i++)
  {
    jobs[i].id = i;
  }

  return 0;
}

void hwce::start()
{
}



void Hwce_base::init(uint32_t base, int lineLen, int lineStride, int featLen, int featStride, int nbFeatures, int featLoop) {
  this->baseStart = base;
  this->baseLoop = base;
  this->base = base;
  this->lineLen = lineLen;
  this->lineStride = lineStride;
  this->featLen = featLen;
  this->featStride = featStride;
  wordCount = 0;
  lineCount = 0;
  featCount = 0;
  featLoopCount = 0;
  this->nbFeatures = nbFeatures;
  this->featLoop = featLoop;
  pendingEof = false;
}

uint32_t Hwce_base::get() {
  return base;
}

Hwce_base::Hwce_base(hwce *top) : top(top) {}

void Hwce_base::update() {
  // Update address pointer
  wordCount++;

    // Check end of line for stride
  base += 4;
  if (wordCount == lineLen) {
    // End of line reached
    //gv_trace_dumpMsg(&top->trace, "Reached end of line (stride: 0x%x)\n", lineStride);
    wordCount = 0;
    lineCount++;
    base += (lineStride - lineLen) * 4;
    // Now check end of feature for stride
    if (lineCount == featLen) {
      // End of feature reached
      //gv_trace_dumpMsg(&top->trace, "Reached end of feature (stride: 0x%x)\n", featStride);
      lineCount = 0;
      base += (featStride - lineStride*featLen) * 4;
      featLoopCount++;
      featCount++;
      pendingEof = true;
      // TODO seems completly wrong
      if (featLoopCount < featLoop) {
        //gv_trace_dumpMsg(&top->trace, "Feature loop (nbFeatures: %d, nbFeaturesInLoop: %d)\n", featLoopCount, featLoop);
        base = baseLoop;        
      } else {
        featLoopCount = 0;
        baseLoop = base;
        if (featCount == nbFeatures) {
          //gv_trace_dumpMsg(&top->trace, "Finished all features\n");
          featCount = 0;
          base = baseStart;
        }
      }
    }
  }
}

void Hwce_base::startFeature() {
  pendingEof = false;
}

bool Hwce_base::reachedEof() {
  return pendingEof;
}


extern "C" void *vp_constructor(const char *config)
{
  return (void *)new hwce(config);
}
