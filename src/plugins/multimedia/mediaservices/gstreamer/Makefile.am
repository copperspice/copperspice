MULTIMEDIA_DIRS += src/plugins/multimedia/mediaservices/gstreamer \
   src/plugins/multimedia/mediaservices/gstreamer/audiodecoder  \
   src/plugins/multimedia/mediaservices/gstreamer/mediaplayer \
   src/plugins/multimedia/mediaservices/gstreamer/tools

MULTIMEDIA_GSTREAMER_PUBLIC_INCLUDES =

MULTIMEDIA_GSTREAMER_INCLUDES =

MULTIMEDIA_GSTREAMER_PRIVATE_INCLUDES =

MULTIMEDIA_GSTREAMER_AUDIODECODER_PRIVATE_INCLUDES = \
	qgstreameraudiodecodercontrol.h \
	qgstreameraudiodecoderservice.h \
	qgstreameraudiodecodersession.h \
	qgstreameraudiodecoderserviceplugin.h

MULTIMEDIA_GSTREAMER_MEDIAPLAYER_PRIVATE_INCLUDES = \
	qgstreameravailabilitycontrol.h \
	qgstreamermetadataprovider.h \
	qgstreamerplayerservice.h \
	qgstreamerplayerserviceplugin.h \
	qgstreamerstreamscontrol.h \
	qgstreamerplayercontrol.h \
	qgstreamerplayersession.h

MULTIMEDIA_GSTREAMER_TOOLS_PRIVATE_INCLUDES = \
	qgstutils_p.h \
	qgstreameraudioprobecontrol_p.h \
	qgstreamerbufferprobe_p.h \
	qgstreamerbushelper_p.h \
	qgstreamermessage_p.h \
	qgstreamervideowidget_p.h \
	qgstreamervideoinputdevicecontrol_p.h \
	qgstreamervideoprobecontrol_p.h \
	qgstreamervideorendererinterface_p.h \
	qgstreamervideooverlay_p.h \
	qgstreamervideowindow_p.h \
	qgstreamervideorenderer_p.h \
	qvideosurfacegstsink_p.h \
	qgstvideorenderersink_p.h \
	qgstvideorendererplugin_p.h \
	qgstvideobuffer_p.h

MULTIMEDIA_PUBLIC_INCLUDES += $(MULTIMEDIA_GSTREAMER_PUBLIC_INCLUDES)
MULTIMEDIA_INCLUDES += $(MULTIMEDIA_GSTREAMER_INCLUDES)

MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_GSTREAMER_PRIVATE_INCLUDES) \
   $(MULTIMEDIA_GSTREAMER_AUDIODECODER_PRIVATE_INCLUDES) \
   $(MULTIMEDIA_GSTREAMER_MEDIAPLAYER_PRIVATE_INCLUDES) \
   $(MULTIMEDIA_GSTREAMER_TOOLS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/multimedia/mediaservices/gstreamer/%,$(MULTIMEDIA_GSTREAMER_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/gstreamer/%,$(MULTIMEDIA_GSTREAMER_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/%,$(MULTIMEDIA_GSTREAMER_AUDIODECODER_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/%,$(MULTIMEDIA_GSTREAMER_MEDIAPLAYER_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/gstreamer/tools/%,$(MULTIMEDIA_GSTREAMER_TOOLS_PRIVATE_INCLUDES))


if MULTIMEDIA

if GSTREAMER

lib_LTLIBRARIES += \
   lib/CsMultimedia_gst_audiodecoder@BUILD_ABI@.la \
   lib/CsMultimedia_gst_mediaplayer@BUILD_ABI@.la

# plugin 1
lib_CsMultimedia_gst_audiodecoder@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore/private  \
   -Iinclude/QtGui \
   -Iinclude/QtNetwork \
   -Iinclude/QtMultimedia -Iprivateinclude/QtMultimedia/private \
   $(AM_CXXFLAGS) \
   ${GSTREAMER_CFLAGS} \
   ${GLIB_CFLAGS} \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsMultimedia_gst_audiodecoder@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   -avoid-version -no-undefined -module

lib_CsMultimedia_gst_audiodecoder@BUILD_ABI@_la_LIBADD = \
   lib/libCsCore@BUILD_ABI@.la \
   lib/libCsGui@BUILD_ABI@.la \
   lib/libCsNetwork@BUILD_ABI@.la \
   lib/libCsMultimedia@BUILD_ABI@.la \
   ${GSTREAMER_LIBS} \
   ${GSTREAMER_BASE_LIBS} \
   ${GSTREAMER_AUDIO_LIBS} \
   ${GSTREAMER_VIDEO_LIBS} \
   ${GSTREAMER_APP_LIBS} \
   ${GLIB_LIBS}

lib_CsMultimedia_gst_audiodecoder@BUILD_ABI@_la_SOURCES = \
	src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecodercontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecoderservice.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecodersession.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/audiodecoder/qgstreameraudiodecoderserviceplugin.cpp \
	\
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstutils.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreameraudioprobecontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamerbufferprobe.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamerbushelper.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamermessage.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideowidget.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideoinputdevicecontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideoprobecontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideorendererinterface.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideooverlay.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideowindow.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideorenderer.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideobuffer.cpp

# plugin 2
lib_CsMultimedia_gst_mediaplayer@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore/private  \
   -Iinclude/QtGui \
   -Iinclude/QtNetwork \
   -Iinclude/QtMultimedia -Iprivateinclude/QtMultimedia/private \
   $(AM_CXXFLAGS) \
   ${GSTREAMER_CFLAGS} \
   ${GLIB_CFLAGS} \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsMultimedia_gst_mediaplayer@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   -avoid-version -no-undefined -module

lib_CsMultimedia_gst_mediaplayer@BUILD_ABI@_la_LIBADD = \
   lib/libCsCore@BUILD_ABI@.la \
   lib/libCsGui@BUILD_ABI@.la \
   lib/libCsNetwork@BUILD_ABI@.la \
   lib/libCsMultimedia@BUILD_ABI@.la \
   ${GSTREAMER_LIBS} \
   ${GSTREAMER_BASE_LIBS} \
   ${GSTREAMER_AUDIO_LIBS} \
   ${GSTREAMER_VIDEO_LIBS} \
   ${GSTREAMER_APP_LIBS} \
   ${GLIB_LIBS}

lib_CsMultimedia_gst_mediaplayer@BUILD_ABI@_la_SOURCES = \
	src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayerservice.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerstreamscontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamermetadataprovider.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreameravailabilitycontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayerserviceplugin.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayercontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/mediaplayer/qgstreamerplayersession.cpp \
	\
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstutils.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreameraudioprobecontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamerbufferprobe.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamerbushelper.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamermessage.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideowidget.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideoinputdevicecontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideoprobecontrol.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideorendererinterface.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideooverlay.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideowindow.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstreamervideorenderer.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideobuffer.cpp


if GSTREAMER_ABI_VERSION_0_10

lib_CsMultimedia_gst_audiodecoder@BUILD_ABI@_la_SOURCES += \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qvideosurfacegstsink.cpp

lib_CsMultimedia_gst_mediaplayer@BUILD_ABI@_la_SOURCES += \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qvideosurfacegstsink.cpp

#   qgstbufferpoolinterface.cpp
#   gstvideoconnector.c

else

lib_CsMultimedia_gst_audiodecoder@BUILD_ABI@_la_SOURCES += \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorenderersink.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorendererplugin.cpp

lib_CsMultimedia_gst_mediaplayer@BUILD_ABI@_la_SOURCES += \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorenderersink.cpp \
	src/plugins/multimedia/mediaservices/gstreamer/tools/qgstvideorendererplugin.cpp

endif


endif

endif