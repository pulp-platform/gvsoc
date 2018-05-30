ROOT_VP_BUILD_DIR ?= $(CURDIR)/build
BUILD_DIR = $(ROOT_VP_BUILD_DIR)/engine

VP_DIRS = vp


-include $(PULP_SDK_HOME)/install/rules/vp_models.mk


CC = g++

CFLAGS +=  -MMD -MP -O2 -g -fpic -Isrc -std=c++11 -Werror -Wall -I$(PULP_SDK_WS_INSTALL)/include

LDFLAGS += -O2 -g -shared -Werror -Wall -L$(PULP_SDK_HOME)/install/ws/lib -Wl,--whole-archive -ljson -Wl,--no-whole-archive

ifdef VP_USE_SYSTEMC
CFLAGS += -D__VP_USE_SYSTEMC -I$(SYSTEMC_HOME)/include
#LDFLAGS += -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc
endif

VP_SRCS = src/vp.cpp src/trace/trace.cpp src/clock/clock.cpp src/trace/vcd.cpp src/power/power.cpp
VP_OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(patsubst src/%.c,$(BUILD_DIR)/%.o,$(VP_SRCS)))

VP_HEADERS += $(shell find include -name *.hpp)
VP_HEADERS += $(shell find include -name *.h)

VP_HEADERS += $(shell find python -name *.py)



define declareInstallFile

$(PULP_SDK_WS_INSTALL)/$(1): $(1)
	install -D $(1) $$@

INSTALL_FILES += $(PULP_SDK_WS_INSTALL)/$(1)

endef

-include $(VP_OBJS:.o=.d)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/libpulpvp.so: $(VP_OBJS)
	@mkdir -p $(basename $@)
	$(CC) $^ -o $@ $(LDFLAGS)



$(foreach file, $(VP_HEADERS), $(eval $(call declareInstallFile,$(file))))



$(PULP_SDK_WS_INSTALL)/lib/libpulpvp.so: $(BUILD_DIR)/libpulpvp.so
	install -D $^ $@


headers: $(INSTALL_FILES)

build: headers $(PULP_SDK_WS_INSTALL)/lib/libpulpvp.so vp_build

clean: vp_clean
	rm -rf $(BUILD_DIR)

.PHONY: build
