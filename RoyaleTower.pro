QT += widgets

CONFIG += c++17

SOURCES += \
    carditem.cpp \
    enemy.cpp \
    gamescene.cpp \
    gameview.cpp \
    levelselectui.cpp \
    main.cpp \
    mainwindow.cpp \
    startui.cpp \
    towerr.cpp

HEADERS += \
    bgm.h \
    carditem.h \
    enemy.h \
    gamescene.h \
    gameview.h \
    levelselectui.h \
    mainwindow.h \
    startui.h \
    towerr.h

FORMS += \
    levelselectui.ui \
    mainwindow.ui \
    startui.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

QMAKE_CXXFLAGS += -O1

QT+= multimedia
