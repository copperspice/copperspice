
CORE_DIRS += src/corelib/concurrent

CORE_PUBLIC_INCLUDES += \
	QFuture \
	QFutureInterfaceBase \
	QMutableFutureIterator \
	QFutureIterator \
	QFutureInterface \
	QFutureSynchronizer \
	QFutureWatcher \
	QFutureWatcherBase \
	QRunnable \
	QtConcurrentFilter \
	QtConcurrentMap \
	QtConcurrentRun \
	QThreadPool

CORE_CONCURRENT_INCLUDES = \
	qfuture.h \
	qfutureinterface.h \
	qmutablefutureiterator.h \
	qfutureiterator.h \
	qfutureinterfacebase.h \
	qfuturesynchronizer.h \
	qfuturewatcher.h \
	qfuturewatcherbase.h \
	qrunnable.h \
	qtconcurrentcompilertest.h \
	qtconcurrentexception.h \
	qtconcurrentfilter.h \
	qtconcurrentfilterkernel.h \
	qtconcurrentfunctionwrappers.h \
	qtconcurrentiteratekernel.h \
	qtconcurrentmap.h \
	qtconcurrentmapkernel.h \
	qtconcurrentmedian.h \
	qtconcurrentreducekernel.h \
	qtconcurrentresultstore.h \
	qtconcurrentrun.h \
	qtconcurrentrunbase.h \
	qtconcurrentstoredfunctioncall.h \
	qtconcurrentthreadengine.h \
	qthreadpool.h

CORE_CONCURRENT_PRIVATE_INCLUDES = \
	qfutureinterface_p.h \
	qfuturewatcher_p.h \
	qthreadpool_p.h	

CORE_INCLUDES += $(CORE_CONCURRENT_INCLUDES)
CORE_PRIVATE_INCLUDES += $(CORE_CONCURRENT_PRIVATE_INCLUDES)


noinst_HEADERS+=$(patsubst %,src/corelib/concurrent/%,$(CORE_CONCURRENT_INCLUDES)) \
		$(patsubst %,src/corelib/concurrent/%,$(CORE_CONCURRENT_PRIVATE_INCLUDES))

lib_libCsCore1_la_SOURCES += \
	src/corelib/concurrent/qfuture.cpp \
	src/corelib/concurrent/qfutureinterface.cpp \
	src/corelib/concurrent/qfuturesynchronizer.cpp \
	src/corelib/concurrent/qfuturewatcher.cpp \
	src/corelib/concurrent/qrunnable.cpp \
	src/corelib/concurrent/qtconcurrentexception.cpp \
	src/corelib/concurrent/qtconcurrentfilter.cpp \
	src/corelib/concurrent/qtconcurrentiteratekernel.cpp \
	src/corelib/concurrent/qtconcurrentmap.cpp \
	src/corelib/concurrent/qtconcurrentresultstore.cpp \
	src/corelib/concurrent/qtconcurrentthreadengine.cpp \
	src/corelib/concurrent/qthreadpool.cpp
