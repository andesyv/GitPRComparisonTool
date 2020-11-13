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
            message(STATUS "Set submodule versions")
            # cxxopts
            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                execute_process(COMMAND ${GIT_EXECUTABLE} checkout v2.2.1
                                WORKING_DIRECTORY ${CMAKE_PREFIX_PATH}/cxxopts
                                RESULT_VARIABLE GIT_SUBMOD_RESULT)
            endif()
            
            # libgit2
            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                execute_process(COMMAND ${GIT_EXECUTABLE} checkout v1.1.1
                                WORKING_DIRECTORY ${CMAKE_PREFIX_PATH}/libgit2
                                RESULT_VARIABLE GIT_SUBMOD_RESULT)
            endif()
            # if(NOT GIT_SUBMOD_RESULT EQUAL "0" AND NOT ${Matplot++_FOUND})
            #     execute_process(COMMAND ${GIT_EXECUTABLE} checkout v1.1.1
            #         WORKING_DIRECTORY ${CMAKE_PREFIX_PATH}/matplot
            #         RESULT_VARIABLE GIT_SUBMOD_RESULT)
            # endif()
        endif()
    endif()
endif()

if(NOT GIT_SUBMOD_RESULT EQUAL "0")
    message(FATAL_ERROR "The submodules were not downloaded correctly! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
else()
    # cxxopts
    add_subdirectory(${CMAKE_PREFIX_PATH}/cxxopts EXCLUDE_FROM_ALL)
    target_link_libraries(GitPRComp PUBLIC cxxopts)
    target_include_directories(GitPRComp PUBLIC ${CMAKE_PREFIX_PATH}/cxxopts/include)

    # libgit2
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Shared Library (OFF for Static)" FORCE)
    add_subdirectory(${CMAKE_PREFIX_PATH}/libgit2 EXCLUDE_FROM_ALL)
    target_include_directories(GitPRComp PUBLIC ${CMAKE_PREFIX_PATH}/libgit2/include)
    target_link_libraries(GitPRComp PUBLIC git2)


    # Matplot++
    set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
    set(BUILD_TESTS OFF CACHE BOOL "Build tests")
    find_package(Matplot++ QUIET)
    if (NOT ${Matplot++_FOUND})
        message(STATUS "Matplot++ not found, manually linking using submodule instead.")
        add_subdirectory(${CMAKE_PREFIX_PATH}/matplot)
    endif()
    # target_include_directories(GitPRComp PUBLIC ${CMAKE_PREFIX_PATH}/matplot)
    target_link_libraries(GitPRComp PUBLIC matplot)

endif()