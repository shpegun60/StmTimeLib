INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

include(gnu/gnumapfile.pri)
include(clang/clangmapfile.pri)

HEADERS += \
    $$PWD/Dwt.h \
    $$PWD/HTimer.h \
    $$PWD/Tick.h \
    $$PWD/irq/IRQGuard.h \
    $$PWD/itime_depency.h \
    $$PWD/itime_policy.h \
    $$PWD/interval.h \
    $$PWD/thirdparty/basic_types.h \
    $$PWD/thirdparty/macro.h \
    $$PWD/thirdparty/status.h \
    $$PWD/thirdparty/tools.h \
    $$PWD/virtual.h \
    \
    $$PWD/interval/OneShotIBase.h \
    $$PWD/interval/ITimeBase.h \
    $$PWD/interval/OneShotITimer.h \
    $$PWD/interval/StackITimer.h \
    \
    $$PWD/virtual/OneShotVBase.h \
    $$PWD/virtual/OneShotVTimer.h \
    $$PWD/virtual/StackVTimer.h \
    $$PWD/virtual/VTimeBase.h \
    $$PWD/virtual/VTimer.h \
	
	

SOURCES += \
    $$PWD/Dwt.cpp\
	$$PWD/HTimer.cpp\
	$$PWD/virtual/VTimer.cpp \
