list(APPEND CORE_PUBLIC_INCLUDES
)

list(APPEND CORE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_internal.h
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_macro.h
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_signal.h
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_slot.h
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/rcu_guarded.hpp
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/rcu_list.hpp
)

list(APPEND CORE_PRIVATE_INCLUDES
)

target_sources(CsCore
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_signal.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/signal/cs_slot.cpp
)
