cmake_minimum_required(VERSION 3.10)
project(gvsoc-engine LANGUAGES CXX C)

set(GVSOC_ENGINE_CXX_SRCS "src/trace/lxt2.cpp"
                          "src/trace/event.cpp"
                          "src/trace/trace.cpp"
                          "src/trace/raw/trace_dumper.cpp"
                          "src/trace/raw.cpp"
                          "src/trace/fst.cpp"
                          "src/trace/vcd.cpp"
                          "src/clock/clock.cpp"
                          "src/vp.cpp"
                          "src/proxy.cpp"
                          "src/power/power.cpp"
    )

set(GVSOC_ENGINE_C_SRCS "src/trace/fst/lz4.c"
                        "src/trace/fst/fastlz.c"
                        "src/trace/fst/fstapi.c"
                        "src/trace/lxt2_write.c")

set(GVSOC_ENGINE_INC_DIRS "include")

#"vp/clock_domain_impl.cpp"
#"vp/time_domain_impl.cpp"
#"vp/power_engine_impl.cpp"
#"vp/trace_domain_impl.cpp"

add_library(gvsoc SHARED ${GVSOC_ENGINE_CXX_SRCS} ${GVSOC_ENGINE_C_SRCS})
target_include_directories(gvsoc PUBLIC ${GVSOC_ENGINE_INC_DIRS})
target_link_libraries(gvsoc PUBLIC json-tools z pthread ${CMAKE_DL_LIBS})
install(TARGETS gvsoc
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

add_executable(gvsoc-engine "src/main.cpp")
target_link_libraries(gvsoc-engine PRIVATE gvsoc json-tools z pthread ${CMAKE_DL_LIBS})
install(TARGETS gvsoc-engine
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )
add_subdirectory(vp)
