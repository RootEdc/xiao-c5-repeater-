cmake_minimum_required(VERSION 3.16)

# Jawna, bezwzględna ścieżka do komponentu main
list(APPEND EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/main")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(xiao_c5_repeater)




