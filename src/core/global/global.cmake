list(APPEND CORE_PUBLIC_INCLUDES
   Q_INT16
   Q_INT32
   Q_INT64
   Q_INT8
   Q_LLONG
   Q_LONG
   Q_UINT16
   Q_UINT32
   Q_UINT64
   Q_UINT8
   Q_ULLONG
   Q_ULONG
   QAssert
   QFlag
   QFlags
   QGlobal
   QLibraryInfo
   QLog
   QSysInfo
   QtCore
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_int16.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_int32.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_int64.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_int8.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_llong.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_long.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_uint16.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_uint32.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_uint64.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_uint8.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_ullong.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/q_ulong.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qassert.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qendian.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qexport.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qfeatures.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qflag.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qflags.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qglobal.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qglobal_debug.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qlibraryinfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qlog.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qnamespace.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qnumeric.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qplatformdefs.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qplatformposix.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qsysinfo.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qt_windows.h
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qtcore.h
)

list(APPEND CORE_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qnumeric_p.h
)

set_property(SOURCE
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qlibraryinfo.cpp
   APPEND PROPERTY COMPILE_DEFINITIONS
   BUILD_DATE="${BUILD_DATE}"
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qassert.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qexport.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qglobal.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qlibraryinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qlog.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qmalloc.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qnumeric.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/global/qnamespace.cpp
)

