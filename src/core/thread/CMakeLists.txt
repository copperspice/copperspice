set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    QMutex
    QMutexData
    QMutexLocker
    QBasicAtomicInt
    QBasicAtomicPointer
    QReadLocker
    QReadWriteLock
    QSemaphore
    QThread
    QThreadStorage
    QThreadStorageData
    QWaitCondition
    QWriteLocker
    PARENT_SCOPE
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qatomic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qbasicatomic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qbasicatomicint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qbasicatomicpointer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qmutex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qmutexdata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qmutexlocker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qreadlocker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qreadwritelock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsemaphore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qthread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qthreadstorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qthreadstoragedata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qwaitcondition.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qwritelocker.h
    PARENT_SCOPE
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qmutex_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qmutexpool_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qorderedmutexlocker_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qreadwritelock_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qthread_p.h
    PARENT_SCOPE
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qatomic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qmutex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qmutexpool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qreadwritelock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qsemaphore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qthreadstorage.cpp
    PARENT_SCOPE
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qmutex_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qthread_win.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qwaitcondition_win.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qmutex_linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qthread_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qwaitcondition_unix.cpp
        PARENT_SCOPE
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/qmutex_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qthread_unix.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qwaitcondition_unix.cpp
        PARENT_SCOPE
    )
endif()
