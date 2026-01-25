cmake_minimum_required(VERSION 3.10)
project(VirtualComPort)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Библиотека DateTime
add_library(date_time 
    date_time/src/date_time.cpp
    date_time/include/date_time.hpp
)

target_include_directories(date_time PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/date_time/include
)

# Исходные файлы виртуального COM порта
set(VCOM_SOURCES
    virtual_com_port/include/virtual_com_port.hpp
)

# Добавляем платформо-зависимые файлы
if(WIN32)
    list(APPEND VCOM_SOURCES virtual_com_port/src/virtual_com_port_win.cpp)
elseif(UNIX)
    list(APPEND VCOM_SOURCES virtual_com_port/src/virtual_com_port_posix.cpp)
endif()

# Программа сенсора
add_executable(sensor sensor.cpp ${VCOM_SOURCES})

# Подключаем заголовочные файлы
target_include_directories(sensor PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/virtual_com_port/include
    ${CMAKE_CURRENT_SOURCE_DIR}/date_time/include
)

# Линковка библиотек
target_link_libraries(sensor date_time)

if(UNIX)
    target_link_libraries(sensor util)
endif()

# Программа логгера
add_executable(logger logger.cpp ${VCOM_SOURCES})

# Подключаем заголовочные файлы
target_include_directories(logger PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/virtual_com_port/include
    ${CMAKE_CURRENT_SOURCE_DIR}/date_time/include
)

# Линковка библиотек
target_link_libraries(logger date_time)

if(UNIX)
    target_link_libraries(logger util)
endif()

# Установка
install(TARGETS sensor logger DESTINATION bin)
