set(EXTRA_AVFOUNDATION_LIBS
   CsCore${BUILD_ABI}
   CsGui${BUILD_ABI}
   CsNetwork${BUILD_ABI}
   CsMultimedia${BUILD_ABI}
)

set(EXTRA_AVFOUNDATION_CXXFLAGS)
set(EXTRA_AVFOUNDATION_LDFLAGS)

set(MULTIMEDIA_PUBLIC_INCLUDES
    ${MULTIMEDIA_PUBLIC_INCLUDES}
)

set(MULTIMEDIA_INCLUDES
    ${MULTIMEDIA_INCLUDES}
)

set(MULTIMEDIA_PRIVATE_INCLUDES
   ${MULTIMEDIA_PRIVATE_INCLUDES}
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

if(WITH_MULTIMEDIA AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   set(AVFOUNDATION_MEDIAPLAYER_SOURCES
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

   set(EXTRA_AVFOUNDATION_LDFLAGS
      ${EXTRA_AVFOUNDATION_LDFLAGS}
      -framework AVFoundation
      -framework CoreMedia
      -framework QuartzCore
      -framework AppKit
      -framework OpenGL
   )

   function_variable_fixup("${EXTRA_AVFOUNDATION_CXXFLAGS}" EXTRA_AVFOUNDATION_CXXFLAGS)
   function_variable_fixup("${EXTRA_AVFOUNDATION_LDFLAGS}"  EXTRA_AVFOUNDATION_LDFLAGS)

   add_library(CsMultimedia_avf_mediaplayer${BUILD_ABI} MODULE
      ${AVFOUNDATION_MEDIAPLAYER_SOURCES}
   )

   target_link_libraries(CsMultimedia_avf_mediaplayer${BUILD_ABI}
      ${EXTRA_AVFOUNDATION_LIBS}
   )

   target_include_directories(
      CsMultimedia_avf_mediaplayer${BUILD_ABI} PRIVATE
   )

   target_compile_definitions(CsMultimedia_avf_mediaplayer${BUILD_ABI} PRIVATE
      -DQT_PLUGIN
      -DQMEDIA_AVF_MEDIAPLAYER
   )

   set_target_properties(CsMultimedia_avf_mediaplayer${BUILD_ABI} PROPERTIES
       VERSION "0"
       SOVERSION "0.0.0"
       COMPILE_FLAGS ${EXTRA_AVFOUNDATION_CXXFLAGS}
       LINK_FLAGS ${EXTRA_AVFOUNDATION_LDFLAGS}
   )

   set_target_properties(CsMultimedia_avf_mediaplayer${BUILD_ABI} PROPERTIES PREFIX "")

   install(TARGETS CsMultimedia_avf_mediaplayer${BUILD_ABI}  DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
