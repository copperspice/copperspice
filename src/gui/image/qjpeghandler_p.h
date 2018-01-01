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

#ifndef QJPEGHANDLER_P_H
#define QJPEGHANDLER_P_H

#include <QtGui/qimageiohandler.h>
#include <QtCore/QSize>
#include <QtCore/QRect>

QT_BEGIN_NAMESPACE

class QJpegHandlerPrivate;
class QJpegHandler : public QImageIOHandler
{
 public:
   QJpegHandler();
   ~QJpegHandler();

   bool canRead() const override;
   bool read(QImage *image) override;
   bool write(const QImage &image) override;

   QByteArray name() const override;

   static bool canRead(QIODevice *device);

   QVariant option(ImageOption option) const override;
   void setOption(ImageOption option, const QVariant &value) override;
   bool supportsOption(ImageOption option) const override;

 private:
   QJpegHandlerPrivate *d;
};

QT_END_NAMESPACE

#endif // QJPEGHANDLER_P_H
