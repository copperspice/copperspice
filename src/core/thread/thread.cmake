list(APPEND CORE_PUBLIC_INCLUDES
   QMutex
   QMutexData
   QMutexLocker
   QAtomicInt
   QAtomicPointer
   QReadLocker
   QReadWriteLock
   QSemaphore
   QThread
   QThreadStorage
   QThreadStorageData
   QWaitCondition
   QWriteLocker
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qatomic.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qatomicint.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qatomicpointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutex.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutexdata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutexlocker.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qreadlocker.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qreadwritelock.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qsemaphore.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthread.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthreadstorage.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthreadstoragedata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qwaitcondition.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qwritelocker.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutex_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutexpool_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qorderedmutexlocker_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qreadwritelock_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthread_p.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutex.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutexpool.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qreadwritelock.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qsemaphore.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthread.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthreadstorage.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutex_mac.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthread_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qwaitcondition_unix.cpp
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutex_linux.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthread_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qwaitcondition_unix.cpp
  )

elseif(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutex_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthread_unix.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qwaitcondition_unix.cpp
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_sources(CsCore
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qmutex_win.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qthread_win.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/thread/qwaitcondition_win.cpp
   )

endif()
