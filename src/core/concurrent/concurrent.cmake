list(APPEND CORE_PUBLIC_INCLUDES
   QFuture
   QFutureInterfaceBase
   QMutableFutureIterator
   QFutureIterator
   QFutureInterface
   QFutureSynchronizer
   QFutureWatcher
   QFutureWatcherBase
   QRunnable
   QtConcurrentFilter
   QtConcurrentMap
   QtConcurrentRun
   QThreadPool
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfuture.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfutureinterface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qmutablefutureiterator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfutureiterator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfutureinterfacebase.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfuturesynchronizer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfuturewatcher.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfuturewatcherbase.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qrunnable.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentcompilertest.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentexception.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentfilter.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentfilterkernel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentfunctionwrappers.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentiteratekernel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentmap.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentmapkernel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentmedian.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentreducekernel.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentresultstore.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentrun.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentrunbase.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentstoredfunctioncall.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentthreadengine.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qthreadpool.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfutureinterface_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfuturewatcher_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qthreadpool_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfuture.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfutureinterface.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfuturesynchronizer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qfuturewatcher.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qrunnable.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentexception.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentfilter.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentiteratekernel.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentmap.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentresultstore.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qtconcurrentthreadengine.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/concurrent/qthreadpool.cpp
)
