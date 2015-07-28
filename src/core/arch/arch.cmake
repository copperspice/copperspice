set(CORE_PUBLIC_INCLUDES
    ${CORE_PUBLIC_INCLUDES}
    QAtomicInt
    QAtomicPointer
)

set(CORE_INCLUDES
    ${CORE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/qatomic_arch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/qatomic_generic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/qatomic_i386.h
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/qatomic_macosx.h
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/qatomic_windows.h
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/qatomic_x86_64.h
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/qatomicint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/qatomicpointer.h
)
