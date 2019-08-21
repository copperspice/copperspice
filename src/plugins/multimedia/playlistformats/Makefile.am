MULTIMEDIA_DIRS += src/plugins/multimedia/playlistformats

MULTIMEDIA_PLAYLISTFORMATS_PUBLIC_INCLUDES =

MULTIMEDIA_PLAYLISTFORMATS_INCLUDES =

MULTIMEDIA_PLAYLISTFORMATS_PRIVATE_INCLUDES = qm3u_plugin.h

MULTIMEDIA_PUBLIC_INCLUDES += $(MULTIMEDIA_PLAYLISTFORMATS_PUBLIC_INCLUDES)
MULTIMEDIA_INCLUDES += $(MULTIMEDIA_PLAYLISTFORMATS_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_PLAYLISTFORMATS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/plugins/multimedia/playlistformats/%,$(MULTIMEDIA_PLAYLISTFORMATS_INCLUDES)) \
	$(patsubst %,src/plugins/multimedia/playlistformats/%,$(MULTIMEDIA_PLAYLISTFORMATS_PRIVATE_INCLUDES))

if MULTIMEDIA
lib_LTLIBRARIES += lib/CsMultimedia_m3u@BUILD_ABI@.la
endif

lib_CsMultimedia_m3u@BUILD_ABI@_la_CXXFLAGS = -Iinclude -Iprivateinclude \
   -Iinclude/QtCore  -Iprivateinclude/QtCore  -Iinclude/QtNetwork \
   -Iinclude/QtMultimedia -Iprivateinclude/QtMultimedia/private \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_PLUGIN

lib_CsMultimedia_m3u@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) \
   -avoid-version -no-undefined -module

lib_CsMultimedia_m3u@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la lib/libCsNetwork@BUILD_ABI@.la \
   lib/libCsMultimedia@BUILD_ABI@.la

lib_CsMultimedia_m3u@BUILD_ABI@_la_SOURCES = \
	src/plugins/multimedia/playlistformats/qm3u_plugin.cpp
