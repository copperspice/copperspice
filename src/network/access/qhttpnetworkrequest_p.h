/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QHTTPNETWORKREQUEST_P_H
#define QHTTPNETWORKREQUEST_P_H

#ifndef QT_NO_HTTP

#include <qhttpnetworkheader_p.h>

QT_BEGIN_NAMESPACE

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

    QHttpNetworkRequest(const QUrl &url = QUrl(), Operation operation = Get, Priority priority = NormalPriority);
    QHttpNetworkRequest(const QHttpNetworkRequest &other);
    virtual ~QHttpNetworkRequest();
    QHttpNetworkRequest &operator=(const QHttpNetworkRequest &other);
    bool operator==(const QHttpNetworkRequest &other) const;

    QUrl url() const;
    void setUrl(const QUrl &url);

    int majorVersion() const;
    int minorVersion() const;

    qint64 contentLength() const;
    void setContentLength(qint64 length);

    QList<QPair<QByteArray, QByteArray> > header() const;
    QByteArray headerField(const QByteArray &name, const QByteArray &defaultValue = QByteArray()) const;
    void setHeaderField(const QByteArray &name, const QByteArray &data);

    Operation operation() const;
    void setOperation(Operation operation);

    QByteArray customVerb() const;
    void setCustomVerb(const QByteArray &customOperation);

    Priority priority() const;
    void setPriority(Priority priority);

    bool isPipeliningAllowed() const;
    void setPipeliningAllowed(bool b);

    bool withCredentials() const;
    void setWithCredentials(bool b);

    bool isSsl() const;
    void setSsl(bool);

    void setUploadByteDevice(QNonContiguousByteDevice *bd);
    QNonContiguousByteDevice* uploadByteDevice() const;

private:
    QSharedDataPointer<QHttpNetworkRequestPrivate> d;
    friend class QHttpNetworkRequestPrivate;
    friend class QHttpNetworkConnectionPrivate;
    friend class QHttpNetworkConnectionChannel;
};

class QHttpNetworkRequestPrivate : public QHttpNetworkHeaderPrivate
{
public:
    QHttpNetworkRequestPrivate(QHttpNetworkRequest::Operation op,
        QHttpNetworkRequest::Priority pri, const QUrl &newUrl = QUrl());

    QHttpNetworkRequestPrivate(const QHttpNetworkRequestPrivate &other);
    ~QHttpNetworkRequestPrivate();

    bool operator==(const QHttpNetworkRequestPrivate &other) const;
    QByteArray methodName() const;
    QByteArray uri(bool throughProxy) const;

    static QByteArray header(const QHttpNetworkRequest &request, bool throughProxy);

    QHttpNetworkRequest::Operation operation;
    QByteArray customVerb;
    QHttpNetworkRequest::Priority priority;
    mutable QNonContiguousByteDevice* uploadByteDevice;
    bool autoDecompress;
    bool pipeliningAllowed;
    bool withCredentials;
    bool ssl;
};

QT_END_NAMESPACE

#endif // QT_NO_HTTP


#endif // QHTTPNETWORKREQUEST_H
