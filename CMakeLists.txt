# Tell compiler to use C++20 features. The code doesn't actually use any of them.
# target_compile_features(poly PUBLIC cxx_std_20)

# version 3.11 or later of CMake or needed later for installing GoogleTest
# so let's require it now.
cmake_minimum_required(VERSION 3.11-3.18)

project(RingBufLib)

file(GLOB RingBufLib_SRC
     "*.c"
)

add_definitions(-D_REPLACE_CONST)

add_library(${PROJECT_NAME} ${RingBufLib_SRC})

enable_testing ()
add_subdirectory (test)

