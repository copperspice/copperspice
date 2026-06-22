add_library(CsMultimedia_m3u MODULE "")
add_library(CopperSpice::CsMultimedia_m3u ALIAS CsMultimedia_m3u)

set_target_properties(CsMultimedia_m3u PROPERTIES OUTPUT_NAME CsMultimedia_m3u${BUILD_ABI} PREFIX "")


list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/playlistformats/qm3u_plugin.h
)

target_sources(CsMultimedia_m3u
   PRIVATE
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/playlistformats/qm3u_plugin.cpp
)

target_link_libraries(CsMultimedia_m3u
   PRIVATE
   CsMultimedia
)

target_compile_definitions(CsMultimedia_m3u
   PRIVATE
   -DIN_TRUE
   -DQT_PLUGIN
)

install(TARGETS CsMultimedia_m3u DESTINATION ${CMAKE_INSTALL_LIBDIR})
