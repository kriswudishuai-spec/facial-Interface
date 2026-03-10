cmake_minimum_required(VERSION 3.16)

project(ClassroomAttentionDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Qt ---
if(NOT DEFINED QT_VERSION_MAJOR)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
else()
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
endif()

# --- OpenCV（指定你本机的 OpenCVConfig.cmake 路径） ---
if(NOT OpenCV_DIR)
    set(OpenCV_DIR "C:/Users/MK/Documents/opencv/build/x64/vc16/lib")
endif()
find_package(OpenCV REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    AttentionAnalyzer.cpp
    AttentionAnalyzer.h
    EnvSensor.cpp
    EnvSensor.h
    DeviceController.cpp
    DeviceController.h
    ControllerLogic.cpp
    ControllerLogic.h
    EmotionAnalyzer.cpp
    EmotionAnalyzer.h
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${OpenCV_LIBS}
)


