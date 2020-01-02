list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_internal.h
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_macro.h
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_signal.h
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_slot.h
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_signal.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_slot.cpp
)
