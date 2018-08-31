ifeq '$(udma/version)' '2'
IMPLEMENTATIONS += pulp/udma/udma_v2_$(pulp_chip)_impl
COMPONENTS += pulp/udma/udma_v2
pulp/udma/udma_v2_$(pulp_chip)_impl_SRCS += pulp/udma/udma_v2_impl.cpp
pulp/udma/udma_v2_$(pulp_chip)_impl_SRCS += pulp/udma/udma_uart_v1.cpp
endif

ifeq '$(udma/spim/version)' '2'
pulp/udma/udma_v2_$(pulp_chip)_impl_SRCS += pulp/udma/udma_spim_v2.cpp
endif

ifeq '$(udma/cpi/version)' '1'
pulp/udma/udma_v2_$(pulp_chip)_impl_SRCS += pulp/udma/udma_cpi_v1.cpp
endif

ifeq '$(udma/hyper/version)' '1'
pulp/udma/udma_v2_$(pulp_chip)_impl_SRCS += pulp/udma/udma_hyper_v1.cpp
endif
