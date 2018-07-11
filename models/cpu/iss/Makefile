IMPLEMENTATIONS += cpu/iss/iss_wrapper

COMPONENTS += cpu/iss/iss

cpu/iss/iss_wrapper_SRCS = cpu/iss/src/iss_wrapper.cpp cpu/iss/src/iss.cpp cpu/iss/src/insn_cache.cpp cpu/iss/src/csr.cpp cpu/iss/src/decoder.cpp cpu/iss/src/trace.cpp $(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscv_decoder_gen.cpp cpu/iss/softfloat/softfloat.cpp cpu/iss/flexfloat/flexfloat.cpp
cpu/iss/iss_wrapper_CFLAGS += -I$(CURDIR)/cpu/iss/include -DRISCV=1 -DRISCY -I$(CURDIR)/cpu/iss/softfloat -I$(CURDIR)/cpu/iss/flexfloat

$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscv_decoder_gen.hpp: cpu/iss/isa_gen/isa_riscv_gen.py cpu/iss/isa_gen/isa_gen.py
	./cpu/iss/isa_gen/isa_riscv_gen.py --source-file=$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscv_decoder_gen.cpp --header-file=$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscv_decoder_gen.hpp

$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscv_decoder_gen.cpp: cpu/iss/isa_gen/isa_riscv_gen.py cpu/iss/isa_gen/isa_gen.py
	./cpu/iss/isa_gen/isa_riscv_gen.py --source-file=$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscv_decoder_gen.cpp --header-file=$(VP_BUILD_DIR)/cpu/iss/iss_wrapper/riscv_decoder_gen.hpp
