list(APPEND MULTIMEDIA_PUBLIC_INCLUDES
)

list(APPEND MULTIMEDIA_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/radio/qradiodata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/radio/qradiotuner.h
)

list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
)

target_sources(CsMultimedia
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/radio/qradiodata.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/radio/qradiotuner.cpp
)
