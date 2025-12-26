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
