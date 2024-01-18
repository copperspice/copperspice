/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QGRAPHICS_PROXYWIDGET_P_H
#define QGRAPHICS_PROXYWIDGET_P_H

#include <qgraphicsproxywidget.h>

#include <qgraphics_widget_p.h>

#if !defined(QT_NO_GRAPHICSVIEW)

class QGraphicsProxyWidgetPrivate : public QGraphicsWidgetPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsProxyWidget)

 public:
   QGraphicsProxyWidgetPrivate()
      : dragDropWidget(nullptr), posChangeMode(NoMode), sizeChangeMode(NoMode), visibleChangeMode(NoMode),
        enabledChangeMode(NoMode), styleChangeMode(NoMode), paletteChangeMode(NoMode),
        tooltipChangeMode(NoMode), focusFromWidgetToProxy(0)
   {
   }

   void init();
   void sendWidgetMouseEvent(QGraphicsSceneMouseEvent *event);
   void sendWidgetMouseEvent(QGraphicsSceneHoverEvent *event);
   void sendWidgetKeyEvent(QKeyEvent *event);
   void setWidget_helper(QWidget *widget, bool autoShow);

   QWidget *findFocusChild(QWidget *child, bool next) const;
   void removeSubFocusHelper(QWidget *widget, Qt::FocusReason reason);

   void _q_removeWidgetSlot();

   void embedSubWindow(QWidget *);
   void unembedSubWindow(QWidget *);

   bool isProxyWidget() const override;

   QPointer<QWidget> widget;
   QPointer<QWidget> lastWidgetUnderMouse;
   QPointer<QWidget> embeddedMouseGrabber;
   QWidget *dragDropWidget;
   Qt::DropAction lastDropAction;

   void updateWidgetGeometryFromProxy();
   void updateProxyGeometryFromWidget();

   void updateProxyInputMethodAcceptanceFromWidget();

   QPointF mapToReceiver(const QPointF &pos, const QWidget *receiver) const;

   enum ChangeMode {
      NoMode,
      ProxyToWidgetMode,
      WidgetToProxyMode
   };

   quint32 posChangeMode : 2;
   quint32 sizeChangeMode : 2;
   quint32 visibleChangeMode : 2;
   quint32 enabledChangeMode : 2;
   quint32 styleChangeMode : 2;
   quint32 paletteChangeMode : 2;
   quint32 tooltipChangeMode : 2;
   quint32 focusFromWidgetToProxy : 1;
   quint32 proxyIsGivingFocus : 1;
};

#endif

#endif
