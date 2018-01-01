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

#ifndef ADPREADER_H
#define ADPREADER_H

#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtXml/QXmlStreamReader>

QT_BEGIN_NAMESPACE

struct ContentItem {
    ContentItem(const QString &t, const QString &r, int d)
       : title(t), reference(r), depth(d) {}
    QString title;
    QString reference;
    int depth;
};

struct KeywordItem {
    KeywordItem(const QString &k, const QString &r)
       : keyword(k), reference(r) {}
    QString keyword;
    QString reference;
};

class AdpReader : public QXmlStreamReader
{
public:
    void readData(const QByteArray &contents);
    QList<ContentItem> contents() const;
    QList<KeywordItem> keywords() const;
    QSet<QString> files() const;

    QMap<QString, QString> properties() const;

private:
    void readProject();
    void readProfile();
    void readDCF();
    void addFile(const QString &file);

    QMap<QString, QString> m_properties;
    QList<ContentItem> m_contents;
    QList<KeywordItem> m_keywords;
    QSet<QString> m_files;
};

QT_END_NAMESPACE

#endif
