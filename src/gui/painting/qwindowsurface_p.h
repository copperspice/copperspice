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

#ifndef QWINDOWSURFACE_P_H
#define QWINDOWSURFACE_P_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QPaintDevice;
class QRegion;
class QRect;
class QPoint;
class QImage;
class QWindowSurfacePrivate;
class QPlatformWindow;

class Q_GUI_EXPORT QWindowSurface
{
 public:
   enum WindowSurfaceFeature {
      PartialUpdates               = 0x00000001, // Supports doing partial updates.
      PreservedContents            = 0x00000002, // Supports doing flush without first doing a repaint.
      StaticContents               = 0x00000004, // Supports having static content regions when being resized.
      AllFeatures                  = 0xffffffff  // For convenience
   };
   using WindowSurfaceFeatures = QFlags<WindowSurfaceFeature>;

   QWindowSurface(QWidget *window, bool setDefaultSurface = true);
   virtual ~QWindowSurface();

   QWidget *window() const;

   virtual QPaintDevice *paintDevice() = 0;

   // 'widget' can be a child widget, in which case 'region' is in child widget coordinates and
   // offset is the (child) widget's offset in relation to the window surface. On QWS, 'offset'
   // can be larger than just the offset from the top-level widget as there may also be window
   // decorations which are painted into the window surface.
   virtual void flush(QWidget *widget, const QRegion &region, const QPoint &offset) = 0;

#if !defined(Q_WS_QPA)
   virtual void setGeometry(const QRect &rect);
   QRect geometry() const;
#else
   virtual void resize(const QSize &size);
   QSize size() const;
   inline QRect geometry() const {
      return QRect(QPoint(), size());   //### cleanup before Qt5
   }
#endif

   virtual bool scroll(const QRegion &area, int dx, int dy);

   virtual void beginPaint(const QRegion &);
   virtual void endPaint(const QRegion &);

   virtual QImage *buffer(const QWidget *widget);
   virtual QPixmap grabWidget(const QWidget *widget, const QRect &rectangle = QRect()) const;

   virtual QPoint offset(const QWidget *widget) const;
   inline QRect rect(const QWidget *widget) const;

   bool hasFeature(WindowSurfaceFeature feature) const;
   virtual WindowSurfaceFeatures features() const;

   void setStaticContents(const QRegion &region);
   QRegion staticContents() const;

 protected:
   bool hasStaticContents() const;

 private:
   QWindowSurfacePrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWindowSurface::WindowSurfaceFeatures)

inline QRect QWindowSurface::rect(const QWidget *widget) const
{
   return widget->rect().translated(offset(widget));
}

inline bool QWindowSurface::hasFeature(WindowSurfaceFeature feature) const
{
   return (features() & feature) != 0;
}

QT_END_NAMESPACE

#endif // QWINDOWSURFACE_P_H
