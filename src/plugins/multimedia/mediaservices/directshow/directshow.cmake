list(APPEND MULTIMEDIA_PRIVATE_INCLUDES
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/directshow_plugin.h

   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_global.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_service.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_control.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_session.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_viewfindersettingscontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_imageprocessingcontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsimage_capturecontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsvideo_renderer.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsvideo_devicecontrol.h

   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshoweventloop.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowpinenum.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowmediatype.h

   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/dsplayer_global.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowioreader.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowiosource.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowmediatypelist.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowplayercontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowplayerservice.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowsamplescheduler.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowvideorenderercontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/mediasamplevideobuffer.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/videosurfacefilter.h

   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowaudioendpointcontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowmetadatacontrol.h
   ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/vmr9videowindowcontrol.h
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")

   add_library(CsMultimedia_DirectShow MODULE "")
   add_library(CopperSpice::CsMultimedia_DirectShow ALIAS CsMultimedia_DirectShow)

   set_target_properties(CsMultimedia_DirectShow PROPERTIES OUTPUT_NAME CsMultimedia_DirectShow${BUILD_ABI} PREFIX "")

   target_sources(CsMultimedia_DirectShow
      PRIVATE

      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/directshow_plugin.cpp

      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_service.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_control.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_session.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_viewfindersettingscontrol.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dscamera_imageprocessingcontrol.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsimage_capturecontrol.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsvideo_renderer.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/camera/dsvideo_devicecontrol.cpp

      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshoweventloop.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowmediatype.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowpinenum.cpp

      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowioreader.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowiosource.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowmediatypelist.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowplayercontrol.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowplayerservice.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowsamplescheduler.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowvideorenderercontrol.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/mediasamplevideobuffer.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/videosurfacefilter.cpp

      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowaudioendpointcontrol.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/directshowmetadatacontrol.cpp
      ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/player/vmr9videowindowcontrol.cpp
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

   if(BUILDING_RPM)
      install(TARGETS CsMultimedia_DirectShow DESTINATION ${CMAKE_INSTALL_LIBDIR}/copperspice/plugins/mediaservices)
   else()
      install(TARGETS CsMultimedia_DirectShow DESTINATION ${CMAKE_INSTALL_LIBDIR})
   endif()
endif()
