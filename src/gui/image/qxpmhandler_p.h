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

#ifndef QXPMHANDLER_P_H
#define QXPMHANDLER_P_H

#include <QtGui/qimageiohandler.h>

#ifndef QT_NO_IMAGEFORMAT_XPM

QT_BEGIN_NAMESPACE

class QXpmHandler : public QImageIOHandler
{
 public:
   QXpmHandler();

   bool canRead() const override;
   bool read(QImage *image) override;
   bool write(const QImage &image) override;

   static bool canRead(QIODevice *device);

   QByteArray name() const override;

   QVariant option(ImageOption option) const override;
   void setOption(ImageOption option, const QVariant &value) override;
   bool supportsOption(ImageOption option) const override;

 private:
   bool readHeader();
   bool readImage(QImage *image);

   enum State {
      Ready,
      ReadHeader,
      Error
   };
   State state;

   int width;
   int height;
   int ncols;
   int cpp;
   QByteArray buffer;
   int index;
   QString fileName;
};

QT_END_NAMESPACE

#endif // QT_NO_IMAGEFORMAT_XPM

#endif // QXPMHANDLER_P_H
