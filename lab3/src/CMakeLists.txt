cmake_minimum_required(VERSION 3.10)
project(lab3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(PROCESS_MANAGER_SOURCES
        ../../lab2/src/process_manager/src/process_manager_win.cpp
    )
    set(SHARED_MEMORY_SOURCES
        shared_memory/src/shared_memory_win.cpp
    )
    set(THREADS_SOURCES
        threads/src/threads_win.cpp
    )
    set(TIME_SOURCES
        time/src/time_win.cpp
    )
elseif(UNIX)
    set(PROCESS_MANAGER_SOURCES
        ../../lab2/src/process_manager/src/process_manager_posix.cpp
    )
    set(SHARED_MEMORY_SOURCES
        shared_memory/src/shared_memory_posix.cpp
    )
    set(THREADS_SOURCES
        threads/src/threads_posix.cpp
    )
    set(TIME_SOURCES
        time/src/time_posix.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform: only Windows (WIN32) and UNIX are supported")
endif()

add_library(process_manager STATIC
    ${PROCESS_MANAGER_SOURCES}
    ../../lab2/src/process_manager/include/process_manager.hpp
)

target_include_directories(process_manager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lab2/src/process_manager/include
)

add_library(shared_memory STATIC
    ${SHARED_MEMORY_SOURCES}
    shared_memory/include/shared_memory.hpp
)

target_include_directories(shared_memory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shared_memory/include
)

if(UNIX)
    target_link_libraries(shared_memory PRIVATE rt pthread)
endif()
и
add_library(threads STATIC
    ${THREADS_SOURCES}
    threads/include/threads.hpp
)

target_include_directories(threads PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/threads/include
)

if(UNIX)
    target_link_libraries(threads PRIVATE pthread)
endif()

add_library(time STATIC
    ${TIME_SOURCES}
    time/include/time.hpp
)

target_include_directories(time PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/time/include
)

add_executable(lab3 main.cpp)
target_link_libraries(lab3 PRIVATE process_manager shared_memory threads time)

add_executable(increment_counter increment_counter.cpp)
target_link_libraries(increment_counter PRIVATE  process_manager shared_memory threads time)

add_executable(multiply_counter multiply_counter.cpp)
target_link_libraries(multiply_counter PRIVATE  process_manager shared_memory threads time)

if(UNIX)
    target_link_libraries(lab3 PRIVATE pthread)
    target_link_libraries(increment_counter PRIVATE pthread)
    target_link_libraries(multiply_counter PRIVATE pthread)
endif()
