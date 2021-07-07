set(TESTBENCH_SRCS "i2s_verif.cpp"
    "spim_verif.cpp"
    "testbench.cpp"
    )

vp_model(NAME testbench
    PREFIX "devices/testbench"
    SOURCES ${TESTBENCH_SRCS}
    )

find_library(SNDFILE_LIB sndfile)
if(SNDFILE_LIB)
    target_compile_options(testbench PRIVATE "-DUSE_SNDFILE")
    target_link_libraries(testbench PRIVATE sndfile)
endif()

