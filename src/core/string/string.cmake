set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    QString8
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_string_iterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_encoding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/cs_char.h
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring8.h
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/string/qstring8.cpp
)
