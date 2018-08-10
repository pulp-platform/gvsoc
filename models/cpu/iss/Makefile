COMPONENTS += cpu/iss/iss

COMMON_SRCS = cpu/iss/src/iss_wrapper.cpp cpu/iss/src/iss.cpp cpu/iss/src/insn_cache.cpp cpu/iss/src/csr.cpp cpu/iss/src/decoder.cpp cpu/iss/src/trace.cpp cpu/iss/softfloat/softfloat.cpp cpu/iss/flexfloat/flexfloat.c
COMMON_CFLAGS = -I$(CURDIR)/cpu/iss/include -DRISCV=1 -DRISCY -I$(CURDIR)/cpu/iss/softfloat -I$(CURDIR)/cpu/iss/flexfloat


define declare_iss_isa_build

$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/$(1)_decoder_gen.hpp: cpu/iss/isa_gen/isa_riscv_gen.py cpu/iss/isa_gen/isa_gen.py
	./cpu/iss/isa_gen/isa_riscv_gen.py --source-file=$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/$(1)_decoder_gen.cpp --header-file=$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/$(1)_decoder_gen.hpp $(2)

$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/$(1)_decoder_gen.cpp: cpu/iss/isa_gen/isa_riscv_gen.py cpu/iss/isa_gen/isa_gen.py
	./cpu/iss/isa_gen/isa_riscv_gen.py --source-file=$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/$(1)_decoder_gen.cpp --header-file=$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/$(1)_decoder_gen.hpp $(2)

endef

define declare_iss_build

IMPLEMENTATIONS += cpu/iss/iss_$(1)

cpu/iss/iss_$(1)_SRCS += $(COMMON_SRCS)
cpu/iss/iss_$(1)_CFLAGS += $(COMMON_CFLAGS)

endef

$(eval $(call declare_iss_isa_build,riscy))

cpu/iss/iss_riscy_SRCS += $(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscy_decoder_gen.cpp
$(eval $(call declare_iss_build,riscy))

cpu/iss/iss_riscy_single_regfile_CFLAGS += -DISS_SINGLE_REGFILE
cpu/iss/iss_riscy_single_regfile_SRCS += $(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscy_decoder_gen.cpp
$(eval $(call declare_iss_build,riscy_single_regfile))

