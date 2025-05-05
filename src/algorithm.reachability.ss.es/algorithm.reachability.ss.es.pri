message("pri file being processed: $$PWD")

HEADERS += \
    $$PWD/estree-ml.h \
    $$PWD/simpleestree.h \
    $$PWD/esvertexdata.h \
    $$PWD/estree-queue.h \
    $$PWD/estree-bqueue.h \
    $$PWD/sesvertexdata.h \
    $$PWD/simpleestree_selectrandom.h 

SOURCES += \
    $$PWD/estree-ml.cpp \
    $$PWD/simpleestree.cpp \
    $$PWD/esvertexdata.cpp \
    $$PWD/estree-queue.cpp \
    $$PWD/estree-bqueue.cpp \
    $$PWD/sesvertexdata.cpp \
    $$PWD/simpleestree_selectrandom.cpp
