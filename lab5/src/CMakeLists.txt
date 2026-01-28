cmake_minimum_required(VERSION 3.10)
project(TemperatureServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти SQLite3
find_package(SQLite3)

if(NOT SQLite3_FOUND)
    message(WARNING "SQLite3 not found, will try to use system library")
    if(WIN32)
        set(SQLITE3_LIBRARIES "sqlite3")
    else()
        set(SQLITE3_LIBRARIES "sqlite3")
    endif()
else()
    set(SQLITE3_LIBRARIES SQLite::SQLite3)
endif()

# Библиотека DateTime
add_library(date_time 
    date_time/src/date_time.cpp
    date_time/include/date_time.hpp
)

target_include_directories(date_time PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/date_time/include
)

# Библиотека Database
add_library(database
    database/src/database.cpp
    database/include/database.hpp
)

target_include_directories(database PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/database/include
    ${CMAKE_CURRENT_SOURCE_DIR}/date_time/include
)

target_link_libraries(database date_time ${SQLITE3_LIBRARIES})

# Библиотека HTTP Server
set(HTTP_SERVER_SOURCES
    http_server/include/http_server.hpp
)

if(WIN32)
    list(APPEND HTTP_SERVER_SOURCES http_server/src/http_server_win.cpp)
    set(HTTP_SERVER_LIBS ws2_32)
elseif(UNIX)
    list(APPEND HTTP_SERVER_SOURCES http_server/src/http_server_posix.cpp)
    set(HTTP_SERVER_LIBS pthread)
endif()

add_library(http_server ${HTTP_SERVER_SOURCES})

target_include_directories(http_server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/http_server/include
)

target_link_libraries(http_server ${HTTP_SERVER_LIBS})

# Исходные файлы виртуального COM порта
set(VCOM_SOURCES
    virtual_com_port/include/virtual_com_port.hpp
)

if(WIN32)
    list(APPEND VCOM_SOURCES virtual_com_port/src/virtual_com_port_win.cpp)
elseif(UNIX)
    list(APPEND VCOM_SOURCES virtual_com_port/src/virtual_com_port_posix.cpp)
endif()

# Программа сенсора (из lab4)
add_executable(sensor sensor.cpp ${VCOM_SOURCES})

target_include_directories(sensor PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/virtual_com_port/include
    ${CMAKE_CURRENT_SOURCE_DIR}/date_time/include
)

target_link_libraries(sensor date_time)

if(UNIX)
    target_link_libraries(sensor util)
endif()

# Главная программа сервера
add_executable(server server.cpp ${VCOM_SOURCES})

target_include_directories(server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/virtual_com_port/include
    ${CMAKE_CURRENT_SOURCE_DIR}/date_time/include
    ${CMAKE_CURRENT_SOURCE_DIR}/database/include
    ${CMAKE_CURRENT_SOURCE_DIR}/http_server/include
)

target_link_libraries(server date_time database http_server ${SQLITE3_LIBRARIES})

if(UNIX)
    target_link_libraries(server util)
endif()

# Копирование веб-файлов
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/web/ 
     DESTINATION ${CMAKE_BINARY_DIR}/web/)

# Установка
install(TARGETS sensor server DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/web/ DESTINATION share/temperature_server/web)
