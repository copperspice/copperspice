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

#include <qhttp_multipart.h>
#include <qhttp_multipart_p.h>

#include <qdatetime.h>        // for initializing the random number generator with QTime
#include <qmutex.h>
#include <qthreadstorage.h>

QHttpPart::QHttpPart() : d(new QHttpPartPrivate)
{
}

QHttpPart::QHttpPart(const QHttpPart &other) : d(other.d)
{
}

QHttpPart::~QHttpPart()
{
   d = nullptr;
}

QHttpPart &QHttpPart::operator=(const QHttpPart &other)
{
   d = other.d;
   return *this;
}

bool QHttpPart::operator==(const QHttpPart &other) const
{
   return d == other.d || *d == *other.d;
}

void QHttpPart::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
   d->setCookedHeader(header, value);
}

void QHttpPart::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
   d->setRawHeader(headerName, headerValue);
}

void QHttpPart::setBody(const QByteArray &body)
{
   d->setBody(body);
}

void QHttpPart::setBodyDevice(QIODevice *device)
{
   d->setBodyDevice(device);
}

QHttpMultiPart::QHttpMultiPart(QObject *parent)
   : QObject(parent), d_ptr(new QHttpMultiPartPrivate)
{
   Q_D(QHttpMultiPart);
   d->contentType = MixedType;
}

QHttpMultiPart::QHttpMultiPart(QHttpMultiPart::ContentType contentType, QObject *parent)
   : QObject(parent), d_ptr(new QHttpMultiPartPrivate)
{
   Q_D(QHttpMultiPart);
   d->contentType = contentType;
}

QHttpMultiPart::~QHttpMultiPart()
{
}

void QHttpMultiPart::append(const QHttpPart &httpPart)
{
   d_func()->parts.append(httpPart);
}

void QHttpMultiPart::setContentType(QHttpMultiPart::ContentType contentType)
{
   d_func()->contentType = contentType;
}

QByteArray QHttpMultiPart::boundary() const
{
   return d_func()->boundary;
}

void QHttpMultiPart::setBoundary(const QByteArray &boundary)
{
   d_func()->boundary = boundary;
}

qint64 QHttpPartPrivate::bytesAvailable() const
{
   checkHeaderCreated();
   qint64 bytesAvailable = header.count();

   if (bodyDevice) {
      bytesAvailable += bodyDevice->bytesAvailable() - readPointer;
   } else {
      bytesAvailable += body.count() - readPointer;
   }

   // the device might have closed etc., so make sure we do not return a negative value
   return qMax(bytesAvailable, (qint64) 0);
}

qint64 QHttpPartPrivate::readData(char *data, qint64 maxSize)
{
   checkHeaderCreated();
   qint64 bytesRead = 0;
   qint64 headerDataCount = header.count();

   // read header if it has not been read yet
   if (readPointer < headerDataCount) {
      bytesRead = qMin(headerDataCount - readPointer, maxSize);
      const char *headerData = header.constData();
      memcpy(data, headerData + readPointer, bytesRead);
      readPointer += bytesRead;
   }

   // read content if there is still space
   if (bytesRead < maxSize) {
      if (bodyDevice) {
         qint64 dataBytesRead = bodyDevice->read(data + bytesRead, maxSize - bytesRead);
         if (dataBytesRead == -1) {
            return -1;
         }
         bytesRead += dataBytesRead;
         readPointer += dataBytesRead;
      } else {
         qint64 contentBytesRead = qMin(body.count() - readPointer + headerDataCount, maxSize - bytesRead);
         const char *contentData = body.constData();
         // if this method is called several times, we need to find the
         // right offset in the content ourselves:
         memcpy(data + bytesRead, contentData + readPointer - headerDataCount, contentBytesRead);
         bytesRead += contentBytesRead;
         readPointer += contentBytesRead;
      }
   }

   return bytesRead;
}

qint64 QHttpPartPrivate::size() const
{
   checkHeaderCreated();
   qint64 size = header.count();

   if (bodyDevice) {
      size += bodyDevice->size();
   } else {
      size += body.count();
   }

   return size;
}

bool QHttpPartPrivate::reset()
{
   bool ret = true;
   if (bodyDevice) {
      if (! bodyDevice->reset()) {
         ret = false;
      }
   }

   readPointer = 0;

   return ret;
}

void QHttpPartPrivate::checkHeaderCreated() const
{
   if (! headerCreated) {
      // copied from QHttpNetworkRequestPrivate::header() and adapted
      QList<QPair<QByteArray, QByteArray> > fields = allRawHeaders();
      QList<QPair<QByteArray, QByteArray> >::const_iterator it = fields.constBegin();

      for (; it != fields.constEnd(); ++it) {
         header += it->first + ": " + it->second + "\r\n";
      }

      header += "\r\n";
      headerCreated = true;
   }
}

static QThreadStorage<bool *> *seedCreatedStorage()
{
   static QThreadStorage<bool *> retval;
   return &retval;
}

