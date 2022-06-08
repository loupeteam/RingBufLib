# version 3.11 or later of CMake or needed later for installing GoogleTest
# so let's require it now.
cmake_minimum_required(VERSION 3.11-3.18)

# project(RingBufLib_test)

file(GLOB RingBufLib_SRC
"*.cpp"
"*.c"
)

# add_compile_options(-fpermissive -Wno-write-strings)
add_definitions(-D_BUR_FORMAT_BRELF)
add_definitions(-D_REPLACE_CONST)

#if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MYPROJECT_BUILD_TESTING))
    #enable_testing()
    #add_subdirectory(test)
Include(FetchContent)

FetchContent_Declare(
Catch2
GIT_REPOSITORY https://github.com/catchorg/Catch2.git
GIT_TAG        v3.0.0-preview3
)

# set(CMAKE_BUILD_TYPE Debug)

FetchContent_MakeAvailable(Catch2)
add_executable(RingBufLib_test ${RingBufLib_SRC})

target_link_libraries(RingBufLib_test PRIVATE Catch2::Catch2WithMain)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(RingBufLib_test OUTPUT_DIR ${PROJECT_SOURCE_DIR}/report)

target_include_directories(RingBufLib_test PUBLIC ${PROJECT_SOURCE_DIR}/../../includes/)
target_include_directories(RingBufLib_test PUBLIC ${PROJECT_SOURCE_DIR}/../../includes/loupe/Includes/)
target_include_directories(RingBufLib_test PUBLIC ${PROJECT_SOURCE_DIR}/../)
set_target_properties( RingBufLib_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/ )

#add_test(NAME testAll COMMAND test)
#endif()

target_link_libraries(RingBufLib_test PUBLIC RingBufLib)