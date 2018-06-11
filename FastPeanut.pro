QT += core gui widgets
CONFIG += c++11

INCLUDEPATH += src/core

SOURCES += \
    src/main.cpp \
    src/gui/mainwindow.cpp \
    src/core/peanut.cpp \
    src/core/peanutcpu.cpp \
    src/core/peanutmemory.cpp \
    src/core/assembler.cpp \
    src/core/tokenizer.cpp \
    src/gui/terminalsimulator.cpp \
    src/gui/terminaledit.cpp \
    src/gui/terminaldataio.cpp \
    src/gui/displaysimulator.cpp \
    src/gui/textedittab.cpp \
    src/gui/textedit.cpp \
    src/gui/texthighlighter.cpp

HEADERS += \
    src/gui/mainwindow.h \
    src/core/peanut.h \
    src/core/peanutcpu.h \
    src/core/peanutmemory.h \
    src/core/peanutglobal.h \
    src/core/assembler.h \
    src/core/tokenizer.h \
    src/core/peanutdataio.h \
    src/gui/terminalsimulator.h \
    src/gui/terminaledit.h \
    src/gui/terminaldataio.h \
    src/core/peanutdisplay.h \
    src/gui/displaysimulator.h \
    src/gui/textedittab.h \
    src/gui/textedit.h \
    src/gui/texthighlighter.h
