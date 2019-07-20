MULTIMEDIA_DIRS += src/plugins/multimedia/mediaservices/avfoundation \
   src/plugins/multimedia/mediaservices/avfoundation/mediaplayer

MULTIMEDIA_AVFOUNDATION_PUBLIC_INCLUDES =

MULTIMEDIA_AVFOUNDATION_INCLUDES =

MULTIMEDIA_AVFOUNDATION_PRIVATE_INCLUDES =

MULTIMEDIA_AVFOUNDATION_MEDIAPLAYER_PRIVATE_INCLUDES = \
	avfdisplaylink.h \
	avfmediaplayercontrol.h \
	avfmediaplayermetadatacontrol.h \
	avfmediaplayerservice.h \
	avfmediaplayersession.h \
	avfmediaplayerserviceplugin.h \
	avfvideooutput.h \
	avfvideowindowcontrol.h \
	avfvideowidgetcontrol.h \
	avfvideowidget.h \
	avfvideorenderercontrol.h \
	avfvideoframerenderer.h

MULTIMEDIA_PUBLIC_INCLUDES += $(MULTIMEDIA_AVFOUNDATION_PUBLIC_INCLUDES)
MULTIMEDIA_INCLUDES += $(MULTIMEDIA_AVFOUNDATION_INCLUDES)

MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_AVFOUNDATION_PRIVATE_INCLUDES) \
   $(MULTIMEDIA_AVFOUNDATION_MEDIAPLAYER_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/multimedia/mediaservices/avfoundation/%,$(MULTIMEDIA_AVFOUNDATION_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/avfoundation/%,$(MULTIMEDIA_AVFOUNDATION_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/%,$(MULTIMEDIA_AVFOUNDATION_MEDIAPLAYER_PRIVATE_INCLUDES))


if OSTYPE_DARWIN

if MULTIMEDIA

lib_LTLIBRARIES += \
   lib/CsMultimedia_avf_mediaplayer@BUILD_ABI@.la

lib_CsMultimedia_avf_mediaplayer@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore/private  \
   -Iinclude/QtGui \
   -Iinclude/QtNetwork \
   -Iinclude/QtMultimedia -Iprivateinclude/QtMultimedia/private \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN \
   -DQMEDIA_AVF_MEDIAPLAYER

lib_CsMultimedia_avf_mediaplayer@BUILD_ABI@_la_OBJCXXFLAGS = $(lib_CsMultimedia_avf_mediaplayer@BUILD_ABI@_la_CXXFLAGS)

lib_CsMultimedia_avf_mediaplayer@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   -avoid-version -no-undefined -module \
   -framework AVFoundation \
   -framework CoreMedia \
   -framework QuartzCore \
   -framework AppKit \
   -framework OpenGL

lib_CsMultimedia_avf_mediaplayer@BUILD_ABI@_la_LIBADD = \
   lib/libCsCore@BUILD_ABI@.la \
   lib/libCsGui@BUILD_ABI@.la \
   lib/libCsNetwork@BUILD_ABI@.la \
   lib/libCsMultimedia@BUILD_ABI@.la

lib_CsMultimedia_avf_mediaplayer@BUILD_ABI@_la_SOURCES = \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfdisplaylink.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayercontrol.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayermetadatacontrol.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerservice.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayerserviceplugin.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfmediaplayersession.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideooutput.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowindowcontrol.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidgetcontrol.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideowidget.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideorenderercontrol.mm \
	src/plugins/multimedia/mediaservices/avfoundation/mediaplayer/avfvideoframerenderer.mm

endif

endif