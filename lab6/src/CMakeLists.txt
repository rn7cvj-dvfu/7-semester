cmake_minimum_required(VERSION 3.16)
project(TemperatureMonitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Найти Qt6 или Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network Charts)

# Исходные файлы
set(PROJECT_SOURCES
    main.cpp
    mainwindow/mainwindow.cpp
    mainwindow/mainwindow.h
    apihandler/apihandler.cpp
    apihandler/apihandler.h
)

# Создание исполняемого файла
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(temperature_monitor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(temperature_monitor
        ${PROJECT_SOURCES}
    )
endif()

# Линковка библиотек Qt
target_link_libraries(temperature_monitor PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Charts
)

# Платформо-специфичные настройки
if(WIN32)
    set_property(TARGET temperature_monitor PROPERTY WIN32_EXECUTABLE TRUE)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_finalize_executable(temperature_monitor)
endif()

# Установка
install(TARGETS temperature_monitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
