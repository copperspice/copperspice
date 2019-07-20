MULTIMEDIA_DIRS += src/plugins/multimedia/mediaservices/directshow \
   src/plugins/multimedia/mediaservices/directshow/kernel src/plugins/multimedia/mediaservices/directshow/player

MULTIMEDIA_DIRECTSHOW_PUBLIC_INCLUDES =

MULTIMEDIA_DIRECTSHOW_INCLUDES =

MULTIMEDIA_DIRECTSHOW_PRIVATE_INCLUDES = \
	directshow_plugin.h

MULTIMEDIA_DIRECTSHOW_KERNEL_PRIVATE_INCLUDES = \
	directshoweventloop.h \
	directshowglobal.h \
	directshowpinenum.h \
	directshowmediatype.h

MULTIMEDIA_DIRECTSHOW_PLAYER_PRIVATE_INCLUDES = \
	directshowioreader.h \
	directshowiosource.h \
	directshowmediatypelist.h \
	directshowplayercontrol.h \
	directshowplayerservice.h \
	directshowsamplescheduler.h \
	directshowvideorenderercontrol.h \
	mediasamplevideobuffer.h \
	videosurfacefilter.h \
	\
	directshowaudioendpointcontrol.h \
	directshowmetadatacontrol.h \
	vmr9videowindowcontrol.h

MULTIMEDIA_PUBLIC_INCLUDES += $(MULTIMEDIA_DIRECTSHOW_PUBLIC_INCLUDES)
MULTIMEDIA_INCLUDES += $(MULTIMEDIA_DIRECTSHOW_INCLUDES)

MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_DIRECTSHOW_PRIVATE_INCLUDES) \
   $(MULTIMEDIA_DIRECTSHOW_KERNEL_PRIVATE_INCLUDES) \
   $(MULTIMEDIA_DIRECTSHOW_PLAYER_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/multimedia/mediaservices/directshow/%,$(MULTIMEDIA_DIRECTSHOW_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/directshow/%,$(MULTIMEDIA_DIRECTSHOW_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/directshow/kernel/%,$(MULTIMEDIA_DIRECTSHOW_KERNEL_PRIVATE_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/mediaservices/directshow/player/%,$(MULTIMEDIA_DIRECTSHOW_PLAYER_PRIVATE_INCLUDES))

if OSTYPE_WIN

lib_LTLIBRARIES += lib/CsMultimedia_DirectShow@BUILD_ABI@.la

lib_CsMultimedia_DirectShow@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore/private  \
   -Iinclude/QtGui \
   -Iinclude/QtNetwork \
   -Iinclude/QtMultimedia -Iprivateinclude/QtMultimedia/private \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN \
   -DQMEDIA_DIRECTSHOW_PLAYER \
   -DNO_DSHOW_STRSAFE

lib_CsMultimedia_DirectShow@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   -avoid-version -no-undefined -module \
   -Wl,-lstrmiids -Wl,-ldmoguids -Wl,-luuid

lib_CsMultimedia_DirectShow@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la \
   lib/libCsGui@BUILD_ABI@.la \
   lib/libCsNetwork@BUILD_ABI@.la  \
   lib/libCsMultimedia@BUILD_ABI@.la \
   -lole32 -loleaut32 -lmsdmo -lgdi32

# if not using OpenGL
# lib_libCsMultimedia@BUILD_ABI@_la_LIBADD+= -lgdi32 -luser32

lib_CsMultimedia_DirectShow@BUILD_ABI@_la_SOURCES = \
	src/plugins/multimedia/mediaservices/directshow/directshow_plugin.cpp \
	\
	src/plugins/multimedia/mediaservices/directshow/kernel/directshoweventloop.cpp \
	src/plugins/multimedia/mediaservices/directshow/kernel/directshowmediatype.cpp \
	src/plugins/multimedia/mediaservices/directshow/kernel/directshowpinenum.cpp \
	\
	src/plugins/multimedia/mediaservices/directshow/player/directshowioreader.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/directshowiosource.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/directshowmediatypelist.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/directshowplayercontrol.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/directshowplayerservice.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/directshowsamplescheduler.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/directshowvideorenderercontrol.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/mediasamplevideobuffer.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/videosurfacefilter.cpp \
	\
	src/plugins/multimedia/mediaservices/directshow/player/directshowaudioendpointcontrol.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/directshowmetadatacontrol.cpp \
	src/plugins/multimedia/mediaservices/directshow/player/vmr9videowindowcontrol.cpp

endif