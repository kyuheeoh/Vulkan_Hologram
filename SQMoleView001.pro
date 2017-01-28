TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lvulkan
LIBS += -lpthread
LIBS += -lxcb

SOURCES += main.cpp \
    Shell.cpp \
    moleculeviewer.cpp \
    ShellXcb.cpp \
    Simulation.cpp \
    Meshes.cpp

HEADERS += \
    vkapplication.h \
    Shell.h \
    Helpers.h \
    moleculeviewer.h \
    build_options.h \
    ShellXcb.h \
    Simulation.h \
    Meshes.h \
    Hologram.frag.h \
    Hologram.push_constant.vert.h \
    Hologram.vert.h \
    Meshes.teapot.h
