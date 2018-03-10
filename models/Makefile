ROOT_VP_BUILD_DIR ?= $(CURDIR)/build

PULP_PROPERTIES += udma/archi timer/version stdout/version soc_eu/version \
	fc_itc/version icache_ctrl/version xne/version fll/version event_unit/version \
	cluster_ctrl/version pulp_chip soc/cluster

properties := $(foreach prop,$(PULP_PROPERTIES), --property=$(prop))


VP_DIRS=memory pulp pulp/fll pulp/stdout pulp/chips/pulpissimo cpu/iss \
  interco utils pulp/itc pulp/hwpe/example pulp/timer pulp/soc_eu pulp/udma \
  pulp/hwpe/xne pulp/icache_ctrl pulp/cluster pulp/event_unit \
  pulp/chips/pulp


-include $(ROOT_VP_BUILD_DIR)/props.mk

-include $(PULP_SDK_HOME)/install/rules/vp_models.mk

props:
	plpinfo mkgen --makefile=$(ROOT_VP_BUILD_DIR)/props.mk $(properties)

build: vp_build

clean: vp_clean

.PHONY: clean build props
