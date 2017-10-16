GUI_DIRS += src/gui/painting

GUI_PUBLIC_INCLUDES += \
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
	QPainterPathPrivate \
	QPainterPathStroker \
	QPen \
	QPolygon \
	QPolygonF \
	QPrintEngine \
	QPrinter \
	QPrinterInfo \
	QRadialGradient \
	QRegion \
	QRgb \
	QStylePainter \
	QTextItem \
	QTileRules \
	QTransform

GUI_PAINTING_INCLUDES = \
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
	qpainterpathprivate.h \
	qpainterpathstroker.h \
	qpen.h \
	qpolygon.h \
	qpolygonf.h \
	qprintengine.h \
	qprinter.h \
	qprinterinfo.h \
	qradialgradient.h \
	qregion.h \
	qrgb.h \
	qstylepainter.h \
	qtextitem.h \
	qtilerules.h \
	qtransform.h

GUI_PAINTING_PRIVATE_INCLUDES = \
	qbackingstore_p.h \
	qbezier_p.h \
	qblendfunctions_p.h \
	qblittable_p.h \
	qcolor_p.h \
	qcosmeticstroker_p.h \
	qcssutil_p.h \
	qcups_p.h \
	qdatabuffer_p.h \
	qdrawhelper_arm_simd_p.h \
	qdrawhelper_mmx_p.h \
	qdrawhelper_neon_p.h \
	qdrawhelper_p.h \
	qdrawhelper_sse_p.h \
	qdrawhelper_x86_p.h \
	qemulationpaintengine_p.h \
	qfixed_p.h \
	qgraphicssystem_p.h \
	qgraphicssystemplugin_p.h \
	qgraphicssystem_mac_p.h \
	qgraphicssystem_raster_p.h \
	qgraphicssystem_runtime_p.h \
	qgraphicssystemex_p.h \
	qgraphicssystemfactory_p.h \
	qgrayraster_p.h \
	qimagescale_p.h \
	qmath_p.h \
	qmemrotate_p.h \
	qoutlinemapper_p.h \
	qpaintengine_alpha_p.h \
	qpaintengine_blitter_p.h \
	qpaintengine_mac_p.h \
	qpaintengine_p.h \
	qpaintengine_preview_p.h \
	qpaintengine_raster_p.h \
	qpaintengine_x11_p.h \
	qpaintengineex_p.h \
	qpainter_p.h \
	qpainterpath_p.h \
	qpathclipper_p.h \
	qpdf_p.h \
	qpen_p.h \
	qpolygonclipper_p.h \
	qprintengine_pdf_p.h \
	qprintengine_mac_p.h \
        qpaintbuffer_p.h \
	qprintengine_ps_p.h \
	qprintengine_win_p.h \
	qprinter_p.h \
	qprinterinfo_p.h \
	qprinterinfo_unix_p.h \
	qrasterdefs_p.h \
	qrasterizer_p.h \
	qregion_mac.cpp \
	qstroker_p.h \
	qtessellator_p.h \
	qtextureglyphcache_p.h \
	qunifiedtoolbarsurface_mac_p.h \
	qvectorpath_p.h \
	qwindowsurface_p.h \
	qwindowsurface_mac_p.h \
	qunifiedtoolbarsurface_mac_p.h \
	qwindowsurface_raster_p.h \
	qwindowsurface_x11_p.h \
	qregion_x11.cpp \
	qregion_win.cpp


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
	src/gui/painting/qcosmeticstroker.cpp \
	src/gui/painting/qcssutil.cpp \
	src/gui/painting/qdrawutil.cpp \
	src/gui/painting/qemulationpaintengine.cpp \
	src/gui/painting/qgraphicssystem.cpp \
	src/gui/painting/qgraphicssystem_raster.cpp \
	src/gui/painting/qgraphicssystemfactory.cpp \
	src/gui/painting/qgraphicssystem_runtime.cpp \
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
	src/gui/painting/qpdf.cpp \
	src/gui/painting/qpen.cpp \
	src/gui/painting/qpolygon.cpp \
	src/gui/painting/qprintengine_pdf.cpp \
	src/gui/painting/qprintengine_ps.cpp \
	src/gui/painting/qprinter.cpp \
	src/gui/painting/qprinterinfo.cpp \
	src/gui/painting/qrasterizer.cpp \
	src/gui/painting/qregion.cpp \
	src/gui/painting/qstroker.cpp \
	src/gui/painting/qstylepainter.cpp \
	src/gui/painting/qtessellator.cpp \
	src/gui/painting/qtextureglyphcache.cpp \
	src/gui/painting/qtransform.cpp \
	src/gui/painting/qwindowsurface.cpp \
	src/gui/painting/qpaintbuffer.cpp \
	src/gui/painting/qpaintengine_raster.cpp \
	src/gui/painting/qdrawhelper.cpp \
	src/gui/painting/qimagescale.cpp \
	src/gui/painting/qgrayraster.c \
	src/gui/painting/qgraphicssystemplugin.cpp \
	src/gui/painting/qpaintengine_blitter.cpp \
	src/gui/painting/qblittable.cpp \
	src/gui/painting/qbackingstore.cpp \
	src/gui/painting/qwindowsurface_raster.cpp

if OSTYPE_WIN
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/painting/qcolormap_win.cpp \
	src/gui/painting/qpaintdevice_win.cpp \
	src/gui/painting/qprintengine_win.cpp \
	src/gui/painting/qprinterinfo_win.cpp

lib_libCsGui@BUILD_ABI@_la_LDFLAGS += -lmsimg32
endif

if X11
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/painting/qcolormap_x11.cpp \
	src/gui/painting/qpaintdevice_x11.cpp \
	src/gui/painting/qpaintengine_x11.cpp \
	src/gui/painting/qprinterinfo_unix.cpp \
	src/gui/painting/qcups.cpp \
	src/gui/painting/qwindowsurface_x11.cpp
endif

if COCOA
lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/painting/qcups.cpp \
	src/gui/painting/qcolormap_mac.cpp \
	src/gui/painting/qpaintdevice_mac.cpp \
	src/gui/painting/qpaintengine_mac.cpp \
	src/gui/painting/qgraphicssystem_mac.cpp \
	src/gui/painting/qprinterinfo_mac.cpp \
	src/gui/painting/qprintengine_mac.mm \
	src/gui/painting/qwindowsurface_mac.cpp \
	src/gui/painting/qunifiedtoolbarsurface_mac.cpp
endif
