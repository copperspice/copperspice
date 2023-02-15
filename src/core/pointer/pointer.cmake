list(APPEND CORE_PUBLIC_INCLUDES
   QExplicitlySharedDataPointer
   QPointer
   QScopedArrayPointer
   QScopedPointer
   QScopedPointerArrayDeleter
   QScopedPointerDeleter
   QScopedPointerPodDeleter
   QScopedValueRollback
   QSharedData
   QSharedDataPointer
   QSharedPointer
   QWeakPointer
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qexplicitlyshareddatapointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qpointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedarraypointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedpointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedpointerarraydeleter.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedpointerdeleter.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedpointerpoddeleter.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedvaluerollback.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qshareddata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qshareddatapointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qsharedpointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qsharedpointer_impl.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qweakpointer.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qpointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedpointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedvaluerollback.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qshareddata.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qsharedpointer.cpp
)

