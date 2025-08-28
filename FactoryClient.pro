QT       += core gui multimedia multimediawidgets network sql charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ChatWidget.cpp \
    DeviceDataWidget.cpp \
    LoginDialog.cpp \
    MainWindow.cpp \
    SocketClient.cpp \
    TicketManager.cpp \
    VideoWidget.cpp \
    main.cpp

HEADERS += \
    ChatWidget.h \
    DeviceDataWidget.h \
    LoginDialog.h \
    MainWindow.h \
    SocketClient.h \
    TicketManager.h \
    VideoWidget.h

FORMS += \
    ChatWidget.ui \
    DeviceDataWidget.ui \
    LoginDialog.ui \
    MainWindow.ui \
    TicketManager.ui \
    VideoWidget.ui

TRANSLATIONS += \
    FactoryClient_zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    Common \
    database

## 配置选项
#win32: {
#    LIBS += -lws2_32
#}

#unix: {
#    LIBS += -lpthread
#}

## 发布模式
#RELEASE += {
#    QMAKE_CXXFLAGS += -O2
#}

## 调试模式
#DEBUG += {
#    QMAKE_CXXFLAGS += -g
#}

## 安装路径
#target.path = $$[QT_INSTALL_EXAMPLES]/industrial/FactoryClient
#INSTALLS += target
