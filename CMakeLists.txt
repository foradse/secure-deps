cmake_minimum_required(VERSION 3.16)
project(secure_deps LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(secure_deps_lib
  src/allowlist.cpp
  src/cmake_fetchcontent.cpp
  src/makefile_parser.cpp
)
target_include_directories(secure_deps_lib PUBLIC include)

add_executable(secure_deps src/main.cpp)
target_link_libraries(secure_deps PRIVATE secure_deps_lib)

enable_testing()
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(secure_deps_tests
  tests/test_allowlist.cpp
  tests/test_cmake_fetchcontent.cpp
  tests/test_makefile_parser.cpp
)
target_link_libraries(secure_deps_tests PRIVATE secure_deps_lib gtest_main)
include(GoogleTest)
gtest_discover_tests(secure_deps_tests)
