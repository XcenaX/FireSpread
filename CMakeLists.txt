cmake_minimum_required(VERSION 3.28)
project(CMakeFire1 VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_subdirectory(src)

target_include_directories(CMakeFire1 PRIVATE ${CMAKE_SOURCE_DIR}/include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_package(RapidJSON CONFIG REQUIRED)
target_link_libraries(CMakeFire1 PRIVATE rapidjson)
