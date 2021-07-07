cmake_minimum_required(VERSION 3.10)
project(gvsoc_launcher LANGUAGES CXX)

# Is it user somewhere ?

add_library(gvsoc_launcher SHARED "src/launcher.cpp")
target_include_directories(gvsoc_launcher PUBLIC "include/")
target_link_libraries(gvsoc_launcher PRIVATE pthread)
