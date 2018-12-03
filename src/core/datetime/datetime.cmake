set(CORE_PUBLIC_INCLUDES
   ${CORE_PUBLIC_INCLUDES}
   QDate
   QDateTime
   QTime
   QTimeZone
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdatetime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone.h
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdatetime_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdatetimeparser_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone_data.hs
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdatetime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdatetimeparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone_p.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone_p_win.cpp
    )

elseif(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD)")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone_tzfile.cpp
    )

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CORE_SOURCES
        ${CORE_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qdatetime_mac.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/datetime/qtimezone_p_mac.mm
    )
endif()
