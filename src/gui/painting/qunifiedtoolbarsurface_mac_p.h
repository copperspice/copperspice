/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QUNIFIEDTOOLBARSURFACE_MAC_P_H
#define QUNIFIEDTOOLBARSURFACE_MAC_P_H

#include <qwindowsurface_raster_p.h>
#include <QWidget>
#include <QToolBar>
#include <qwidget_p.h>
#include <qnativeimage_p.h>

QT_BEGIN_NAMESPACE

class QNativeImage;

//
// This is the implementation of the unified toolbar on Mac OS X
// with the graphics system raster.
//
// General idea:
// -------------
// We redirect the painting of widgets inside the unified toolbar
// to a special window surface, the QUnifiedToolbarSurface.
// We need a separate window surface because the unified toolbar
// is out of the content view.
// The input system is the same as for the unified toolbar with the
// native (CoreGraphics) engine.
//
// Execution flow:
// ---------------
// The unified toolbar is triggered by QMainWindow::setUnifiedTitleAndToolBarOnMac().
// It calls QMainWindowLayout::insertIntoMacToolbar() which will
// set all the appropriate variables (offsets, redirection, ...).
// When Qt tells a widget to repaint, QWidgetPrivate::drawWidget()
// checks if the widget is inside the unified toolbar and exits without
// painting is that is the case.
// We trigger the rendering of the unified toolbar in QWidget::repaint()
// and QWidget::update().
// We keep track of flush requests via "flushRequested" variable. That
// allow flush() to be a no-op if no repaint occurred for a widget.
// We rely on the needsDisplay: and drawRect: mecanism for drawing our
// content into the graphics context.
//
// Notes:
// ------
// The painting of items inside the unified toolbar is expensive.
// Too many repaints will drastically slow down the whole application.
//

class QUnifiedToolbarSurfacePrivate
{
 public:
   virtual ~QUnifiedToolbarSurfacePrivate() {}

   QNativeImage *image;
   uint inSetGeometry : 1;
};

class Q_GUI_EXPORT QUnifiedToolbarSurface : public QRasterWindowSurface
{
 public:
   QUnifiedToolbarSurface(QWidget *widget);
   ~QUnifiedToolbarSurface();

   void flush(QWidget *widget);
   void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
   void setGeometry(const QRect &rect);
   void beginPaint(const QRegion &rgn);
   void insertToolbar(QWidget *toolbar, const QPoint &offset);
   void removeToolbar(QToolBar *toolbar);
   void updateToolbarOffset(QWidget *widget);
   void renderToolbar(QWidget *widget, bool forceFlush = false);
   void recursiveRedirect(QObject *widget, QWidget *parent_toolbar, const QPoint &offset);

   QPaintDevice *paintDevice();
   CGContextRef imageContext();

 private:
   void prepareBuffer(QImage::Format format, QWidget *widget);
   void recursiveRemoval(QObject *object);

   Q_DECLARE_PRIVATE(QUnifiedToolbarSurface)
   QScopedPointer<QUnifiedToolbarSurfacePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QUNIFIEDTOOLBARSURFACE_MAC_P_H
