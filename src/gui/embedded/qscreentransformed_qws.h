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

#ifndef QSCREENTRANSFORMED_QWS_H
#define QSCREENTRANSFORMED_QWS_H

#include <QtGui/qscreenproxy_qws.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_TRANSFORMED

class QTransformedScreenPrivate;

class QTransformedScreen : public QProxyScreen
{

 public:
   explicit QTransformedScreen(int display_id);
   ~QTransformedScreen();

   enum Transformation { None, Rot90, Rot180, Rot270 };

   void setTransformation(Transformation t);
   Transformation transformation() const;
   int transformOrientation() const;

   QSize mapToDevice(const QSize &s) const;
   QSize mapFromDevice(const QSize &s) const;

   QPoint mapToDevice(const QPoint &, const QSize &) const;
   QPoint mapFromDevice(const QPoint &, const QSize &) const;

   QRect mapToDevice(const QRect &, const QSize &) const;
   QRect mapFromDevice(const QRect &, const QSize &) const;

   QRegion mapToDevice(const QRegion &, const QSize &) const;
   QRegion mapFromDevice(const QRegion &, const QSize &) const;

   bool connect(const QString &displaySpec);

   bool isTransformed() const {
      return transformation() != None;
   }

   void exposeRegion(QRegion region, int changing);
   void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
   void solidFill(const QColor &color, const QRegion &region);
   void setDirty(const QRect &);

   QRegion region() const;

 private:
   friend class QTransformedScreenPrivate;
   QTransformedScreenPrivate *d_ptr;
};

#endif // QT_NO_QWS_TRANSFORMED

QT_END_NAMESPACE

#endif // QSCREENTRANSFORMED_QWS_H
