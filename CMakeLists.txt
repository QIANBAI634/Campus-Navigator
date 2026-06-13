cmake_minimum_required(VERSION 3.16)
project(CampusNavigator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# 尝试查找 Qt6，找不到则用 Qt5
find_package(Qt6 QUIET COMPONENTS Widgets Core)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets Core)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

add_executable(CampusNavigator
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/graph.h
    src/navigationdata.cpp
    src/navigationdata.h
    src/diarymanager.cpp
    src/diarymanager.h
    src/mapwidget.cpp
    src/mapwidget.h
    src/campusdatabase.cpp
    src/campusdatabase.h
    src/usermanager.cpp
    src/usermanager.h
    src/fooddatabase.h
    src/huffman.h
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(CampusNavigator PRIVATE Qt6::Widgets Qt6::Core)
else()
    target_link_libraries(CampusNavigator PRIVATE Qt5::Widgets Qt5::Core)
endif()

# Windows 下设置为 Windows 应用程序（不显示控制台），并定义数学常量
if(WIN32)
    set_target_properties(CampusNavigator PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    target_compile_definitions(CampusNavigator PRIVATE _USE_MATH_DEFINES)
endif()
