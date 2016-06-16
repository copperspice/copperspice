
CORE_DIRS += src/core/concurrent

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


noinst_HEADERS+=$(patsubst %,src/core/concurrent/%,$(CORE_CONCURRENT_INCLUDES)) \
		$(patsubst %,src/core/concurrent/%,$(CORE_CONCURRENT_PRIVATE_INCLUDES))

lib_libCsCore@BUILD_ABI@_la_SOURCES += \
	src/core/concurrent/qfuture.cpp \
	src/core/concurrent/qfutureinterface.cpp \
	src/core/concurrent/qfuturesynchronizer.cpp \
	src/core/concurrent/qfuturewatcher.cpp \
	src/core/concurrent/qrunnable.cpp \
	src/core/concurrent/qtconcurrentexception.cpp \
	src/core/concurrent/qtconcurrentfilter.cpp \
	src/core/concurrent/qtconcurrentiteratekernel.cpp \
	src/core/concurrent/qtconcurrentmap.cpp \
	src/core/concurrent/qtconcurrentresultstore.cpp \
	src/core/concurrent/qtconcurrentthreadengine.cpp \
	src/core/concurrent/qthreadpool.cpp
