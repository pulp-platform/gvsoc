export VP_ROOT_DIR = $(CURDIR)

clean:
	make -C engine clean
	make -C models clean

build:
	install -D vp_models.mk $(PULP_SDK_INSTALL)/rules/vp_models.mk
	make -C engine build
	make -C models props
	make -C models build

checkout:
	git submodule update --init
