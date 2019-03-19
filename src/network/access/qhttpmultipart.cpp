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

#include <qhttpmultipart.h>
#include <qhttpmultipart_p.h>
#include <qdatetime.h>        // for initializing the random number generator with QTime
#include <qmutex.h>
#include <qthreadstorage.h>

QT_BEGIN_NAMESPACE

QHttpPart::QHttpPart() : d(new QHttpPartPrivate)
{
}

QHttpPart::QHttpPart(const QHttpPart &other) : d(other.d)
{
}

/*!
    Destroys this QHttpPart.
*/
QHttpPart::~QHttpPart()
{
   d = 0;
}

/*!
    Creates a copy of \a other.
*/
QHttpPart &QHttpPart::operator=(const QHttpPart &other)
{
   d = other.d;
   return *this;
}

/*!
    Returns true if this object is the same as \a other (i.e., if they
    have the same headers and body).

    \sa operator!=()
*/
bool QHttpPart::operator==(const QHttpPart &other) const
{
   return d == other.d || *d == *other.d;
}

/*!
    \fn bool QHttpPart::operator!=(const QHttpPart &other) const

    Returns true if this object is not the same as \a other.

    \sa operator==()
*/

/*!
    Sets the value of the known header \a header to be \a value,
    overriding any previously set headers.

    \sa QNetworkRequest::KnownHeaders, setRawHeader(), QNetworkRequest::setHeader()
*/
void QHttpPart::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
   d->setCookedHeader(header, value);
}

/*!
    Sets the header \a headerName to be of value \a headerValue. If \a
    headerName corresponds to a known header (see
    QNetworkRequest::KnownHeaders), the raw format will be parsed and
    the corresponding "cooked" header will be set as well.

    Note: setting the same header twice overrides the previous
    setting. To accomplish the behaviour of multiple HTTP headers of
    the same name, you should concatenate the two values, separating
    them with a comma (",") and set one single raw header.

    \sa QNetworkRequest::KnownHeaders, setHeader(), QNetworkRequest::setRawHeader()
*/
void QHttpPart::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
   d->setRawHeader(headerName, headerValue);
}

/*!
    Sets the body of this MIME part to \a body. The body set with this method
    will be used unless the device is set via setBodyDevice(). For a large
    amount of data (e.g. an image), use setBodyDevice(), which will not copy
    the data internally.

    \sa setBodyDevice()
*/
void QHttpPart::setBody(const QByteArray &body)
{
   d->setBody(body);
}

/*!
  Sets the device to read the content from to \a device. For large amounts of data
  this method should be preferred over setBody(),
  because the content is not copied when using this method, but read
  directly from the device.
  \a device must be open and readable. QHttpPart does not take ownership
  of \a device, i.e. the device must be closed and destroyed if necessary.
  if \a device is sequential (e.g. sockets, but not files),
  QNetworkAccessManager::post() should be called after \a device has
  emitted finished().
  For unsetting the device and using data set via setBody(), use
  "setBodyDevice(0)".

  \sa setBody(), QNetworkAccessManager::post()
  */
void QHttpPart::setBodyDevice(QIODevice *device)
{
   d->setBodyDevice(device);
}



/*!
    \class QHttpMultiPart
    \brief The QHttpMultiPart class resembles a MIME multipart message to be sent over HTTP.
    \since 4.8

    \ingroup network
    \inmodule QtNetwork

    The QHttpMultiPart resembles a MIME multipart message, as described in RFC 2046,
    which is to be sent over HTTP.
    A multipart message consists of an arbitrary number of body parts (see QHttpPart),
    which are separated by a unique boundary. The boundary of the QHttpMultiPart is
    constructed with the string "boundary_.oOo._" followed by random characters,
    and provides enough uniqueness to make sure it does not occur inside the parts itself.
    If desired, the boundary can still be set via setBoundary().

    As an example, consider the following code snippet, which constructs a multipart
    message containing a text part followed by an image part:

    \snippet doc/src/snippets/code/src_network_access_qhttpmultipart.cpp 0

    \sa QHttpPart, QNetworkAccessManager::post()
*/

/*!
    \enum QHttpMultiPart::ContentType

    List of known content types for a multipart subtype as described
    in RFC 2046 and others.

    \value MixedType    corresponds to the "multipart/mixed" subtype,
    meaning the body parts are independent of each other, as described
    in RFC 2046.

    \value RelatedType  corresponds to the "multipart/related" subtype,
    meaning the body parts are related to each other, as described in RFC 2387.

    \value FormDataType       corresponds to the "multipart/form-data"
    subtype, meaning the body parts contain form elements, as described in RFC 2388.

    \value AlternativeType   corresponds to the "multipart/alternative"
    subtype, meaning the body parts are alternative representations of
    the same information, as described in RFC 2046.

    \sa setContentType()
*/

/*!
    Constructs a QHttpMultiPart with content type MixedType and sets
    \a parent as the parent object.

    \sa QHttpMultiPart::ContentType
*/
QHttpMultiPart::QHttpMultiPart(QObject *parent)
   : QObject(parent), d_ptr(new QHttpMultiPartPrivate)
{
   Q_D(QHttpMultiPart);
   d->contentType = MixedType;
}

/*!
    Constructs a QHttpMultiPart with content type \a contentType and
    sets parent as the parent object.

    \sa QHttpMultiPart::ContentType
*/
QHttpMultiPart::QHttpMultiPart(QHttpMultiPart::ContentType contentType, QObject *parent)
   : QObject(parent), d_ptr(new QHttpMultiPartPrivate)
{
   Q_D(QHttpMultiPart);
   d->contentType = contentType;
}

/*!
    Destroys the multipart.
*/
QHttpMultiPart::~QHttpMultiPart()
{
}

/*!
    Appends \a httpPart to this multipart.
*/
void QHttpMultiPart::append(const QHttpPart &httpPart)
{
   d_func()->parts.append(httpPart);
}

/*!
    Sets the content type to \a contentType. The content type will be used
    in the HTTP header section when sending the multipart message via
    QNetworkAccessManager::post().
    In case you want to use a multipart subtype not contained in
    QHttpMultiPart::ContentType,
    you can add the "Content-Type" header field to the QNetworkRequest
    by hand, and then use this request together with the multipart
    message for posting.

    \sa QHttpMultiPart::ContentType, QNetworkAccessManager::post()
*/
void QHttpMultiPart::setContentType(QHttpMultiPart::ContentType contentType)
{
   d_func()->contentType = contentType;
}

/*!
    returns the boundary.

    \sa setBoundary()
*/
QByteArray QHttpMultiPart::boundary() const
{
   return d_func()->boundary;
}

/*!
    Sets the boundary to \a boundary.

    Usually, you do not need to generate a boundary yourself; upon construction
    the boundary is initiated with the string "boundary_.oOo._" followed by random
    characters, and provides enough uniqueness to make sure it does not occur
    inside the parts itself.

    \sa boundary()
*/
void QHttpMultiPart::setBoundary(const QByteArray &boundary)
{
   d_func()->boundary = boundary;
}



// ------------------------------------------------------------------
// ----------- implementations of private classes: ------------------
// ------------------------------------------------------------------



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
   if (bodyDevice)
      if (!bodyDevice->reset()) {
         ret = false;
      }
   readPointer = 0;
   return ret;
}
void QHttpPartPrivate::checkHeaderCreated() const
{
   if (!headerCreated) {
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

Q_GLOBAL_STATIC(QThreadStorage<bool *>, seedCreatedStorage);

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
   Q_UNUSED(data);
   Q_UNUSED(maxSize);
   return -1;
}


QT_END_NAMESPACE
