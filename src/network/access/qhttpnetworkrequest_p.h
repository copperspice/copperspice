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

#ifndef QHTTPNETWORKREQUEST_P_H
#define QHTTPNETWORKREQUEST_P_H



#include <qhttpnetworkheader_p.h>


class QNonContiguousByteDevice;
class QHttpNetworkRequestPrivate;

class QHttpNetworkRequest: public QHttpNetworkHeader
{
 public:
   enum Operation {
      Options,
      Get,
      Head,
      Post,
      Put,
      Delete,
      Trace,
      Connect,
      Custom
   };

   enum Priority {
      HighPriority,
      NormalPriority,
      LowPriority
   };

   explicit QHttpNetworkRequest(const QUrl &url = QUrl(), Operation operation = Get, Priority priority = NormalPriority);
   QHttpNetworkRequest(const QHttpNetworkRequest &other);
   virtual ~QHttpNetworkRequest();
   QHttpNetworkRequest &operator=(const QHttpNetworkRequest &other);
   bool operator==(const QHttpNetworkRequest &other) const;

   QUrl url() const override;
   void setUrl(const QUrl &url) override;

   int majorVersion() const override;
   int minorVersion() const override;

   qint64 contentLength() const override;
   void setContentLength(qint64 length) override;

   QList<QPair<QByteArray, QByteArray> > header() const override;
   QByteArray headerField(const QByteArray &name, const QByteArray &defaultValue = QByteArray()) const override;
   void setHeaderField(const QByteArray &name, const QByteArray &data) override;

   Operation operation() const;
   void setOperation(Operation operation);

   QByteArray customVerb() const;
   void setCustomVerb(const QByteArray &customOperation);

   Priority priority() const;
   void setPriority(Priority priority);

   bool isPipeliningAllowed() const;
   void setPipeliningAllowed(bool b);

   bool isSPDYAllowed() const;
   void setSPDYAllowed(bool b);
   bool withCredentials() const;
   void setWithCredentials(bool b);

   bool isSsl() const;
   void setSsl(bool);

    bool isPreConnect() const;
    void setPreConnect(bool preConnect);
    bool isFollowRedirects() const;
    void setFollowRedirects(bool followRedirect);
    int redirectCount() const;
    void setRedirectCount(int count);
   void setUploadByteDevice(QNonContiguousByteDevice *bd);
   QNonContiguousByteDevice *uploadByteDevice() const;

   QByteArray methodName() const;
   QByteArray uri(bool throughProxy) const;
 private:
   QSharedDataPointer<QHttpNetworkRequestPrivate> d;
   friend class QHttpNetworkRequestPrivate;
   friend class QHttpNetworkConnectionPrivate;
   friend class QHttpNetworkConnectionChannel;
   friend class QHttpProtocolHandler;
   friend class QSpdyProtocolHandler;
};

class QHttpNetworkRequestPrivate : public QHttpNetworkHeaderPrivate
{
 public:
   QHttpNetworkRequestPrivate(QHttpNetworkRequest::Operation op,
                              QHttpNetworkRequest::Priority pri, const QUrl &newUrl = QUrl());

   QHttpNetworkRequestPrivate(const QHttpNetworkRequestPrivate &other);
   ~QHttpNetworkRequestPrivate();

   bool operator==(const QHttpNetworkRequestPrivate &other) const;

   static QByteArray header(const QHttpNetworkRequest &request, bool throughProxy);

   QHttpNetworkRequest::Operation operation;
   QByteArray customVerb;
   QHttpNetworkRequest::Priority priority;
   mutable QNonContiguousByteDevice *uploadByteDevice;
   bool autoDecompress;
   bool pipeliningAllowed;
   bool spdyAllowed;
   bool withCredentials;
   bool ssl;
   bool preConnect;
   bool followRedirect;
   int redirectCount;
};






#endif
