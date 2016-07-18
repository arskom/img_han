#-------------------------------------------------
#
# Project created by QtCreator 2016-06-20T09:06:17
#
#-------------------------------------------------

QT       += core gui widgets

lessThan(QT_MAJOR_VERSION, 5): error("requires Qt 5")

# Because of: QAbstractScrollArea::sizeAdjustPolicy
#     cf. http://doc.qt.io/qt-5/qabstractscrollarea.html#sizeAdjustPolicy-prop
lessThan(QT_MINOR_VERSION, 2): error("requires Qt 5.2")

TARGET = deneme2
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    wheeledgraphicsview.cpp

HEADERS  += mainwindow.h \
    wheeledgraphicsview.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++11
