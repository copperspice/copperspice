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

#ifndef QBLITTABLE_P_H
#define QBLITTABLE_P_H

#include <QtCore/qsize.h>
#include <qpixmap_blitter_p.h>

#ifndef QT_NO_BLITTABLE
QT_BEGIN_NAMESPACE

class QImage;
class QBlittablePrivate;

class Q_GUI_EXPORT QBlittable
{
   Q_DECLARE_PRIVATE(QBlittable);

 public:
   enum Capability {

      SolidRectCapability              = 0x0001,
      SourcePixmapCapability           = 0x0002,
      SourceOverPixmapCapability       = 0x0004,
      SourceOverScaledPixmapCapability = 0x0008,
      AlphaFillRectCapability          = 0x0010,
      OpacityPixmapCapability          = 0x0020,

      // Internal ones
      OutlineCapability                = 0x0001000,
   };
   using Capabilities = QFlags<Capability>;

   QBlittable (const QSize &size, Capabilities caps);
   virtual ~QBlittable();

   Capabilities capabilities() const;
   QSize size() const;

   virtual void fillRect(const QRectF &rect, const QColor &color) = 0;
   virtual void drawPixmap(const QRectF &rect, const QPixmap &pixmap, const QRectF &subrect) = 0;
   virtual void alphaFillRect(const QRectF &rect, const QColor &color, QPainter::CompositionMode cmode) {
      Q_UNUSED(rect);
      Q_UNUSED(color);
      Q_UNUSED(cmode);
      qWarning("Please implement alphaFillRect function in your platform or remove AlphaFillRectCapability from it");
   }

   virtual void drawPixmapOpacity(const QRectF &rect, const QPixmap &pixmap, const QRectF &subrect,
                                  QPainter::CompositionMode cmode, qreal opacity) {
      Q_UNUSED(rect);
      Q_UNUSED(pixmap);
      Q_UNUSED(subrect);
      Q_UNUSED(cmode);
      Q_UNUSED(opacity);
      qWarning("Please implement drawPixmapOpacity function in your platform or remove OpacityPixmapCapability from it");
   }

   bool isLocked() const;

   QImage *lock();
   void unlock();

 protected:
   virtual QImage *doLock() = 0;
   virtual void doUnlock() = 0;
   QBlittablePrivate *d_ptr;
};

QT_END_NAMESPACE

#endif //QT_NO_BLITTABLE
#endif //QBLITTABLE_P_H
