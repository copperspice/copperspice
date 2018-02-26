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

#ifndef QHTTPHEADER_P_H
#define QHTTPHEADER_P_H

#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qpair.h>
#include <qscopedpointer.h>

class QHttpHeaderPrivate;
class QHttpRequestHeaderPrivate;
class QHttpResponseHeaderPrivate;

class Q_NETWORK_EXPORT QHttpHeader
{
public:
    QHttpHeader();
    QHttpHeader(const QHttpHeader &other);
    QHttpHeader(const QString &str);
    virtual ~QHttpHeader();

    QHttpHeader &operator=(const QHttpHeader &other);

    void setValue(const QString &key, const QString &value);
    void setValues(const QList<QPair<QString, QString> > &values);
    void addValue(const QString &key, const QString &value);
    QList<QPair<QString, QString> > values() const;
    bool hasKey(const QString &key) const;
    QStringList keys() const;
    QString value(const QString &key) const;
    QStringList allValues(const QString &key) const;
    void removeValue(const QString &key);
    void removeAllValues(const QString &key);

    bool hasContentLength() const;
    qint64 contentLength() const;
    void setContentLength(qint64 len);

    bool hasContentType() const;
    QString contentType() const;
    void setContentType(const QString &type);

    virtual QString toString() const;
    bool isValid() const;

    virtual int majorVersion() const = 0;
    virtual int minorVersion() const = 0;

protected:
    virtual bool parseLine(const QString &line, int number);
    bool parse(const QString &str);
    void setValid(bool);

    QHttpHeader(QHttpHeaderPrivate &dd, const QString &str = QString());
    QHttpHeader(QHttpHeaderPrivate &dd, const QHttpHeader &header);

    QScopedPointer<QHttpHeaderPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(QHttpHeader)
};

class Q_NETWORK_EXPORT QHttpRequestHeader : public QHttpHeader
{
 public:
   QHttpRequestHeader();
   QHttpRequestHeader(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);
   QHttpRequestHeader(const QHttpRequestHeader &other);
   QHttpRequestHeader(const QString &str);

   QHttpRequestHeader &operator=(const QHttpRequestHeader &other);

   void setRequest(const QString &method, const QString &path, int majorVer = 1, int minorVer = 1);

   QString method() const;
   QString path() const;

   int majorVersion() const override;
   int minorVersion() const override;

   QString toString() const override;

 protected:
   bool parseLine(const QString &line, int number) override;

 private:
   Q_DECLARE_PRIVATE(QHttpRequestHeader)
};

class Q_NETWORK_EXPORT QHttpResponseHeader : public QHttpHeader
{
public:
    QHttpResponseHeader();
    QHttpResponseHeader(const QHttpResponseHeader &other);
    QHttpResponseHeader(const QString &str);
    QHttpResponseHeader(int code, const QString &text = QString(), int majorVer = 1, int minorVer = 1);

    QHttpResponseHeader &operator=(const QHttpResponseHeader &other);

    void setStatusLine(int code, const QString &text = QString(), int majorVer = 1, int minorVer = 1);

    int statusCode() const;
    QString reasonPhrase() const;

    int majorVersion() const;
    int minorVersion() const;

    QString toString() const;

protected:
    bool parseLine(const QString &line, int number);

private:
    Q_DECLARE_PRIVATE(QHttpResponseHeader)
    friend class QHttpPrivate;
};

#endif
