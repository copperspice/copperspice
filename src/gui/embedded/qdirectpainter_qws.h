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

#ifndef QDIRECTPAINTER_QWS_H
#define QDIRECTPAINTER_QWS_H

#include <QtCore/qobject.h>
#include <QtGui/qregion.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DIRECTPAINTER
class QDirectPainterPrivate;
struct QWSEmbedEvent;

class Q_GUI_EXPORT QDirectPainter : public QObject
{
   GUI_CS_OBJECT(QDirectPainter)
   Q_DECLARE_PRIVATE(QDirectPainter)

 public:

   enum SurfaceFlag { NonReserved = 0,
                      Reserved = 1,
                      ReservedSynchronous = 3
                    };

   explicit QDirectPainter(QObject *parentObject = 0, SurfaceFlag flag = NonReserved);
   ~QDirectPainter();

   void setRegion(const QRegion &);
   QRegion requestedRegion() const;
   QRegion allocatedRegion() const;

   void setGeometry(const QRect &);
   QRect geometry() const;

   WId winId() const;
   virtual void regionChanged(const QRegion &exposedRegion);

   void startPainting(bool lockDisplay = true);
   void endPainting();
   void endPainting(const QRegion &region);
   void flush(const QRegion &region);

   void raise();
   void lower();

   static QRegion reserveRegion(const QRegion &);
   static QRegion reservedRegion();
   static QRegion region() {
      return reservedRegion();
   }

   static uchar *frameBuffer();
   static int screenDepth();
   static int screenWidth();
   static int screenHeight();
   static int linestep();

   static void lock();
   static void unlock();

 private:
   friend  void qt_directpainter_region(QDirectPainter *dp, const QRegion &alloc, int type);
   friend void qt_directpainter_embedevent(QDirectPainter *, const QWSEmbedEvent *);
};

#endif

QT_END_NAMESPACE

#endif // QDIRECTPAINTER_QWS_H
