list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraserviceplugin.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameracontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerametadatacontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfimagecapturecontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraservice.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerasession.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfstoragelocation.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfaudioinputselectorcontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerainfocontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediavideoprobecontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerarenderercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameradevicecontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerafocuscontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraexposurecontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerautility.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraviewfindersettingscontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfimageencodercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraflashcontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfvideoencodersettingscontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediacontainercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfaudioencodersettingscontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediarecordercontrol.h

   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfdisplaylink.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayermetadatacontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerservice.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayersession.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerserviceplugin.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideooutput.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowindowcontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidgetcontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidget.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideorenderercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideoframerenderer.h
)

if(WITH_MULTIMEDIA AND CMAKE_SYSTEM_NAME MATCHES "Darwin")

   # plugin 1
   add_library(CsMultimedia_avf_camera MODULE "")
   add_library(CopperSpice::CsMultimedia_avf_camera ALIAS CsMultimedia_avf_camera)

   set_target_properties(CsMultimedia_avf_camera PROPERTIES OUTPUT_NAME CsMultimedia_avf_camera${BUILD_ABI} PREFIX "")

   target_sources(CsMultimedia_avf_camera
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraserviceplugin.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameracontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerametadatacontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfimagecapturecontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraservice.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerasession.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfstoragelocation.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfaudioinputselectorcontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerainfocontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediavideoprobecontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameradevicecontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerarenderercontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerafocuscontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraexposurecontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcamerautility.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraviewfindersettingscontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfimageencodercontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfcameraflashcontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfvideoencodersettingscontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediacontainercontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfaudioencodersettingscontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/camera/avfmediarecordercontrol.mm
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
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfdisplaylink.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayercontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayermetadatacontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerservice.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerserviceplugin.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayersession.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideooutput.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowindowcontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidgetcontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidget.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideorenderercontrol.mm
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideoframerenderer.mm
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
