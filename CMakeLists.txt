cmake_minimum_required(VERSION 3.8)
project(GitPRComp)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_CXX_STANDARD 17)

# add_subdirectory(src)
add_executable(GitPRComp src/main.cpp)

