message("pri file being processed: $$PWD")

HEADERS += \
    $$PWD/estree-ml.h \
    $$PWD/simpleestree.h \
    $$PWD/esvertexdata.h \
    $$PWD/estree-queue.h \
    $$PWD/estree-bqueue.h \
    $$PWD/sesvertexdata.h \
    $$PWD/simpleestree_selectrandom.h \
    $$PWD/simpleestree_multipletreearcs.h \
    $$PWD/sesvertexdatamultipleparents.h \
    $$PWD/reservoirsampler.h \
    $$PWD/simpleestree_reservoirsampling.h \
    $$PWD/simpleestree_timestamps.h \
    $$PWD/sesvertexdata_timestamps.h \
    $$PWD/simpleestree_timestamps_fpm.h \
    $$PWD/estree-ml_timestamps_fpm.h

SOURCES += \
    $$PWD/estree-ml.cpp \
    $$PWD/simpleestree.cpp \
    $$PWD/esvertexdata.cpp \
    $$PWD/estree-queue.cpp \
    $$PWD/estree-bqueue.cpp \
    $$PWD/sesvertexdata.cpp \
    $$PWD/simpleestree_selectrandom.cpp \
    $$PWD/simpleestree_multipletreearcs.cpp \
    $$PWD/sesvertexdatamultipleparents.cpp \
    $$PWD/reservoirsampler.cpp \
    $$PWD/simpleestree_reservoirsampling.cpp \
    $$PWD/simpleestree_timestamps.cpp \
    $$PWD/sesvertexdata_timestamps.cpp \
    $$PWD/simpleestree_timestamps_fpm.cpp \
    $$PWD/estree-ml_timestamps_fpm.cpp
