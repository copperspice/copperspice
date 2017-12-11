add_definitions(
    -DMAKE_PHONON_LIB
    -DQT_NO_DBUS
    -DQT_BUILD_PHONON_LIB
)
set(EXTRA_PHONON_LIBS
    CsCore${BUILD_ABI}
    CsGui${BUILD_ABI}
    CsNetwork${BUILD_ABI}
    CsOpenGL${BUILD_ABI}
)
set(EXTRA_PHONON_CXXFLAGS)
set(EXTRA_PHONON_LDFLAGS)
set(EXTRA_PHONON2_LIBS ${EXTRA_PHONON_LIBS} CsPhonon${BUILD_ABI})
set(EXTRA_PHONON2_CXXFLAGS)
set(EXTRA_PHONON2_LDFLAGS)
set(PHONON2_SUFFIX)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
    ${CMAKE_BINARY_DIR}/include/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork
    ${CMAKE_BINARY_DIR}/privateinclude/QtNetwork/private
    ${CMAKE_BINARY_DIR}/include/phonon
    ${CMAKE_BINARY_DIR}/privateinclude/phonon
    ${CMAKE_BINARY_DIR}/privateinclude/phonon/private
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB2_INCLUDES}
    ${GOBJECT2_INCLUDES}
)

set(PHONON_PUBLIC_INCLUDES
    AbstractAudioOutput
    AbstractMediaStream
    AbstractVideoOutput
    AddonInterface
    AudioDataOutput
    AudioDataOutputInterface
    AudioOutput
    AudioOutputInterface
    BackendCapabilities
    BackendInterface
    Effect
    EffectInterface
    EffectParameter
    EffectWidget
    GlobalConfig
    MediaController
    MediaNode
    MediaObject
    MediaObjectInterface
    MediaSource
    ObjectDescription
    ObjectDescriptionModel
    Path
    PlatformPlugin
    PulseSupport
    SeekSlider
    StreamInterface
    VideoPlayer
    VideoWidget
    VideoWidgetInterface
    VolumeFaderEffect
    VolumeFaderInterface
    VolumeSlider
    AudioCaptureDevice
    SubtitleDescription
    EffectDescription
    ObjectDescriptionData
    AudioChannelDescription
    AudioCaptureDeviceModel
    AudioChannelDescriptionModel
    AudioOutputDevice
    AudioOutputDeviceModel
    ObjectDescriptionModelData
    EffectDescriptionModel
    SubtitleDescriptionModel
    AudioOutputInterface40
    AudioOutputInterface42
    VideoWidgetInterface44
    VideoWidgetInterfaceLatest
    Global
    phonon
)

set(PHONON_3RDPARTY_INCLUDES
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractaudiooutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractmediastream.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractvideooutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/addoninterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiodataoutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiodataoutputinterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiooutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiooutputinterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/backendcapabilities.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/backendinterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effect.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effectinterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effectparameter.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effectwidget.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/globalconfig.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediacontroller.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/medianode.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediaobject.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediaobjectinterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediasource.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/objectdescription.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/objectdescriptionmodel.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/path.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/phonondefs.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/phonon_export.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/phononnamespace.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/platformplugin.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/pulsesupport.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/seekslider.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/streaminterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/videoplayer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/videowidget.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/videowidgetinterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/volumefadereffect.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/volumefaderinterface.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/volumeslider.h
)

set(PHONON_MAIN_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/audiocapturedevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/audiochanneldescriptionmodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/audiooutputdevicemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/audiochanneldescription.h
    ${CMAKE_CURRENT_SOURCE_DIR}/audiocapturedevicemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/audiooutputdevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/phonon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/objectdescriptiondata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/effectdescription.h
    ${CMAKE_CURRENT_SOURCE_DIR}/audiooutputinterface40.h
    ${CMAKE_CURRENT_SOURCE_DIR}/videowidgetinterfacelatest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/subtitledescriptionmodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/effectdescriptionmodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/objectdescriptionmodeldata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/audiooutputinterface42.h
    ${CMAKE_CURRENT_SOURCE_DIR}/videowidgetinterface44.h
    ${CMAKE_CURRENT_SOURCE_DIR}/subtitledescription.h
    ${CMAKE_CURRENT_SOURCE_DIR}/global.h
)

set(PHONON_PRIVATE_INCLUDES
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractaudiooutput_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractmediastream_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractvideooutput_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiodataoutput_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiooutputadaptor_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiooutput_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/backendcapabilities_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effect_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effectparameter_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effectwidget_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/factory_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/frontendinterface_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/globalconfig_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/globalstatic_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/iodevicestream_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/medianode_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/medianodedestructionhandler_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediaobject_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediasource_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/objectdescription_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/objectdescriptionmodel_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/path_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/phonondefs_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/phononnamespace_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/platform_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/qsettingsgroup_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/seekslider_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/streaminterface_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/swiftslider_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/videowidget_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/volumefadereffect_p.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/volumeslider_p.h
)

