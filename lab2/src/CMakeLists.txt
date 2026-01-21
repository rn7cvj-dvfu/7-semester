cmake_minimum_required(VERSION 3.10)
project(lab2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Определяем исходные файлы в зависимости от платформы
if(WIN32)
    set(PROCESS_MANAGER_SOURCES
        process_manager/src/process_manager_win.cpp
    )
else()
    set(PROCESS_MANAGER_SOURCES
        process_manager/src/process_manager_posix.cpp
    )
endif()

# Создаем библиотеку для управления процессами
add_library(process_manager STATIC
    ${PROCESS_MANAGER_SOURCES}
    process_manager/include/process_manager.hpp
)

# Указываем путь к заголовочным файлам для библиотеки
target_include_directories(process_manager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/process_manager/include
)

# Основная программа, использующая библиотеку
add_executable(lab2 main.cpp)

# Линкуем библиотеку к исполняемому файлу
target_link_libraries(lab2 PRIVATE process_manager)

# Для POSIX систем добавляем поддержку pthread (если требуется)
if(UNIX)
    target_link_libraries(lab2 PRIVATE pthread)
endif()
