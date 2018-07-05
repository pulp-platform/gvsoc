ifeq '$(pulp_chip)' 'oprecompkw'
inc_job_fifo=1
endif

ifeq '$(pulp_chip)' 'oprecompkw_sa'
inc_job_fifo=1
endif

ifeq '$(inc_job_fifo)' '1'
IMPLEMENTATIONS += pulp/chips/oprecompkw/job_fifo_impl
COMPONENTS += pulp/chips/oprecompkw/job_fifo
pulp/chips/oprecompkw/job_fifo_impl_SRCS = pulp/chips/oprecompkw/job_fifo_impl.cpp
endif
