ROOT_VP_BUILD_DIR ?= $(CURDIR)/build

VP_DIRS=memory pulp pulp/fll pulp/stdout pulp/chips/pulpissimo cpu/iss \
  interco utils pulp/itc pulp/hwpe/example pulp/timer pulp/soc_eu pulp/udma \
  pulp/hwpe/xne pulp/icache_ctrl pulp/cluster pulp/event_unit


-include $(PULP_SDK_HOME)/install/rules/vp_models.mk


build: vp_build

clean: vp_clean

.PHONY: clean build
