set(F_GVSOC_ISS_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

function(generate_isa)
    cmake_parse_arguments(
        GEN_ISA
        ""
        "NAME;THINGY;PREFIX"
        "SOURCES"
        ${ARGN}
        )
    message(STATUS "generate_isa name=${GEN_ISA_NAME} thingy=${GEN_ISA_THINGY}")
    add_custom_command(
        OUTPUT "${GEN_ISA_NAME}_decoder_gen.cpp" "${GEN_ISA_NAME}_decoder_gen.hpp"
        COMMAND ${F_GVSOC_ISS_DIR}/isa_gen/isa_generator.py
            --source-file="${GEN_ISA_NAME}_decoder_gen.cpp"
            --header-file="${GEN_ISA_NAME}_decoder_gen.hpp" ${GEN_ISA_THINGY}
        )

    set(GEN_ISA_FILES
        "${GEN_ISA_NAME}_decoder_gen.cpp"
        "${GEN_ISA_NAME}_decoder_gen.hpp"
        )
    set(ISS_FILES
        "${F_GVSOC_ISS_DIR}/src/csr.cpp"
        "${F_GVSOC_ISS_DIR}/src/decoder.cpp"
        "${F_GVSOC_ISS_DIR}/src/insn_cache.cpp"
        "${F_GVSOC_ISS_DIR}/src/iss.cpp"
        "${F_GVSOC_ISS_DIR}/src/resource.cpp"
        "${F_GVSOC_ISS_DIR}/src/trace.cpp"
        "${F_GVSOC_ISS_DIR}/vp/src/iss_wrapper.cpp"
        "${F_GVSOC_ISS_DIR}/flexfloat/flexfloat.c"
        )

    vp_model(NAME ${GEN_ISA_NAME}
        PREFIX "${GEN_ISA_PREFIX}"
        SOURCES ${GEN_ISA_SOURCES} ${GEN_ISA_FILES} ${ISS_FILES}
        )
    target_include_directories(${GEN_ISA_NAME} PRIVATE
        "${F_GVSOC_ISS_DIR}/include"
        "${F_GVSOC_ISS_DIR}/vp/include"
        "${F_GVSOC_ISS_DIR}/sa/include"
        "${F_GVSOC_ISS_DIR}/sa/ext"
        "${F_GVSOC_ISS_DIR}/flexfloat"
        "${F_GVSOC_ISS_DIR}/sa/ext/bfd"
        )
    target_compile_definitions(${GEN_ISA_NAME} PRIVATE
        -DRISCV=1
        -DRISCY
        )
endfunction()
