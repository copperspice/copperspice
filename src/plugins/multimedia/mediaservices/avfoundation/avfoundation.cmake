list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraserviceplugin.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameracontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerametadatacontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfimagecapturecontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraservice.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerasession.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfstoragelocation.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfaudioinputselectorcontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerainfocontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediavideoprobecontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerarenderercontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameradevicecontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerafocuscontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraexposurecontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerautility.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraviewfindersettingscontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfimageencodercontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraflashcontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfvideoencodersettingscontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediacontainercontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfaudioencodersettingscontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediarecordercontrol.h

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

   # plugin 1
   add_library(CsMultimedia_avf_camera MODULE "")
   add_library(CopperSpice::CsMultimedia_avf_camera ALIAS CsMultimedia_avf_camera)

   set_target_properties(CsMultimedia_avf_camera PROPERTIES OUTPUT_NAME CsMultimedia_avf_camera${BUILD_ABI} PREFIX "")

   target_sources(CsMultimedia_avf_camera
      PRIVATE
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraserviceplugin.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameracontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerametadatacontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfimagecapturecontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraservice.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerasession.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfstoragelocation.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfaudioinputselectorcontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerainfocontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediavideoprobecontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameradevicecontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerarenderercontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerafocuscontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraexposurecontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerautility.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraviewfindersettingscontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfimageencodercontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraflashcontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfvideoencodersettingscontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediacontainercontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfaudioencodersettingscontrol.mm
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediarecordercontrol.mm
   )

   target_link_libraries(CsMultimedia_avf_camera
      PRIVATE
      CsCore
      CsGui
      CsNetwork
      CsMultimedia
      "-framework AudioToolbox"
      "-framework AVFoundation"
      "-framework Foundation"
      "-framework CoreAudio"
      "-framework CoreMedia"
      "-framework QuartzCore"
   )

   target_compile_definitions(CsMultimedia_avf_camera
      PRIVATE
      -DQT_PLUGIN
      -DQMEDIA_AVF_CAMERA
   )

   # plugin 2
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

   target_link_libraries(CsMultimedia_avf_mediaplayer
      PRIVATE
      CsCore
      CsGui
      CsNetwork
      CsMultimedia
      "-framework AVFoundation"
      "-framework CoreMedia"
      "-framework QuartzCore"
      "-framework AppKit"
      "-framework OpenGL"
   )

   target_compile_definitions(CsMultimedia_avf_mediaplayer
      PRIVATE
      -DQT_PLUGIN
      -DQMEDIA_AVF_MEDIAPLAYER
   )

   install(TARGETS CsMultimedia_avf_camera       DESTINATION ${CMAKE_INSTALL_LIBDIR})
   install(TARGETS CsMultimedia_avf_mediaplayer  DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
