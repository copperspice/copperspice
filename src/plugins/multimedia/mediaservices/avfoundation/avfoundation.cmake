list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfdisplaylink.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayercontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayermetadatacontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerservice.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayersession.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerserviceplugin.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideooutput.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowindowcontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidgetcontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidget.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideorenderercontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideoframerenderer.h
)

if(WITH_MULTIMEDIA AND CMAKE_SYSTEM_NAME MATCHES "Darwin")

   add_library(CsMultimedia_avf_mediaplayer MODULE "")
   add_library(CopperSpice::CsMultimedia_avf_mediaplayer ALIAS CsMultimedia_avf_mediaplayer)

   set_target_properties(CsMultimedia_avf_mediaplayer PROPERTIES OUTPUT_NAME CsMultimedia_avf_mediaplayer${BUILD_ABI} PREFIX "")

   target_sources(CsMultimedia_avf_mediaplayer
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfdisplaylink.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayercontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayermetadatacontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerservice.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerserviceplugin.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayersession.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideooutput.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowindowcontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidgetcontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidget.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideorenderercontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideoframerenderer.mm
   )

   list(APPEND EXTRA_AVFOUNDATION_LDFLAGS
      -framework AVFoundation
      -framework CoreMedia
      -framework QuartzCore
      -framework AppKit
      -framework OpenGL
   )

   function_variable_fixup("${EXTRA_AVFOUNDATION_CXXFLAGS}" EXTRA_AVFOUNDATION_CXXFLAGS)
   function_variable_fixup("${EXTRA_AVFOUNDATION_LDFLAGS}"  EXTRA_AVFOUNDATION_LDFLAGS)

   target_link_libraries(CsMultimedia_avf_mediaplayer
      PRIVATE
      CsCore
      CsGui
      CsNetwork
      CsMultimedia
   )

   target_compile_definitions(CsMultimedia_avf_mediaplayer
      PRIVATE
      -DQT_PLUGIN
      -DQMEDIA_AVF_MEDIAPLAYER
   )

   set_target_properties(CsMultimedia_avf_mediaplayer
      PROPERTIES
      COMPILE_FLAGS ${EXTRA_AVFOUNDATION_CXXFLAGS}
      LINK_FLAGS ${EXTRA_AVFOUNDATION_LDFLAGS}
   )

   install(TARGETS CsMultimedia_avf_mediaplayer  DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
