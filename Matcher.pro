#-------------------------------------------------
#
# Project created by QtCreator 2018-04-15T00:05:03
#
#-------------------------------------------------

QT       -= gui

TARGET = Matcher
TEMPLATE = lib

DEFINES += MATCHER_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        matcher.cpp \
    bozorth3m.cpp \
    matcher_isoconverter.cpp

HEADERS += \
        matcher.h \
        matcher_global.h \ 
    matcher_config.h \
    bozorth3m.h \
    matcher_isoconverter.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

#SupremaSensor
unix:!macx: LIBS += -L$$PWD/../../../../../opt/suprema/x64/ -lUFMatcher
INCLUDEPATH += $$PWD/../../../../../opt/suprema/include

#MCC
unix:!macx: LIBS += -L$$PWD/../MCC/ -lMCC
INCLUDEPATH += $$PWD/../MCC
DEPENDPATH += $$PWD/../MCC
