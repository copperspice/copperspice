GUI_DIRS += src/gui/graphicsview

GUI_PUBLIC_INCLUDES += \
	QGraphicsAnchor \
	QGraphicsAnchorLayout \
	QGraphicsEllipseItem \
	QGraphicsGridLayout \
	QGraphicsItem \
	QGraphicsItemAnimation \
	QGraphicsItemGroup \
	QGraphicsLayout \
	QGraphicsLayoutItem \
	QGraphicsLineItem \
	QGraphicsLinearLayout \
	QGraphicsObject \
	QGraphicsPathItem \
	QGraphicsPixmapItem \
	QGraphicsPolygonItem \
	QGraphicsProxyWidget \
	QGraphicsRectItem \
	QGraphicsRotation \
	QGraphicsScale \
	QGraphicsScene \
	QGraphicsSceneContextMenuEvent \
	QGraphicsSceneDragDropEvent \
	QGraphicsSceneEvent \
	QGraphicsSceneHelpEvent \
	QGraphicsSceneHoverEvent \
	QGraphicsSceneMouseEvent \
	QGraphicsSceneMoveEvent \
	QGraphicsSceneResizeEvent \
	QGraphicsSceneWheelEvent \
	QGraphicsSimpleTextItem \
	QGraphicsTextItem \
	QGraphicsTransform \
	QGraphicsView \
	QGraphicsWidget \
	QDeclarativeListProperty

GUI_GRAPHICSVIEW_INCLUDES = \
	qgraphicsanchor.h \
	qgraphicsanchorlayout.h \
	qgraphicsellipseitem.h \
	qgraphicsgridlayout.h \
	qgraphicsitem.h \
	qgraphicsitemanimation.h \
	qgraphicsitemgroup.h \
	qgraphicslayout.h \
	qgraphicslayoutitem.h \
	qgraphicslinearlayout.h \
	qgraphicslineitem.h \
	qgraphicsobject.h \
	qgraphicspathitem.h \
	qgraphicspixmapitem.h \
	qgraphicspolygonitem.h \
	qgraphicsproxywidget.h \
	qgraphicsrectitem.h \
	qgraphicsrotation.h \
	qgraphicsscale.h \
	qgraphicsscene.h \
	qgraphicsscenecontextmenuevent.h \
	qgraphicsscenedragdropevent.h \
	qgraphicssceneevent.h \
	qgraphicsscenehelpevent.h \
	qgraphicsscenehoverevent.h \
	qgraphicsscenemouseevent.h \
	qgraphicsscenemoveevent.h \
	qgraphicssceneresizeevent.h \
	qgraphicsscenewheelevent.h \
	qgraphicssimpletextitem.h \
	qgraphicstextitem.h \
	qgraphicstransform.h \
	qgraphicsview.h \
	qgraphicswidget.h  \
	qdeclarativelistproperty.h

GUI_GRAPHICSVIEW_PRIVATE_INCLUDES = \
	qgraph_p.h \
	qgraphicsanchorlayout_p.h \
	qgraphicsitem_p.h \
	qgraphicslayout_p.h \
	qgraphicslayoutitem_p.h \
	qgraphicsproxywidget_p.h \
	qgraphicsscene_bsp_p.h \
	qgraphicsscene_p.h \
	qgraphicsscenebsptreeindex_p.h \
	qgraphicssceneindex_p.h \
	qgraphicsscenelinearindex_p.h \
	qgraphicstransform_p.h \
	qgraphicsview_p.h \
	qgraphicswidget_p.h \
	qgridlayoutengine_p.h \
	qsimplex_p.h

GUI_INCLUDES += $(GUI_GRAPHICSVIEW_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_GRAPHICSVIEW_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/graphicsview/%,$(GUI_GRAPHICSVIEW_INCLUDES)) \
		$(patsubst %,src/gui/graphicsview/%,$(GUI_GRAPHICSVIEW_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
	src/gui/graphicsview/qgraphicsanchorlayout.cpp \
	src/gui/graphicsview/qgraphicsanchorlayout_p.cpp \
	src/gui/graphicsview/qgraphicsgridlayout.cpp \
	src/gui/graphicsview/qgraphicsitem.cpp \
	src/gui/graphicsview/qgraphicsitemanimation.cpp \
	src/gui/graphicsview/qgraphicslayout.cpp \
	src/gui/graphicsview/qgraphicslayout_p.cpp \
	src/gui/graphicsview/qgraphicslayoutitem.cpp \
	src/gui/graphicsview/qgraphicslinearlayout.cpp \
	src/gui/graphicsview/qgraphicsproxywidget.cpp \
	src/gui/graphicsview/qgraphicsscene.cpp \
	src/gui/graphicsview/qgraphicsscene_bsp.cpp \
	src/gui/graphicsview/qgraphicsscenebsptreeindex.cpp \
	src/gui/graphicsview/qgraphicssceneevent.cpp \
	src/gui/graphicsview/qgraphicssceneindex.cpp \
	src/gui/graphicsview/qgraphicstransform.cpp \
	src/gui/graphicsview/qgraphicsview.cpp \
	src/gui/graphicsview/qgraphicswidget.cpp \
	src/gui/graphicsview/qgraphicswidget_p.cpp \
	src/gui/graphicsview/qgridlayoutengine.cpp \
	src/gui/graphicsview/qsimplex_p.cpp
