set(EXTRA_DIRECTSHOW_LIBS CsCore${BUILD_ABI} CsMultimedia_DirectShow${BUILD_ABI})

set(MULTIMEDIA_PUBLIC_INCLUDES
    ${MULTIMEDIA_PUBLIC_INCLUDES}
)

set(MULTIMEDIA_PRIVATE_INCLUDES
    ${MULTIMEDIA_PRIVATE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/directshow_plugin.h

    ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshoweventloop.h
    ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowglobal.h
    ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowpinenum.h
    ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/kernel/directshowmediatype.h

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

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    set(MULTIMEDIA_DIRECTSHOW_SOURCES
        ${MULTIMEDIA_DIRECTSHOW_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/plugins/multimedia/mediaservices/directshow/directshow_plugin.cpp

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

    add_library(CsMultimedia_DirectShow${BUILD_ABI} MODULE ${MULTIMEDIA_DIRECTSHOW_SOURCES})

    target_link_libraries(CsMultimedia_DirectShow${BUILD_ABI}
       CsCore${BUILD_ABI}
       CsGui${BUILD_ABI}
       CsNetwork${BUILD_ABI}
       CsMultimedia${BUILD_ABI}
       strmiids
       dmoguids
       uuid
       ole32
       oleaut32
       msdmo
       gdi32
    )

    target_compile_definitions(CsMultimedia_DirectShow${BUILD_ABI} PRIVATE
       -DQMEDIA_DIRECTSHOW_PLAYER
       -DNO_DSHOW_STRSAFE
       -DQT_PLUGIN
    )

    install(TARGETS CsMultimedia_DirectShow${BUILD_ABI} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
