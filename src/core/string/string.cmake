set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    QChar32
    QString8
    QString16
    QStringParser
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_string_iterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_char.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qchar32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring8.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring16.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringparser.h
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qchar32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring8.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring16.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qstringparser.cpp
)
