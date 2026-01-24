cmake_minimum_required(VERSION 3.10)
project(lab3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Определяем исходные файлы process_manager в зависимости от платформы
if(WIN32)
    set(PROCESS_MANAGER_SOURCES
        ../../lab2/src/process_manager/src/process_manager_win.cpp
    )
    set(SHARED_MEMORY_SOURCES
        shared_memory/src/shared_memory_win.cpp
    )
elseif(UNIX)
    set(PROCESS_MANAGER_SOURCES
        ../../lab2/src/process_manager/src/process_manager_posix.cpp
    )
    set(SHARED_MEMORY_SOURCES
        shared_memory/src/shared_memory_posix.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform: only Windows (WIN32) and UNIX are supported")
endif()

# Создаем библиотеку для управления процессами
add_library(process_manager STATIC
    ${PROCESS_MANAGER_SOURCES}
    ../../lab2/src/process_manager/include/process_manager.hpp
)

# Указываем путь к заголовочным файлам для библиотеки
target_include_directories(process_manager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lab2/src/process_manager/include
)

# Создаем библиотеку для разделяемой памяти
add_library(shared_memory STATIC
    ${SHARED_MEMORY_SOURCES}
    shared_memory/include/shared_memory.hpp
)

# Указываем путь к заголовочным файлам для shared_memory
target_include_directories(shared_memory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/shared_memory/include
)

# Для UNIX систем добавляем rt (real-time extensions)
if(UNIX)
    target_link_libraries(shared_memory PRIVATE rt pthread)
endif()

# Основная программа
add_executable(lab3 main.cpp)
target_link_libraries(lab3 PRIVATE process_manager shared_memory)

# Программа increment_counter (увеличивает счетчик на 10)
add_executable(increment_counter increment_counter.cpp)
target_link_libraries(increment_counter PRIVATE shared_memory)

# Программа multiply_counter (умножает на 2, ждет 2с, делит на 2)
add_executable(multiply_counter multiply_counter.cpp)
target_link_libraries(multiply_counter PRIVATE shared_memory)

# Для POSIX систем добавляем поддержку pthread
if(UNIX)
    target_link_libraries(lab3 PRIVATE pthread)
    target_link_libraries(increment_counter PRIVATE pthread)
    target_link_libraries(multiply_counter PRIVATE pthread)
endif()
