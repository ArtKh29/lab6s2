cmake_minimum_required(VERSION 3.16)
project(lab7_variant7 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp mainwindow.h
    eventdialog.cpp eventdialog.h
    event.cpp event.h
)

qt_add_executable(lab7 ${PROJECT_SOURCES})
target_link_libraries(lab7 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
