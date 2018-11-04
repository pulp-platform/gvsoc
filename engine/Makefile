BUILD_DIR ?= $(CURDIR)/build
ROOT_VP_BUILD_DIR ?= $(BUILD_DIR)
ENGINE_BUILD_DIR = $(BUILD_DIR)/engine

VP_DIRS = vp


-include $(PULP_SDK_HOME)/install/rules/vp_models.mk


CC = g++

CFLAGS +=  -MMD -MP -O2 -g -fpic -Isrc -std=c++11 -Werror -Wall -I$(PULP_SDK_WS_INSTALL)/include

LDFLAGS += -O2 -g -shared -Werror -Wall -L$(PULP_SDK_HOME)/install/ws/lib -Wl,--whole-archive -ljson -Wl,--no-whole-archive

ifdef VP_USE_SYSTEMC
CFLAGS += -D__VP_USE_SYSTEMC -I$(SYSTEMC_HOME)/include
LDFLAGS += -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc
endif

CFLAGS_DBG += -DVP_TRACE_ACTIVE=1

VP_SRCS = src/vp.cpp src/trace/trace.cpp src/clock/clock.cpp src/trace/vcd.cpp src/power/power.cpp
VP_OBJS = $(patsubst src/%.cpp,$(ENGINE_BUILD_DIR)/%.o,$(patsubst src/%.c,$(ENGINE_BUILD_DIR)/%.o,$(VP_SRCS)))
VP_DBG_OBJS = $(patsubst src/%.cpp,$(ENGINE_BUILD_DIR)/dbg/%.o,$(patsubst src/%.c,$(ENGINE_BUILD_DIR)/dbg/%.o,$(VP_SRCS)))

VP_HEADERS += $(shell find include -name *.hpp)
VP_HEADERS += $(shell find include -name *.h)

VP_HEADERS += $(shell find python -name *.py)



define declareInstallFile

$(PULP_SDK_WS_INSTALL)/$(1): $(1)
	install -D $(1) $$@

INSTALL_FILES += $(PULP_SDK_WS_INSTALL)/$(1)

endef

-include $(VP_OBJS:.o=.d)

$(ENGINE_BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ENGINE_BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(ENGINE_BUILD_DIR)/dbg/%.o: src/%.c
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS) $(CFLAGS_DBG) -o $@ -c $<

$(ENGINE_BUILD_DIR)/dbg/%.o: src/%.cpp
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS) $(CFLAGS_DBG) -o $@ -c $<

$(ENGINE_BUILD_DIR)/libpulpvp.so: $(VP_OBJS)
	@mkdir -p $(basename $@)
	$(CC) $^ -o $@ $(LDFLAGS)


$(ENGINE_BUILD_DIR)/libpulpvp-debug.so: $(VP_DBG_OBJS)
	@mkdir -p $(basename $@)
	$(CC) $^ -o $@ $(LDFLAGS)


$(foreach file, $(VP_HEADERS), $(eval $(call declareInstallFile,$(file))))



$(PULP_SDK_WS_INSTALL)/lib/libpulpvp.so: $(ENGINE_BUILD_DIR)/libpulpvp.so
	install -D $^ $@

$(PULP_SDK_WS_INSTALL)/lib/libpulpvp-debug.so: $(ENGINE_BUILD_DIR)/libpulpvp-debug.so
	install -D $^ $@


headers: $(INSTALL_FILES)

build: headers $(PULP_SDK_WS_INSTALL)/lib/libpulpvp.so $(PULP_SDK_WS_INSTALL)/lib/libpulpvp-debug.so vp_build

clean: vp_clean
	rm -rf $(ENGINE_BUILD_DIR)

.PHONY: build
