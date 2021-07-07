
set(SOUND_PREFIX "devices/sound")

vp_model(NAME i2s_microphone
         PREFIX ${SOUND_PREFIX}
         SOURCES "i2s_microphone.cpp")

vp_model(NAME i2s_speaker
         PREFIX ${SOUND_PREFIX}
         SOURCES "i2s_speaker.cpp")

vp_model(NAME i2s_clock
         PREFIX ${SOUND_PREFIX}
         SOURCES "i2s_clock.cpp")

find_library(SNDFILE_LIB sndfile)
if(SNDFILE_LIB)
    target_compile_options(i2s_speaker PRIVATE "-DUSE_SNDFILE")
    target_link_libraries(i2s_speaker PRIVATE sndfile)

    target_compile_options(i2s_microphone PRIVATE "-DUSE_SNDFILE")
    target_link_libraries(i2s_microphone PRIVATE sndfile)
endif()
