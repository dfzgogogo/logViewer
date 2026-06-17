QT       += core gui widgets
CONFIG   += c++14
TEMPLATE  = app
TARGET    = logViewer

SOURCES += \
    src/main.cpp \
    src/logfilescanner.cpp \
    src/loglineparser.cpp \
    src/searchworker.cpp \
    src/logdataloader.cpp \
    src/paginationwidget.cpp \
    src/logviewerdialog.cpp

HEADERS += \
    src/oplogentry.h \
    src/logfilescanner.h \
    src/loglineparser.h \
    src/searchworker.h \
    src/logdataloader.h \
    src/paginationwidget.h \
    src/logviewerdialog.h
