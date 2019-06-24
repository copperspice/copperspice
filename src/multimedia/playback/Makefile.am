MULTIMEDIA_DIRS += src/multimedia/playback

MULTIMEDIA_PUBLIC_INCLUDES += \
	QMediaContent \
	QMediaPlayer \
	QMediaPlayList \
	QMediaResource

MULTIMEDIA_PLAYBACK_INCLUDES = \
	qmediacontent.h \
	qmediaplayer.h \
	qmediaplaylist.h \
	qmediaresource.h

MULTIMEDIA_PLAYBACK_PRIVATE_INCLUDES = \
	qmediaplaylist_p.h \
	qmediaplaylistprovider_p.h \
	qmediaplaylistioplugin_p.h \
	qmediaplaylistnavigator_p.h \
	qmedianetworkplaylistprovider_p.h \
	playlistfileparser_p.h

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_PLAYBACK_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_PLAYBACK_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/playback/%,$(MULTIMEDIA_PLAYBACK_INCLUDES)) \
		$(patsubst %,src/multimedia/playback/%,$(MULTIMEDIA_PLAYBACK_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/playback/qmedianetworkplaylistprovider.cpp \
	src/multimedia/playback/qmediacontent.cpp \
	src/multimedia/playback/qmediaplayer.cpp \
	src/multimedia/playback/qmediaplaylist.cpp \
	src/multimedia/playback/qmediaplaylistioplugin.cpp \
	src/multimedia/playback/qmediaplaylistnavigator.cpp \
	src/multimedia/playback/qmediaplaylistprovider.cpp \
	src/multimedia/playback/qmediaresource.cpp \
	src/multimedia/playback/playlistfileparser.cpp
