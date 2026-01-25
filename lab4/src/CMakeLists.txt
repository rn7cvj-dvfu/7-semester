cmake_minimum_required(VERSION 3.10)
project(VirtualComPort)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
)

# Линковка библиотек для Unix
if(UNIX)
    target_link_libraries(sensor util)
endif()

# Установка
install(TARGETS sensor DESTINATION bin)
