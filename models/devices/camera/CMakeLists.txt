vp_model(NAME himax
    PREFIX "devices/camera"
    SOURCES "himax.cpp")

find_library(MAGICK_LIB GraphicsMagick++)
if(MAGICK_LIB)
    target_compile_options(himax PRIVATE "-D__MAGICK__")
    target_link_libraries(himax PRIVATE GraphicsMagick++)
else()
endif()
