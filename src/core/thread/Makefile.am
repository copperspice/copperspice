
CORE_DIRS += src/core/thread

CORE_PUBLIC_INCLUDES += \
	QMutex \
	QMutexData \
	QMutexLocker \
	QAtomicInt \
	QAtomicPointer \
	QReadLocker \
	QReadWriteLock \
	QSemaphore \
	QThread \
	QThreadStorage \
	QThreadStorageData \
	QWaitCondition \
	QWriteLocker

CORE_THREAD_INCLUDES = \
	qatomic.h \
	qatomicint.h \
	qatomicpointer.h \
	qmutex.h \
	qmutexdata.h \
	qmutexlocker.h \
	qreadlocker.h \
	qreadwritelock.h \
	qsemaphore.h \
	qthread.h \
	qthreadstorage.h \
	qthreadstoragedata.h \
	qwaitcondition.h \
	qwritelocker.h

CORE_THREAD_PRIVATE_INCLUDES = \
	qmutex_p.h \
	qmutexpool_p.h \
	qorderedmutexlocker_p.h \
	qreadwritelock_p.h \
	qthread_p.h

CORE_INCLUDES += $(CORE_THREAD_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_THREAD_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/core/thread/%,$(CORE_THREAD_INCLUDES)) \
		$(patsubst %,src/core/thread/%,$(CORE_THREAD_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/thread/qmutex.cpp \
	src/core/thread/qmutexpool.cpp \
	src/core/thread/qreadwritelock.cpp \
	src/core/thread/qsemaphore.cpp \
	src/core/thread/qthread.cpp \
	src/core/thread/qthreadstorage.cpp

if OSTYPE_WIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/thread/qmutex_win.cpp \
	src/core/thread/qthread_win.cpp \
	src/core/thread/qwaitcondition_win.cpp
endif

if OSTYPE_LINUX
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/thread/qmutex_linux.cpp \
	src/core/thread/qthread_unix.cpp \
	src/core/thread/qwaitcondition_unix.cpp
endif

if OSTYPE_BSD
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
        src/core/thread/qmutex_unix.cpp \
        src/core/thread/qthread_unix.cpp \
        src/core/thread/qwaitcondition_unix.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/thread/qmutex_mac.cpp \
	src/core/thread/qthread_unix.cpp \
	src/core/thread/qwaitcondition_unix.cpp
endif
