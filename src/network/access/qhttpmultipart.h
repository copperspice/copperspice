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

#ifndef QHTTPMULTIPART_H
#define QHTTPMULTIPART_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QByteArray>
#include <QtNetwork/QNetworkRequest>
#include <QScopedPointer>


class QHttpPartPrivate;
class QHttpMultiPart;
class QHttpMultiPartPrivate;

class Q_NETWORK_EXPORT QHttpPart
{
 public:
   QHttpPart();
   QHttpPart(const QHttpPart &other);
   ~QHttpPart();

   QHttpPart &operator=(QHttpPart &&other)  {
      swap(other);
      return *this;
   }

   QHttpPart &operator=(const QHttpPart &other);
   bool operator==(const QHttpPart &other) const;

   inline bool operator!=(const QHttpPart &other) const {
      return !operator==(other);
   }

   void setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value);
   void setRawHeader(const QByteArray &headerName, const QByteArray &headerValue);

   void setBody(const QByteArray &body);
   void setBodyDevice(QIODevice *device);

   void swap(QHttpPart &other)  {
      qSwap(d, other.d);
   }

 private:
   QSharedDataPointer<QHttpPartPrivate> d;

   friend class QHttpMultiPartIODevice;
};

class Q_NETWORK_EXPORT QHttpMultiPart : public QObject
{
   NET_CS_OBJECT(QHttpMultiPart)

 public:

   enum ContentType {
      MixedType,
      RelatedType,
      FormDataType,
      AlternativeType
   };

   explicit QHttpMultiPart(QObject *parent = nullptr);
   explicit QHttpMultiPart(ContentType contentType, QObject *parent = nullptr);
   ~QHttpMultiPart();

   void append(const QHttpPart &httpPart);

   void setContentType(ContentType contentType);

   QByteArray boundary() const;
   void setBoundary(const QByteArray &boundary);

 private:
   Q_DECLARE_PRIVATE(QHttpMultiPart)
   Q_DISABLE_COPY(QHttpMultiPart)

   friend class QNetworkAccessManager;
   friend class QNetworkAccessManagerPrivate;

 protected:
   QScopedPointer<QHttpMultiPartPrivate> d_ptr;

};


#endif // QHTTPMULTIPART_H
