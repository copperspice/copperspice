/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QWSMANAGER_QWS_H
#define QWSMANAGER_QWS_H

#include <QtGui/qpixmap.h>
#include <QtCore/qobject.h>
#include <QtGui/qdecoration_qws.h>
#include <QtGui/qevent.h>

#ifndef QT_NO_QWS_MANAGER

class QAction;
class QPixmap;
class QWidget;
class QPopupMenu;
class QRegion;
class QMouseEvent;
class QWSManagerPrivate;

class Q_GUI_EXPORT QWSManager : public QObject
{
   GUI_CS_OBJECT(QWSManager)
   Q_DECLARE_PRIVATE(QWSManager)

 public:
   explicit QWSManager(QWidget *);
   ~QWSManager();

   static QDecoration *newDefaultDecoration();

   QWidget *widget();
   static QWidget *grabbedMouse();
   void maximize();
   void startMove();
   void startResize();

   QRegion region();
   QRegion &cachedRegion();

 protected :
   GUI_CS_SLOT_1(Protected, void menuTriggered(QAction *action))
   GUI_CS_SLOT_2(menuTriggered)

   void handleMove(QPoint g);

   virtual bool event(QEvent *e);
   virtual void mouseMoveEvent(QMouseEvent *);
   virtual void mousePressEvent(QMouseEvent *);
   virtual void mouseReleaseEvent(QMouseEvent *);
   virtual void mouseDoubleClickEvent(QMouseEvent *);
   virtual void paintEvent(QPaintEvent *);
   bool repaintRegion(int region, QDecoration::DecorationState state);

   void menu(const QPoint &);

 private:
   friend class QWidget;
   friend class QETWidget;
   friend class QWidgetPrivate;
   friend class QApplication;
   friend class QApplicationPrivate;
   friend class QWidgetBackingStore;
   friend class QWSWindowSurface;
   friend class QGLDrawable;
};

#include <QtGui/qdecorationdefault_qws.h>

#endif // QT_NO_QWS_MANAGER

#endif // QWSMANAGER_QWS_H
