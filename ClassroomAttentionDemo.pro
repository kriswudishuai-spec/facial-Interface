QT += widgets

CONFIG += c++17 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = ClassroomAttentionDemo

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    AttentionAnalyzer.cpp \
    EnvSensor.cpp \
    DeviceController.cpp \
    ControllerLogic.cpp \
    EmotionAnalyzer.cpp

HEADERS += \
    MainWindow.h \
    AttentionAnalyzer.h \
    EnvSensor.h \
    DeviceController.h \
    ControllerLogic.h \
    EmotionAnalyzer.h

# 注意：qmake 方式使用 OpenCV 需要你手动配置 OpenCV 的 include/lib 路径。
# 推荐用 CMake + vcpkg（更省心）。

