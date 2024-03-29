cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(basic)

set(CMAKE_DEBUG_POSTFIX d)

set(CXX_FLAGS
    -g
    # -DVALGRIND
    -DCHECK_PTHREAD_RETURN_VALUE
    -D_FILE_OFFSET_BITS=64
    -march=native
    # -MMD
    -rdynamic
    )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

# 设置生成文件输出路径
set(LIBRARY_OUTPUT_PATH, ./output/debug/lib)
add_library(basic STATIC "")

target_include_directories(basic PUBLIC ./inc/)
target_include_directories(basic PRIVATE ./extern_inc/)

target_link_directories(basic PRIVATE ./lib/debug/)

target_link_libraries(basic PRIVATE gtest)
target_link_libraries(basic PRIVATE pthread)
include(src/CMakeLists.txt)

