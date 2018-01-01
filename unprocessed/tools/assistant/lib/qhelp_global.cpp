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

#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QRegExp>
#include <QtCore/QMutexLocker>
#include <QtGui/QTextDocument>

#include "qhelp_global.h"

QString QHelpGlobal::uniquifyConnectionName(const QString &name, void *pointer)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    static QHash<QString,quint16> idHash;

    return QString::fromLatin1("%1-%2-%3").
        arg(name).arg(quintptr(pointer)).arg(++idHash[name]);
}

QString QHelpGlobal::documentTitle(const QString &content)
{
    QString title = QCoreApplication::translate("QHelp", "Untitled");
    if (!content.isEmpty()) {
        int start = content.indexOf(QLatin1String("<title>"), 0, Qt::CaseInsensitive) + 7;
        int end = content.indexOf(QLatin1String("</title>"), 0, Qt::CaseInsensitive);
        if ((end - start) > 0) {
            title = content.mid(start, end - start);
            if (Qt::mightBeRichText(title) || title.contains(QLatin1Char('&'))) {
                QTextDocument doc;
                doc.setHtml(title);
                title = doc.toPlainText();
            }
        }
    }
    return title;
}

QString QHelpGlobal::codecFromData(const QByteArray &data)
{
    QString codec = codecFromXmlData(data);
    if (codec.isEmpty())
        codec = codecFromHtmlData(data);
    return codec.isEmpty() ? QLatin1String("utf-8") : codec;
}

QString QHelpGlobal::codecFromHtmlData(const QByteArray &data)
{
    QString head = QString::fromUtf8(data.constData(), qMin(1000, data.size()));
    int start = head.indexOf(QLatin1String("<meta"), 0, Qt::CaseInsensitive);
    if (start > 0) {
        QRegExp r(QLatin1String("charset=([^\"\\s]+)"));
        while (start != -1) {
            const int end = head.indexOf(QLatin1Char('>'), start) + 1;
            if (end <= start)
                break;
            const QString &meta = head.mid(start, end - start).toLower();
            if (r.indexIn(meta) != -1)
                return r.cap(1);
            start = head.indexOf(QLatin1String("<meta"), end,
                                    Qt::CaseInsensitive);
        }
    }
    return QString();
}

QString QHelpGlobal::codecFromXmlData(const QByteArray &data)
{
    QString head = QString::fromUtf8(data.constData(), qMin(1000, data.size()));
    const QRegExp encodingExp(QLatin1String("^\\s*<\\?xml version="
        "\"\\d\\.\\d\" encoding=\"([^\"]+)\"\\?>.*"));
    return encodingExp.exactMatch(head) ? encodingExp.cap(1) : QString();
}