set(PHONON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractaudiooutput.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractaudiooutput_p.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractmediastream.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractvideooutput.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/abstractvideooutput_p.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiodataoutput.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiooutput.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/audiooutputinterface.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/backendcapabilities.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effect.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effectparameter.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/effectwidget.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/factory.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/globalconfig.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/iodevicestream.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediacontroller.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/medianode.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediaobject.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/mediasource.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/objectdescription.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/objectdescriptionmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/path.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/phononnamespace.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/platform.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/pulsesupport.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/seekslider.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/streaminterface.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/swiftslider.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/videoplayer.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/videowidget.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/volumefadereffect.cpp
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/phonon/volumeslider.cpp

    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/abstractvideorenderer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/audiooutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/backend.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/backendnode.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/effect.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/fakesource.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/iodevicereader.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/mediagraph.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/mediaobject.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/videowidget.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/videorenderer_soft.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/volumeeffect.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qbasefilter.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qpin.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qasyncreader.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qaudiocdreader.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qmeminputpin.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/compointer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/phononds9_namespace.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qevr9.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/videorenderer_evr.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/videorenderer_vmr9.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/abstractrenderer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/artssink.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/audiodataoutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/audioeffect.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/audiooutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/backend.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/common.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/devicemanager.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/effect.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/effectmanager.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/glrenderer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/gsthelper.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/medianode.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/medianodeevent.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/mediaobject.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/message.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/phononsrc.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/qwidgetvideosink.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/streamreader.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/videowidget.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/volumefadereffect.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/widgetrenderer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/x11renderer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/medianode.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/backend.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/videowidget.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/mediaobject.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/quicktimevideoplayer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/backendheader.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/medianodevideopart.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/medianodeevent.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/quicktimeaudioplayer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audionode.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiograph.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiooutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/quicktimemetadata.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiomixer.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiodevice.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/backendinfo.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audioconnection.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiopartoutput.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/videoframe.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiosplitter.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audioeffects.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/quicktimestreamreader.h
    ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/mediaobjectaudionode.h
)

set(PHONON_INCLUDES ${PHONON_3RDPARTY_INCLUDES} ${PHONON_MAIN_INCLUDES})

