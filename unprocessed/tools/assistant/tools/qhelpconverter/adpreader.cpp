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

#include <QRegExp>

#include "adpreader.h"

static bool versionIsAtLeast320(const QString &version)
{
    return QRegExp("\\d.\\d\\.\\d").exactMatch(version)
            && (version[0] > '3' || (version[0] == '3' && version[2] >= '2'));
}

QT_BEGIN_NAMESPACE

void AdpReader::readData(const QByteArray &contents)
{
    clear();
    m_contents.clear();
    m_keywords.clear();
    m_properties.clear();
    m_files.clear();
    addData(contents);
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name().toString().toLower() == QLatin1String("assistantconfig")
                && versionIsAtLeast320(attributes().value(QLatin1String("version")).toString())) {
                readProject();
            } else if (name().toString().toLower() == QLatin1String("dcf")) {
                QString ref = attributes().value(QLatin1String("ref")).toString();
                addFile(ref);
                m_contents.append(ContentItem(attributes().value(QLatin1String("title")).toString(),
                    ref, 0));
                readDCF();
            } else {
                raiseError();
            }
        }
    }
}

QList<ContentItem> AdpReader::contents() const
{
    return m_contents;
}

QList<KeywordItem> AdpReader::keywords() const
{
    return m_keywords;
}

QSet<QString> AdpReader::files() const
{
    return m_files;
}

QMap<QString, QString> AdpReader::properties() const
{
    return m_properties;
}

void AdpReader::readProject()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            QString s = name().toString().toLower();
            if (s == QLatin1String("profile")) {
                readProfile();
            } else if (s == QLatin1String("dcf")) {
                QString ref = attributes().value(QLatin1String("ref")).toString();
                addFile(ref);
                m_contents.append(ContentItem(attributes().value(QLatin1String("title")).toString(),
                    ref, 0));
                readDCF();
            } else {
                raiseError();
            }
        }
    }
}

void AdpReader::readProfile()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name().toString().toLower() == QLatin1String("property")) {
                QString prop = attributes().value(QLatin1String("name")).toString().toLower();
                m_properties[prop] = readElementText();
            } else {
                raiseError();
            }
        } else if (isEndElement()) {
            break;
        }
    }
}

void AdpReader::readDCF()
{
    int depth = 0;
    while (!atEnd()) {
        readNext();
        QString str = name().toString().toLower();
        if (isStartElement()) {
            if (str == QLatin1String("section")) {
                QString ref = attributes().value(QLatin1String("ref")).toString();
                addFile(ref);
                m_contents.append(ContentItem(attributes().value(QLatin1String("title")).toString(),
                    ref, ++depth));
            } else if (str == QLatin1String("keyword")) {
                QString ref = attributes().value(QLatin1String("ref")).toString();
                addFile(ref);
                m_keywords.append(KeywordItem(readElementText(), ref));
            } else {
                raiseError();
            }
        } else if (isEndElement()) {
            if (str == QLatin1String("section"))
                --depth;
            else if (str == QLatin1String("dcf"))
                break;
        }
    }
}

void AdpReader::addFile(const QString &file)
{
    QString s = file;
    if (s.startsWith(QLatin1String("./")))
        s = s.mid(2);
    int i = s.indexOf(QLatin1Char('#'));
    if (i > -1)
        s = s.left(i);
    if (!m_files.contains(s))
        m_files.insert(s);
}

QT_END_NAMESPACE
