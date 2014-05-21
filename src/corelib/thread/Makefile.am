
CORE_DIRS += src/corelib/thread

CORE_PUBLIC_INCLUDES += \
	QMutex \
	QMutexData \
	QMutexLocker \
	QBasicAtomicInt \
	QBasicAtomicPointer \
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
	qbasicatomic.h \
	qbasicatomicint.h \
	qbasicatomicpointer.h \
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


noinst_HEADERS+=$(patsubst %,src/corelib/thread/%,$(CORE_THREAD_INCLUDES)) \
		$(patsubst %,src/corelib/thread/%,$(CORE_THREAD_PRIVATE_INCLUDES))

lib_libCsCore1_la_SOURCES += \
	src/corelib/thread/qatomic.cpp \
	src/corelib/thread/qmutex.cpp \
	src/corelib/thread/qmutexpool.cpp \
	src/corelib/thread/qreadwritelock.cpp \
	src/corelib/thread/qsemaphore.cpp \
	src/corelib/thread/qthread.cpp \
	src/corelib/thread/qthreadstorage.cpp

if OSTYPE_WIN
lib_libCsCore1_la_SOURCES += \
	src/corelib/thread/qmutex_win.cpp \
	src/corelib/thread/qthread_win.cpp \
	src/corelib/thread/qwaitcondition_win.cpp
endif

if OSTYPE_LINUX
lib_libCsCore1_la_SOURCES += \
	src/corelib/thread/qmutex_linux.cpp \
	src/corelib/thread/qthread_unix.cpp \
	src/corelib/thread/qwaitcondition_unix.cpp
endif

if OSTYPE_DARWIN
lib_libCsCore1_la_SOURCES += \
	src/corelib/thread/qmutex_mac.cpp \
	src/corelib/thread/qthread_unix.cpp \
	src/corelib/thread/qwaitcondition_unix.cpp
endif