macro_generate_public("${PHONON_PUBLIC_INCLUDES}" phonon)
macro_generate_private("${PHONON_PRIVATE_INCLUDES}" phonon)
macro_generate_misc("${PHONON_INCLUDES}" phonon)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(PHONON2_SOURCES
        ${PHONON2_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/abstractvideorenderer.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/audiooutput.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/backend.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/backendnode.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/effect.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/fakesource.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/iodevicereader.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/mediagraph.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/mediaobject.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/videowidget.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/videorenderer_soft.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/volumeeffect.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qbasefilter.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qpin.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qasyncreader.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qaudiocdreader.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/qmeminputpin.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/videorenderer_evr.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9/videorenderer_vmr9.cpp
    )
    set(EXTRA_PHONON2_LIBS
        ${EXTRA_PHONON2_LIBS}
        opengl32
        strmiids
        dmoguids
        uuid
        msdmo
        ole32
        oleaut32
        gdi32
    )
    add_definitions(
        -DQT_BUILD_DS9_LIB
    )
    include_directories(
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/ds9
    )
    set(PHONON2_SUFFIX "ds9")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
    set(PHONON2_SOURCES
        ${PHONON2_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/abstractrenderer.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/artssink.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/audiodataoutput.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/audioeffect.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/audiooutput.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/backend.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/devicemanager.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/effect.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/effectmanager.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/glrenderer.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/gsthelper.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/medianode.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/medianodeevent.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/mediaobject.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/message.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/phononsrc.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/qwidgetvideosink.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/streamreader.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/videowidget.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/volumefadereffect.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/widgetrenderer.cpp
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer/x11renderer.cpp
    )
    set(EXTRA_PHONON2_LIBS
        ${EXTRA_PHONON2_LIBS}
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARY}
        ${GSTREAMER_AUDIO_LIBRARY}
        ${GSTREAMER_VIDEO_LIBRARY}
        ${GSTREAMER_INTERFACE_LIBRARY}
        # ${LIBXML2_LIBRARIES}
    )
    add_definitions(
        -DQT_NO_OPENGL
        -DQT_BUILD_GSTREAMER_LIB
        ${GSTREAMER_DEFINITIONS}
        ${LIBXML2_DEFINITIONS}
    )
    include_directories(
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/gstreamer
        ${GSTREAMER_INCLUDE_DIR}
        ${LIBXML2_INCLUDE_DIR}
    )
    set(PHONON2_SUFFIX "gstreamer")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(PHONON2_SOURCES
        ${PHONON2_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/quicktimevideoplayer.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/backendheader.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/medianodevideopart.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/medianodeevent.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiooutput.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/backendinfo.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiosplitter.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audioeffects.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/quicktimestreamreader.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/medianode.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/backend.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/mediaobject.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/mediaobjectaudionode.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiomixer.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/quicktimeaudioplayer.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/videoframe.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/quicktimemetadata.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiodevice.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audioconnection.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audiograph.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/audionode.mm
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7/videowidget.mm
    )
    set(EXTRA_PHONON2_LDFLAGS
        ${EXTRA_PHONON2_LDFLAGS}
        -framework AppKit
        -framework AudioUnit
        -framework AudioToolbox
        -framework CoreAudio
        -framework QuartzCore
        -framework OpenGL
    )
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        set(EXTRA_PHONON2_LDFLAGS
            ${EXTRA_PHONON2_LDFLAGS}
            -framework QTKit
        )
    elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "i386")
        set(EXTRA_PHONON2_LDFLAGS
            ${EXTRA_PHONON2_LDFLAGS}
            -framework QuickTime
        )
    endif()
    add_definitions(
        -DQT_NO_OPENGL
        -DQT_BUILD_QT7_LIB
    )
    include_directories(
        ${CMAKE_SOURCE_DIR}/src/3rdparty/phonon/qt7
        ${CMAKE_BINARY_DIR}/include/QtOpenGL
    )
    set(PHONON2_SUFFIX "qt7")
endif()

function_variable_fixup("${EXTRA_PHONON_CXXFLAGS}" EXTRA_PHONON_CXXFLAGS)
function_variable_fixup("${EXTRA_PHONON_LDFLAGS}" EXTRA_PHONON_LDFLAGS)

add_library(CsPhonon${BUILD_ABI} SHARED ${PHONON_SOURCES} ${PHONON_INCLUDES})
target_link_libraries(CsPhonon${BUILD_ABI} ${EXTRA_PHONON_LIBS})

target_include_directories(CsPhonon${BUILD_ABI}
   PUBLIC
   $<INSTALL_INTERFACE:include/phonon>
)

set_target_properties(CsPhonon${BUILD_ABI} PROPERTIES
   VERSION "0" # for compat
   SOVERSION "0.0.0" # for compat
   COMPILE_FLAGS ${EXTRA_PHONON_CXXFLAGS}
   LINK_FLAGS ${EXTRA_PHONON_LDFLAGS}
)

function_variable_fixup("${EXTRA_PHONON2_CXXFLAGS}" EXTRA_PHONON2_CXXFLAGS)
function_variable_fixup("${EXTRA_PHONON2_LDFLAGS}" EXTRA_PHONON2_LDFLAGS)

add_library(phonon_${PHONON2_SUFFIX} MODULE ${PHONON2_SOURCES} ${PHONON_INCLUDES})
target_link_libraries(phonon_${PHONON2_SUFFIX} ${EXTRA_PHONON2_LIBS})

set_target_properties(phonon_${PHONON2_SUFFIX} PROPERTIES
   COMPILE_FLAGS ${EXTRA_PHONON2_CXXFLAGS}
   LINK_FLAGS ${EXTRA_PHONON2_LDFLAGS}
   PREFIX ""
)

if(${PHONON2_SUFFIX} STREQUAL "gstreamer")
    target_include_directories(phonon_${PHONON2_SUFFIX} PRIVATE ${X11_X11_INCLUDE_PATH})
endif()

macro_generate_resources("${PHONON_SOURCES}")
macro_generate_resources("${PHONON2_SOURCES}")
macro_generate_package(
   CsPhonon${BUILD_ABI}
   phonon
   "${EXTRA_PHONON_CXXFLAGS}"
   "CsCore${BUILD_ABI} CsGui${BUILD_ABI} CsNetwork${BUILD_ABI}"
)
macro_generate_package(
   phonon_${PHONON2_SUFFIX}
   phonon
   "${EXTRA_PHONON2_CXXFLAGS}"
   "CsCore${BUILD_ABI} CsGui${BUILD_ABI} CsNetwork${BUILD_ABI} CsPhonon${BUILD_ABI}"
)

install(
   TARGETS CsPhonon${BUILD_ABI}
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
   DIRECTORY ${CMAKE_BINARY_DIR}/include/phonon
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT Devel
)

install(
   TARGETS phonon_${PHONON2_SUFFIX}
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
