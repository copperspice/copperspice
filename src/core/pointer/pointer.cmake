list(APPEND CORE_PUBLIC_INCLUDES
   QExplicitlySharedDataPointer
   QPointer
   QScopedArrayPointer
   QScopedPointer
   QScopedValueRollback
   QSharedData
   QSharedDataPointer
   QSharedPointer
   QWeakPointer
)

if (CsPointer_FOUND)
   # use system headers

else()
   # use annex headers
   target_include_directories(CsCore
      PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/annex/cs_pointer>
   )

   list(APPEND CORE_INCLUDES
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_pointer/cs_enable_shared.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_pointer/cs_shared_array_pointer.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_pointer/cs_shared_pointer.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_pointer/cs_unique_pointer.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_pointer/cs_unique_array_pointer.h
      ${CMAKE_CURRENT_SOURCE_DIR}/../annex/cs_pointer/cs_weak_pointer.h
   )
endif()

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qexplicitlyshareddatapointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qpointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedpointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedarraypointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedvaluerollback.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qshareddata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qshareddatapointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qsharedpointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qsharedpointer_impl.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/quniquepointer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qweakpointer.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qpointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qscopedvaluerollback.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qshareddata.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/qsharedpointer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/pointer/quniquepointer.cpp
)

