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

#ifndef QTSIMPLEXML_H
#define QTSIMPLEXML_H

#include <QString>
#include <QMultiMap>
#include <QMap>

QT_BEGIN_NAMESPACE

class QDomDocument;
class QDomElement;
class QDomNode;
class QIODevice;

class QtSimpleXml
{
public:
    QtSimpleXml(const QString &name = QString());

    QString name() const;
    QString text() const;
    int numChildren() const;
    bool isValid() const;

    const QtSimpleXml &operator [](int index) const;
    QtSimpleXml &operator [](int index);
    QtSimpleXml &operator [](const QString &key);
    QtSimpleXml &operator =(const QString &text);

    void setAttribute(const QString &key, const QString &value);
    QString attribute(const QString &key);

    bool setContent(const QString &content);
    bool setContent(QIODevice *device);
    QString errorString() const;

    QDomDocument toDomDocument() const;
    QDomElement toDomElement(QDomDocument *doc) const;
private:
    void parse(QDomNode node);

    QtSimpleXml *parent;

    QMultiMap<QString, QtSimpleXml *> children;
    QMap<QString, QString> attr;

    QString s;
    QString n;
    bool valid;

    QString errorStr;
};

QT_END_NAMESPACE

#endif
