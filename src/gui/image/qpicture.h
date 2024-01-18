/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#ifndef QPICTURE_H
#define QPICTURE_H

#include <qiodevice.h>
#include <qstringlist.h>
#include <qsharedpointer.h>
#include <qpaintdevice.h>

#ifndef QT_NO_PICTURE

class QPicturePrivate;

class Q_GUI_EXPORT QPicture : public QPaintDevice
{
 public:
   explicit QPicture(int formatVersion = -1);
   QPicture(const QPicture &other);

   ~QPicture();

   bool isNull() const;

   int devType() const override;
   uint size() const;

   const char *data() const;
   virtual void setData(const char *data, uint size);

   bool play(QPainter *painter);

   bool load(QIODevice *dev, const QString &format = QString());
   bool load(const QString &fileName, const QString &format = QString());
   bool save(QIODevice *dev, const QString &format = QString());
   bool save(const QString &fileName, const QString &format = QString());

   QRect boundingRect() const;
   void setBoundingRect(const QRect &rect);

   QPicture &operator=(const QPicture &other);

   QPicture &operator=(QPicture &&other) {
      qSwap(d_ptr, other.d_ptr);
      return *this;
   }

   void swap(QPicture &other) {
      d_ptr.swap(other.d_ptr);
   }

   void detach();
   bool isDetached() const;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPicture &picture);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPicture &picture);

   static QString pictureFormat(const QString &fileName);

   static QStringList inputFormats();
   static QStringList outputFormats();
   static QStringList inputFormatList();
   static QStringList outputFormatList();

   QPaintEngine *paintEngine() const override;

   using DataPtr = QExplicitlySharedDataPointer<QPicturePrivate>;

   DataPtr &data_ptr() {
      return d_ptr;
   }

 protected:
   QPicture(QPicturePrivate &data);

   int metric(PaintDeviceMetric m) const override;

 private:
   Q_DECLARE_PRIVATE(QPicture)

   bool exec(QPainter *p, QDataStream &ds, int i);
   QExplicitlySharedDataPointer<QPicturePrivate> d_ptr;

   friend class QPicturePaintEngine;
   friend class QAlphaPaintEngine;
   friend class QPreviewPaintEngine;
};

#ifndef QT_NO_PICTUREIO

class QIODevice;
class QPictureIO;

using picture_io_handler = void (*)(QPictureIO *);

struct QPictureIOData;

class Q_GUI_EXPORT QPictureIO
{
 public:
   QPictureIO();
   QPictureIO(QIODevice *ioDevice, const QString &format);
   QPictureIO(const QString &fileName, const QString &format);

   QPictureIO(const QPictureIO &) = delete;
   QPictureIO &operator=(const QPictureIO &) = delete;

   ~QPictureIO();

   const QPicture &picture() const;
   int status() const;
   QString format() const;
   QIODevice *ioDevice() const;
   QString fileName() const;
   int quality() const;
   QString description() const;
   const char *parameters() const;
   float gamma() const;

   void setPicture(const QPicture &picture);
   void setStatus(int status);
   void setFormat(const QString &format);
   void setIODevice(QIODevice *ioDevice);
   void setFileName(const QString &fileName);
   void setQuality(int q);
   void setDescription(const QString &description);
   void setParameters(const char *parameters);
   void setGamma(float gamma);

   bool read();
   bool write();

   static QString pictureFormat(const QString &fileName);
   static QString pictureFormat(QIODevice *ioDevice);
   static QStringList inputFormats();
   static QStringList outputFormats();

   static void defineIOHandler(const QString &format, const QString &header, const char *flags,
      picture_io_handler readPicture, picture_io_handler writePicture);

 private:
   void init();

   QPictureIOData *d;
};

#endif // QT_NO_PICTUREIO

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPicture &picture);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPicture &picture);

#endif // QT_NO_PICTURE

#endif
