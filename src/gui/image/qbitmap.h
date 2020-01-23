/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QBITMAP_H
#define QBITMAP_H

#include <qpixmap.h>


class QVariant;

class Q_GUI_EXPORT QBitmap : public QPixmap
{

 public:
   QBitmap();
   QBitmap(const QPixmap &);
   QBitmap(int w, int h);
   explicit QBitmap(const QSize &);
   explicit QBitmap(const QString &fileName, const char *format = nullptr);
   ~QBitmap();

   QBitmap &operator=(const QPixmap &);
   inline void swap(QBitmap &other) {
      QPixmap::swap(other);   // prevent QBitmap<->QPixmap swaps
   }
   operator QVariant() const;

   inline void clear() {
      fill(Qt::color0);
   }

   static QBitmap fromImage(const QImage &image, Qt::ImageConversionFlags flags = Qt::AutoColor);
   static QBitmap fromData(const QSize &size, const uchar *bits,
      QImage::Format monoFormat = QImage::Format_MonoLSB);

   QBitmap transformed(const QMatrix &) const;
   QBitmap transformed(const QTransform &matrix) const;

   using DataPtr = QExplicitlySharedDataPointer<QPlatformPixmap>;
};




#endif
