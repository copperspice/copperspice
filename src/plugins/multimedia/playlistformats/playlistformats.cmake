set(EXTRA_MULTIMEDIA_PLAYLISTFORMATS_LIBS CsMultimedia${BUILD_ABI})

set(MULTIMEDIA_PRIVATE_INCLUDES
   ${MULTIMEDIA_PRIVATE_INCLUDES}
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/playlistformats/qm3u_plugin.h
)

set(MULTIMEDIA_PLAYLISTFORMATS_SOURCES
  ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/playlistformats/qm3u_plugin.cpp
)

add_library(CsMultimedia_m3u${BUILD_ABI} MODULE ${MULTIMEDIA_PLAYLISTFORMATS_SOURCES})
target_link_libraries(CsMultimedia_m3u${BUILD_ABI} PRIVATE ${EXTRA_MULTIMEDIA_PLAYLISTFORMATS_LIBS})

target_compile_definitions(CsMultimedia_m3u${BUILD_ABI} PRIVATE -DIN_TRUE -DQT_PLUGIN)

set_target_properties(CsMultimedia_m3u${BUILD_ABI} PROPERTIES PREFIX "")

install(TARGETS CsMultimedia_m3u${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
