GUI_DIRS += src/gui/effects

GUI_PUBLIC_INCLUDES += \
	QGraphicsEffect \
	QGraphicsBlurEffect \
	QGraphicsColorizeEffect \
	QGraphicsOpacityEffect \
	QGraphicsDropShadowEffect


GUI_EFFECTS_INCLUDES = \
	qgraphicsblureffect.h \
	qgraphicscolorizeeffect.h \
	qgraphicsdropshadoweffect.h \
	qgraphicsopacityeffect.h \
	qgraphicseffect.h

GUI_EFFECTS_PRIVATE_INCLUDES = \
	qgraphicseffect_p.h


GUI_INCLUDES += $(GUI_EFFECTS_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_EFFECTS_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/effects/%,$(GUI_EFFECTS_INCLUDES)) \
		$(patsubst %,src/gui/effects/%,$(GUI_EFFECTS_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/effects/qgraphicseffect.cpp
