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

#ifndef QMNGHANDLER_P_H
#define QMNGHANDLER_P_H

#include <QtCore/qscopedpointer.h>
#include <QtGui/qimageiohandler.h>

QT_BEGIN_NAMESPACE

class QImage;
class QByteArray;
class QIODevice;
class QVariant;
class QMngHandlerPrivate;

class QMngHandler : public QImageIOHandler
{

 public:
   QMngHandler();
   ~QMngHandler();

   bool canRead() const override;
   QByteArray name() const override;
   bool read(QImage *image) override;
   bool write(const QImage &image) override;
   int currentImageNumber() const override;
   int imageCount() const override;

   bool jumpToImage(int imageNumber) override;
   bool jumpToNextImage() override;
   int loopCount() const override;
   int nextImageDelay() const override;

   static bool canRead(QIODevice *device);

   QVariant option(ImageOption option) const override;
   void setOption(ImageOption option, const QVariant &value) override;
   bool supportsOption(ImageOption option) const override;

 private:
   Q_DECLARE_PRIVATE(QMngHandler)
   QScopedPointer<QMngHandlerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QMNGHANDLER_P_H
