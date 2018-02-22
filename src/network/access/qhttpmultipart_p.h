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

#ifndef QHTTPMULTIPART_P_H
#define QHTTPMULTIPART_P_H

#include <qshareddata.h>
#include <qhttppart.h>
#include <qnetworkrequest_p.h>       // for deriving QHttpPartPrivate from QNetworkHeadersPrivate

class QHttpMultiPartPrivate;

class QHttpPartPrivate: public QSharedData, public QNetworkHeadersPrivate
{

 public:
   inline QHttpPartPrivate() : bodyDevice(0), headerCreated(false), readPointer(0) {
   }

   ~QHttpPartPrivate() {
   }

   QHttpPartPrivate(const QHttpPartPrivate &other)
      : QSharedData(other), QNetworkHeadersPrivate(other), body(other.body),
        header(other.header), headerCreated(other.headerCreated), readPointer(other.readPointer) {
      bodyDevice = other.bodyDevice;
   }

   inline bool operator==(const QHttpPartPrivate &other) const {
      return rawHeaders == other.rawHeaders && body == other.body &&
             bodyDevice == other.bodyDevice && readPointer == other.readPointer;
   }

   void setBodyDevice(QIODevice *device) {
      bodyDevice = device;
      readPointer = 0;
   }
   void setBody(const QByteArray &newBody) {
      body = newBody;
      readPointer = 0;
   }

   // QIODevice-style methods called by QHttpMultiPartIODevice (but this class is
   // not a QIODevice):
   qint64 bytesAvailable() const;
   qint64 readData(char *data, qint64 maxSize);
   qint64 size() const;
   bool reset();

   QByteArray body;
   QIODevice *bodyDevice;

 private:
   void checkHeaderCreated() const;

   mutable QByteArray header;
   mutable bool headerCreated;
   qint64 readPointer;
};

class QHttpMultiPartIODevice : public QIODevice
{
 public:
   QHttpMultiPartIODevice(QHttpMultiPartPrivate *parentMultiPart) :
      QIODevice(), multiPart(parentMultiPart), readPointer(0), deviceSize(-1) {
   }

   ~QHttpMultiPartIODevice() {
   }

   bool atEnd() const override {
      return readPointer == size();
   }

   qint64 bytesAvailable() const override {
      return size() - readPointer;
   }

   void close() override {
      readPointer = 0;
      partOffsets.clear();
      deviceSize = -1;
      QIODevice::close();
   }

   qint64 bytesToWrite() const override {
      return 0;
   }

   qint64 size() const override;
   bool isSequential() const override;
   bool reset() override;
   qint64 readData(char *data, qint64 maxSize) override;
   qint64 writeData(const char *data, qint64 maxSize) override;

   QHttpMultiPartPrivate *multiPart;
   qint64 readPointer;
   mutable QList<qint64> partOffsets;
   mutable qint64 deviceSize;
};

class QHttpMultiPartPrivate
{
 public:

   QHttpMultiPartPrivate();

   virtual ~QHttpMultiPartPrivate() {
      delete device;
   }

   QList<QHttpPart> parts;
   QByteArray boundary;
   QHttpMultiPart::ContentType contentType;
   QHttpMultiPartIODevice *device;

};

#endif // QHTTPMULTIPART_P_H
