MULTIMEDIA_DIRS += src/multimedia/video

MULTIMEDIA_PUBLIC_INCLUDES += \
	QVideoFrame \
	QAbstractVideoBuffer \
	QAbstractVideoSurface \
	QVideoSurfaceFormat \
	QtMultimedia


MULTIMEDIA_VIDEO_INCLUDES = \
	qabstractvideobuffer.h \
	qabstractvideosurface.h \
	qvideoframe.h \
	qvideosurfaceformat.h \
	qtmultimedia.h

MULTIMEDIA_VIDEO_PRIVATE_INCLUDES = \
	qabstractvideobuffer_p.h \
	qabstractvideosurface_p.h \
	qimagevideobuffer_p.h \
	qmemoryvideobuffer_p.h

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_VIDEO_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_VIDEO_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/video/%,$(MULTIMEDIA_VIDEO_INCLUDES)) \
		$(patsubst %,src/multimedia/video/%,$(MULTIMEDIA_VIDEO_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/video/qabstractvideobuffer.cpp \
	src/multimedia/video/qabstractvideosurface.cpp \
	src/multimedia/video/qimagevideobuffer.cpp \
	src/multimedia/video/qmemoryvideobuffer.cpp \
	src/multimedia/video/qvideoframe.cpp \
	src/multimedia/video/qvideosurfaceformat.cpp