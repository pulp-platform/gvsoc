IMPLEMENTATIONS += memory/memory_impl
COMPONENTS += memory/memory
memory/memory_impl_SRCS = memory/memory_impl.cpp

IMPLEMENTATIONS += memory/ddr_impl
COMPONENTS += memory/ddr
ifdef VP_USE_SYSTEMC
memory/ddr_impl_SRCS = memory/ddr_sc_impl.cpp
else
memory/ddr_impl_SRCS = memory/ddr_impl.cpp
endif
