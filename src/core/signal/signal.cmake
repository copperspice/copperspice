set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}      
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}   
    ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_macro.h
    ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_signal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_slot.h
)

set(CORE_PRIVATE_INCLUDES
    ${CORE_PRIVATE_INCLUDES}   
)

set(CORE_SOURCES
    ${CORE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_signal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_slot.cpp
)
