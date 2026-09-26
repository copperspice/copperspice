list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/directshow_plugin.h

   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_global.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_service.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_control.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_session.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_viewfindersettingscontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_imageprocessingcontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsimage_capturecontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsvideo_renderer.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsvideo_devicecontrol.h

   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshoweventloop.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowpinenum.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowmediatype.h

   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/dsplayer_global.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowioreader.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowiosource.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowmediatypelist.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowplayercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowplayerservice.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowsamplescheduler.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowvideorenderercontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/mediasamplevideobuffer.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/videosurfacefilter.h

   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowaudioendpointcontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowmetadatacontrol.h
   ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/vmr9videowindowcontrol.h
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")

   add_library(CsMultimedia_DirectShow MODULE "")
   add_library(CopperSpice::CsMultimedia_DirectShow ALIAS CsMultimedia_DirectShow)

   set_target_properties(CsMultimedia_DirectShow PROPERTIES OUTPUT_NAME CsMultimedia_DirectShow${BUILD_ABI} PREFIX "")

   target_sources(CsMultimedia_DirectShow
      PRIVATE

      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/directshow_plugin.cpp

      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_service.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_control.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_session.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_viewfindersettingscontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_imageprocessingcontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsimage_capturecontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsvideo_renderer.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsvideo_devicecontrol.cpp

      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshoweventloop.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowmediatype.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowpinenum.cpp

      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowioreader.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowiosource.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowmediatypelist.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowplayercontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowplayerservice.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowsamplescheduler.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowvideorenderercontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/mediasamplevideobuffer.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/videosurfacefilter.cpp

      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowaudioendpointcontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowmetadatacontrol.cpp
      ${PROJECT_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/vmr9videowindowcontrol.cpp
    )

    target_link_libraries(CsMultimedia_DirectShow
       CsCore
       CsGui
       CsNetwork
       CsMultimedia
       strmiids
       dmoguids
       uuid
       ole32
       oleaut32
       msdmo
       gdi32
    )

    target_compile_definitions(CsMultimedia_DirectShow
       PRIVATE
       -DQT_PLUGIN
       -DQMEDIA_DIRECTSHOW_CAMERA
       -DQMEDIA_DIRECTSHOW_PLAYER
       -DNO_DSHOW_STRSAFE
    )

    install(TARGETS CsMultimedia_DirectShow DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