QHttpMultiPartPrivate::QHttpMultiPartPrivate() : contentType(QHttpMultiPart::MixedType),
   device(new QHttpMultiPartIODevice(this))
{
   if (!seedCreatedStorage()->hasLocalData()) {
      qsrand(QTime(0, 0, 0).msecsTo(QTime::currentTime()) ^ reinterpret_cast<quintptr>(this));
      seedCreatedStorage()->setLocalData(new bool(true));
   }

   boundary = QByteArray("boundary_.oOo._")
              + QByteArray::number(qrand()).toBase64()
              + QByteArray::number(qrand()).toBase64()
              + QByteArray::number(qrand()).toBase64();

   // boundary must not be longer than 70 characters, see RFC 2046, section 5.1.1
   if (boundary.count() > 70) {
      boundary = boundary.left(70);
   }
}

qint64 QHttpMultiPartIODevice::size() const
{
   // if not done yet, we calculate the size and the offsets of each part,
   // including boundary (needed later in readData)
   if (deviceSize == -1) {
      qint64 currentSize = 0;
      qint64 boundaryCount = multiPart->boundary.count();
      for (int a = 0; a < multiPart->parts.count(); a++) {
         partOffsets.append(currentSize);
         // 4 additional bytes for the "--" before and the "\r\n" after the boundary,
         // and 2 bytes for the "\r\n" after the content
         currentSize += boundaryCount + 4 + multiPart->parts.at(a).d->size() + 2;
      }
      currentSize += boundaryCount + 6; // size for ending boundary, 2 beginning and ending dashes and "\r\n"
      deviceSize = currentSize;
   }
   return deviceSize;
}

bool QHttpMultiPartIODevice::isSequential() const
{
   for (int a = 0; a < multiPart->parts.count(); a++) {
      QIODevice *device = multiPart->parts.at(a).d->bodyDevice;
      // we are sequential if any of the bodyDevices of our parts are sequential;
      // when reading from a byte array, we are not sequential
      if (device && device->isSequential()) {
         return true;
      }
   }
   return false;
}

bool QHttpMultiPartIODevice::reset()
{
   for (int a = 0; a < multiPart->parts.count(); a++)
      if (!multiPart->parts[a].d->reset()) {
         return false;
      }
   readPointer = 0;
   return true;
}
qint64 QHttpMultiPartIODevice::readData(char *data, qint64 maxSize)
{
   qint64 bytesRead = 0, index = 0;

   // skip the parts we have already read
   while (index < multiPart->parts.count() &&
          readPointer >= partOffsets.at(index) + multiPart->parts.at(index).d->size()
          + multiPart->boundary.count() + 6) {
      // 6 == 2 boundary dashes, \r\n after boundary, \r\n after multipart
      index++;
   }

   // read the data
   while (bytesRead < maxSize && index < multiPart->parts.count()) {

      // check whether we need to read the boundary of the current part
      QByteArray boundaryData = "--" + multiPart->boundary + "\r\n";
      qint64 boundaryCount = boundaryData.count();
      qint64 partIndex = readPointer - partOffsets.at(index);
      if (partIndex < boundaryCount) {
         qint64 boundaryBytesRead = qMin(boundaryCount - partIndex, maxSize - bytesRead);
         memcpy(data + bytesRead, boundaryData.constData() + partIndex, boundaryBytesRead);
         bytesRead += boundaryBytesRead;
         readPointer += boundaryBytesRead;
         partIndex += boundaryBytesRead;
      }

      // check whether we need to read the data of the current part
      if (bytesRead < maxSize && partIndex >= boundaryCount && partIndex < boundaryCount + multiPart->parts.at(index).d->size()) {
         qint64 dataBytesRead = multiPart->parts[index].d->readData(data + bytesRead, maxSize - bytesRead);

         if (dataBytesRead == -1) {
            return -1;
         }
         bytesRead += dataBytesRead;
         readPointer += dataBytesRead;
         partIndex += dataBytesRead;
      }

      // check whether we need to read the ending CRLF of the current part
      if (bytesRead < maxSize && partIndex >= boundaryCount + multiPart->parts.at(index).d->size()) {
         if (bytesRead == maxSize - 1) {
            return bytesRead;
         }
         memcpy(data + bytesRead, "\r\n", 2);
         bytesRead += 2;
         readPointer += 2;
         index++;
      }
   }

   // check whether we need to return the final boundary
   if (bytesRead < maxSize && index == multiPart->parts.count()) {
      QByteArray finalBoundary = "--" + multiPart->boundary + "--\r\n";
      qint64 boundaryIndex = readPointer + finalBoundary.count() - size();
      qint64 lastBoundaryBytesRead = qMin(finalBoundary.count() - boundaryIndex, maxSize - bytesRead);
      memcpy(data + bytesRead, finalBoundary.constData() + boundaryIndex, lastBoundaryBytesRead);
      bytesRead += lastBoundaryBytesRead;
      readPointer += lastBoundaryBytesRead;
   }

   return bytesRead;
}

qint64 QHttpMultiPartIODevice::writeData(const char *data, qint64 maxSize)
{
   (void) data;
   (void) maxSize;

   return -1;
}
