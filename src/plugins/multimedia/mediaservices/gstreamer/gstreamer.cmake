list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecodercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecoderservice.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecodersession.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecoderserviceplugin.h

   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_audioencoder.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_capturedestination.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_capturebufferformat.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_container.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_control.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_imagecapture.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_imageencoder.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_imageprocessing.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_infocontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_metadata.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_recorder.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_resourcepolicy.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_serviceplugin.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_service.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_session.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_videoencoder.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_viewfindersettings.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_viewfindersettings2.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_zoom.h

   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreameravailabilitycontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamermetadataprovider.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayerservice.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayerserviceplugin.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerstreamscontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayersession.h

   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstutils_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreameraudioinputselector_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreameraudioprobecontrol_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamerbufferprobe_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamerbushelper_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamermessage_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideowidget_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideoinputdevicecontrol_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideoprobecontrol_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideorendererinterface_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideooverlay_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideowindow_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideorenderer_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorendererplugin_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorenderersink_p.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideobuffer_p.h
)

if(WITH_MULTIMEDIA AND GStreamer_FOUND)

   set(GSTREAMER_TOOLS_SOURCES
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstutils.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreameraudioprobecontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamerbufferprobe.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamerbushelper.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamermessage.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideowidget.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideoinputdevicecontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideoprobecontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideorendererinterface.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideooverlay.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideowindow.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideorenderer.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideobuffer.cpp
   )

   # plugin 1
   add_library(CsMultimedia_gst_audiodecoder MODULE "")
   add_library(CopperSpice::CsMultimedia_gst_audiodecoder ALIAS CsMultimedia_gst_audiodecoder)

   set_target_properties(CsMultimedia_gst_audiodecoder PROPERTIES OUTPUT_NAME CsMultimedia_gst_audiodecoder${BUILD_ABI} PREFIX "")

   target_sources(CsMultimedia_gst_audiodecoder
      PRIVATE
      ${GSTREAMER_TOOLS_SOURCES}
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecodercontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecoderservice.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecodersession.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecoderserviceplugin.cpp
   )

   target_sources(CsMultimedia_gst_audiodecoder
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorendererplugin.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorenderersink.cpp
   )

   target_link_libraries(CsMultimedia_gst_audiodecoder
      CsCore
      CsGui
      CsNetwork
      CsMultimedia
      ${GSTREAMER_LIBRARIES}
      ${GSTREAMER_BASE_LIBRARIES}
      ${GSTREAMER_AUDIO_LIBRARIES}
      ${GSTREAMER_VIDEO_LIBRARIES}
      ${GSTREAMER_APP_LIBRARIES}
      ${GLIB2_LIBRARIES}
      ${GOBJECT2_LIBRARIES}
   )

   target_include_directories(
      CsMultimedia_gst_audiodecoder
      PRIVATE
      ${GSTREAMER_INCLUDE_DIR}
      ${GLIB2_INCLUDES}
   )

   target_compile_definitions(CsMultimedia_gst_audiodecoder
      PRIVATE
      -DQT_PLUGIN
   )

   # plugin 2
   add_library(CsMultimedia_gst_camerabin MODULE "")
   add_library(CopperSpice::CsMultimedia_gst_camerabin ALIAS CsMultimedia_gst_camerabin)

   set_target_properties(CsMultimedia_gst_camerabin PROPERTIES OUTPUT_NAME CsMultimedia_gst_camerabin${BUILD_ABI} PREFIX "")

   target_sources(CsMultimedia_gst_camerabin
      PRIVATE
      ${GSTREAMER_TOOLS_SOURCES}
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_serviceplugin.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_service.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_session.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_control.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_audioencoder.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_container.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_imagecapture.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_imageencoder.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_zoom.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_imageprocessing.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_metadata.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_recorder.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_videoencoder.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_resourcepolicy.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_capturedestination.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_viewfindersettings.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_viewfindersettings2.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_capturebufferformat.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/camera/camera_infocontrol.cpp

      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreameraudioinputselector.cpp
   )

   target_sources(CsMultimedia_gst_camerabin
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorenderersink.cpp
   )

   target_link_libraries(CsMultimedia_gst_camerabin
      CsCore
      CsGui
      CsNetwork
      CsMultimedia
      ${GSTREAMER_LIBRARIES}
      ${GSTREAMER_AUDIO_LIBRARIES}
      ${GSTREAMER_VIDEO_LIBRARIES}
      ${GLIB2_LIBRARIES}
      ${GOBJECT2_LIBRARIES}
   )

   target_include_directories(
      CsMultimedia_gst_camerabin
      PRIVATE
      ${GSTREAMER_INCLUDE_DIR}
      ${GLIB2_INCLUDES}
   )

   target_compile_definitions(CsMultimedia_gst_camerabin
      PRIVATE
      -DQT_PLUGIN
   )

   # plugin 3
   add_library(CsMultimedia_gst_mediaplayer MODULE "")
   add_library(CopperSpice::CsMultimedia_gst_mediaplayer ALIAS CsMultimedia_gst_mediaplayer)

   set_target_properties(CsMultimedia_gst_mediaplayer PROPERTIES OUTPUT_NAME CsMultimedia_gst_mediaplayer${BUILD_ABI} PREFIX "")

   target_sources(CsMultimedia_gst_mediaplayer
      PRIVATE
      ${GSTREAMER_TOOLS_SOURCES}
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayerservice.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerstreamscontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamermetadataprovider.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreameravailabilitycontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayerserviceplugin.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayercontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayersession.cpp
   )

   target_sources(CsMultimedia_gst_mediaplayer
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorendererplugin.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorenderersink.cpp
   )

   target_link_libraries(CsMultimedia_gst_mediaplayer
      CsCore
      CsGui
      CsNetwork
      CsMultimedia
      ${GSTREAMER_LIBRARIES}
      ${GSTREAMER_BASE_LIBRARIES}
      ${GSTREAMER_AUDIO_LIBRARIES}
      ${GSTREAMER_VIDEO_LIBRARIES}
      ${GSTREAMER_APP_LIBRARIES}
      ${GLIB2_LIBRARIES}
      ${GOBJECT2_LIBRARIES}
   )

   target_include_directories(
      CsMultimedia_gst_mediaplayer
      PRIVATE
      ${GSTREAMER_INCLUDE_DIR}
      ${GLIB2_INCLUDES}
   )

   target_compile_definitions(CsMultimedia_gst_mediaplayer
      PRIVATE
      -DIN_TRUE
      -DQT_PLUGIN
   )

   set_target_properties(CsMultimedia_gst_audiodecoder PROPERTIES PREFIX "")
   set_target_properties(CsMultimedia_gst_camerabin    PROPERTIES PREFIX "")
   set_target_properties(CsMultimedia_gst_mediaplayer  PROPERTIES PREFIX "")

   install(TARGETS CsMultimedia_gst_audiodecoder DESTINATION ${CMAKE_INSTALL_LIBDIR})
   install(TARGETS CsMultimedia_gst_camerabin    DESTINATION ${CMAKE_INSTALL_LIBDIR})
   install(TARGETS CsMultimedia_gst_mediaplayer  DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
