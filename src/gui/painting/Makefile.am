GUI_DIRS += src/gui/painting

GUI_PUBLIC_INCLUDES += \
	QBackingStore \
	QBrush \
	QBrushData \
	QColor \
	QColormap \
	QConicalGradient \
	QGradient \
	QGradientStop \
	QGradientStops \
	QLinearGradient \
	QMatrix \
	QPaintDevice \
	QPaintEngine \
	QPaintEngineState \
	QPainter \
	QPainterPath \
	QPainterPathStroker \
	QPen \
	QPolygon \
	QPolygonF \
	QRadialGradient \
	QRegion \
	QRgb \
	QRgba64 \
	QStylePainter \
	QTextItem \
	QTileRules \
	QTransform

GUI_PAINTING_INCLUDES = \
	qbackingstore.h \
	qbrush.h \
	qbrushdata.h \
	qcolor.h \
	qcolormap.h \
	qconicalgradient.h \
	qdrawutil.h \
	qgradient.h \
	qgradientstop.h \
	qgradientstops.h \
	qlineargradient.h \
	qmatrix.h \
	qpaintdevice.h \
	qpaintengine.h \
	qpaintenginestate.h \
	qpainter.h \
	qpainterpath.h \
	qpainterpathstroker.h \
	qpen.h \
	qpolygon.h \
	qpolygonf.h \
	qradialgradient.h \
	qregion.h \
	qrgb.h \
	qrgba64.h \
	qstylepainter.h \
	qtextitem.h \
	qtilerules.h \
	qtransform.h

GUI_PAINTING_PRIVATE_INCLUDES = \
	qbezier_p.h \
	qblendfunctions_p.h \
	qblittable_p.h \
	qcolor_p.h \
	qcosmeticstroker_p.h \
	qcssutil_p.h \
	qdatabuffer_p.h \
	qdrawhelper_neon_p.h \
	qdrawhelper_p.h \
	qdrawhelper_x86_p.h \
	qdrawingprimitive_sse2_p.h \
	qemulationpaintengine_p.h \
	qfixed_p.h \
	qgrayraster_p.h \
	qimagescale_p.h \
	qmath_p.h \
	qmemrotate_p.h \
	qoutlinemapper_p.h \
	qpaintengine_alpha_p.h \
	qpaintengine_blitter_p.h \
	qpaintengine_p.h \
	qpaintengine_preview_p.h \
	qpaintengine_raster_p.h \
	qpaintengineex_p.h \
	qpainter_p.h \
	qpainterpath_p.h \
	qpathclipper_p.h \
	qpen_p.h \
	qpolygonclipper_p.h \
	qrasterdefs_p.h \
	qrasterizer_p.h \
	qrgba64_p.h \
	qstroker_p.h \
	qtextureglyphcache_p.h \
	qvectorpath_p.h \
	qwidgetbackingstore_p.h

GUI_INCLUDES += $(GUI_PAINTING_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_PAINTING_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/painting/%,$(GUI_PAINTING_INCLUDES)) \
		$(patsubst %,src/gui/painting/%,$(GUI_PAINTING_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/painting/qbezier.cpp \
	src/gui/painting/qblendfunctions.cpp \
	src/gui/painting/qbrush.cpp \
	src/gui/painting/qcolor.cpp \
	src/gui/painting/qcolor_p.cpp \
	src/gui/painting/qcolormap.cpp \
	src/gui/painting/qcompositionfunctions.cpp \
	src/gui/painting/qcosmeticstroker.cpp \
	src/gui/painting/qcssutil.cpp \
	src/gui/painting/qdrawutil.cpp \
	src/gui/painting/qdrawhelper.cpp \
	src/gui/painting/qdrawhelper_sse2.cpp \
	src/gui/painting/qdrawhelper_sse4.cpp \
	src/gui/painting/qdrawhelper_ssse3.cpp \
	src/gui/painting/qemulationpaintengine.cpp \
	src/gui/painting/qgammatables.cpp \
	src/gui/painting/qmatrix.cpp \
	src/gui/painting/qmemrotate.cpp \
	src/gui/painting/qoutlinemapper.cpp \
	src/gui/painting/qpaintdevice.cpp \
	src/gui/painting/qpaintengine.cpp \
	src/gui/painting/qpaintengine_alpha.cpp \
	src/gui/painting/qpaintengine_preview.cpp \
	src/gui/painting/qpaintengineex.cpp \
	src/gui/painting/qpainter.cpp \
	src/gui/painting/qpainterpath.cpp \
	src/gui/painting/qpathclipper.cpp \
	src/gui/painting/qpen.cpp \
	src/gui/painting/qpolygon.cpp \
	src/gui/painting/qrasterizer.cpp \
	src/gui/painting/qregion.cpp \
	src/gui/painting/qstroker.cpp \
	src/gui/painting/qstylepainter.cpp \
	src/gui/painting/qtextureglyphcache.cpp \
	src/gui/painting/qtransform.cpp \
	src/gui/painting/qpaintengine_raster.cpp \
	src/gui/painting/qimagescale.cpp \
	src/gui/painting/qgrayraster.c \
	src/gui/painting/qpaintengine_blitter.cpp \
	src/gui/painting/qblittable.cpp \
	src/gui/painting/qbackingstore.cpp \
	src/gui/painting/qwidgetbackingstore.cpp


if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_LDFLAGS += -lmsimg32
endif

