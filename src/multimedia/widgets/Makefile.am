MULTIMEDIA_DIRS += src/multimedia/widgets

MULTIMEDIA_PUBLIC_INCLUDES += \
	QVideoWidget

MULTIMEDIA_WIDGETS_INCLUDES = \
	qgraphicsvideoitem.h \
	qvideowidget.h \
	qvideowidgetcontrol.h

MULTIMEDIA_WIDGETS_PRIVATE_INCLUDES = \
	qpaintervideosurface_p.h \
	qvideowidget_p.h

MULTIMEDIA_INCLUDES += $(MULTIMEDIA_WIDGETS_INCLUDES)
MULTIMEDIA_PRIVATE_INCLUDES += $(MULTIMEDIA_WIDGETS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/multimedia/widgets/%,$(MULTIMEDIA_WIDGETS_INCLUDES)) \
		$(patsubst %,src/multimedia/widgets/%,$(MULTIMEDIA_WIDGETS_PRIVATE_INCLUDES))

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES += \
	src/multimedia/widgets/qgraphicsvideoitem.cpp \
	src/multimedia/widgets/qpaintervideosurface.cpp \
	src/multimedia/widgets/qvideowidget.cpp \
	src/multimedia/widgets/qvideowidgetcontrol.cpp