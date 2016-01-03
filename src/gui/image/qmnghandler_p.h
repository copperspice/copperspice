/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
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
   virtual bool canRead() const;
   virtual QByteArray name() const;
   virtual bool read(QImage *image);
   virtual bool write(const QImage &image);
   virtual int currentImageNumber() const;
   virtual int imageCount() const;
   virtual bool jumpToImage(int imageNumber);
   virtual bool jumpToNextImage();
   virtual int loopCount() const;
   virtual int nextImageDelay() const;
   static bool canRead(QIODevice *device);
   virtual QVariant option(ImageOption option) const;
   virtual void setOption(ImageOption option, const QVariant &value);
   virtual bool supportsOption(ImageOption option) const;

 private:
   Q_DECLARE_PRIVATE(QMngHandler)
   QScopedPointer<QMngHandlerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QMNGHANDLER_P_H
