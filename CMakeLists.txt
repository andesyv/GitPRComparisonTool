cmake_minimum_required(VERSION 3.8)
project(GitPRComp)

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_CXX_STANDARD 20)

file(GLOB GitPRCompSRC "src/*.cpp")

# add_subdirectory(src)
add_executable(GitPRComp ${GitPRCompSRC})

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        else()
            execute_process(COMMAND ${GIT_EXECUTABLE} checkout v2.2.1
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/cxxopts
                            RESULT_VARIABLE GIT_SUBMOD_RESULT)
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/lib/cxxopts/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
else()
    add_subdirectory(${CMAKE_PREFIX_PATH}/cxxopts EXCLUDE_FROM_ALL)
    target_link_libraries(GitPRComp PUBLIC cxxopts)
    target_include_directories(GitPRComp PUBLIC ${CMAKE_PREFIX_PATH}/cxxopts/include)
endif()