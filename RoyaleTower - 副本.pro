QT += widgets

CONFIG += c++17

SOURCES += \
    carditem.cpp \
    enemy.cpp \
    gamescene.cpp \
    gameview.cpp \
    main.cpp \
    mainwindow.cpp \
    towerr.cpp

HEADERS += \
    carditem.h \
    enemy.h \
    gamescene.h \
    gameview.h \
    mainwindow.h \
    towerr.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target