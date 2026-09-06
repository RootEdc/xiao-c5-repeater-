cmake_minimum_required(VERSION 3.16)

# Wymuś traktowanie folderu main jako głównego komponentu
set(COMPONENTS main)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(xiao_c5_repeater)



