CORE_DIRS += src/core/datetime

CORE_PUBLIC_INCLUDES += \
   QDate \
   QDateTime \
   QTime \
   QTimeZone

CORE_DATETIME_INCLUDES = \
   qdate.h \
   qdatetime.h \
   qtime.h \
   qtimezone.h

CORE_DATETIME_PRIVATE_INCLUDES = \
   qdatetime_p.h \
   qdatetimeparser_p.h \
   qtimezone_p.h \
   qtimezone_data.h

CORE_PUBLIC_INCLUDES += $(CORE_DATETIME_PUBLIC_INCLUDES)
CORE_INCLUDES += $(CORE_DATETIME_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_DATETIME_PRIVATE_INCLUDES)

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/datetime/qdatetime.cpp \
   src/core/datetime/qdatetimeparser.cpp \
   src/core/datetime/qtimezone.cpp \
   src/core/datetime/qtimezone_p.cpp

if OSTYPE_WIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/datetime/qtimezone_p_win.cpp
endif

if OSTYPE_LINUX
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/datetime/qtimezone_tzfile.cpp
endif

if OSTYPE_BSD
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/datetime/qtimezone_tzfile.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
   src/core/datetime/qdatetime_mac.mm \
   src/core/datetime/qtimezone_p_mac.mm
endif
