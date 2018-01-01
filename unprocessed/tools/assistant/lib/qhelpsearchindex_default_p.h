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

#ifndef QHELPSEARCHINDEXDEFAULT_H
#define QHELPSEARCHINDEXDEFAULT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QDataStream>

QT_BEGIN_NAMESPACE

namespace QtHelpInternal {

struct Document {
    Document(qint16 d, qint16 f)
        : docNumber(d), frequency(f) {}

    Document()
        : docNumber(-1), frequency(0) {}

    bool operator==(const Document &doc) const {
        return docNumber == doc.docNumber;
    }
    bool operator<(const Document &doc) const {
        return frequency > doc.frequency;
    }
    bool operator<=(const Document &doc) const {
        return frequency >= doc.frequency;
    }
    bool operator>(const Document &doc) const {
        return frequency < doc.frequency;
    }

    qint16 docNumber;
    qint16 frequency;
};

struct DocumentInfo : public Document {
    DocumentInfo()
        : Document(-1, 0), documentTitle(QString()), documentUrl(QString()) {}

    DocumentInfo(qint16 d, qint16 f, const QString &title, const QString &url)
        : Document(d, f), documentTitle(title), documentUrl(url) {}

    DocumentInfo(const Document &document, const QString &title, const QString &url)
        : Document(document.docNumber, document.frequency), documentTitle(title), documentUrl(url) {}

    QString documentTitle;
    QString documentUrl;
};

struct Entry {
    Entry(qint16 d) { documents.append(Document(d, 1)); }
    Entry(QVector<Document> l) : documents(l) {}

    QVector<Document> documents;
};

struct PosEntry {
    PosEntry(int p) { positions.append(p); }
    QList<uint> positions;
};

struct Term {
    Term() : frequency(-1) {}
    Term(const QString &t, int f, QVector<Document> l) : term(t), frequency(f), documents(l) {}
    QString term;
    int frequency;
    QVector<Document>documents;
    bool operator<(const Term &i2) const { return frequency < i2.frequency; }
};

struct TermInfo {
    TermInfo() : frequency(-1) {}
    TermInfo(const QString &t, int f, QVector<DocumentInfo> l)
        : term(t), frequency(f), documents(l) {}

    bool operator<(const TermInfo &i2) const { return frequency < i2.frequency; }

    QString term;
    int frequency;
    QVector<DocumentInfo>documents;
};

} // namespace QtHelpInternal

using QtHelpInternal::Document;
using QtHelpInternal::DocumentInfo;
using QtHelpInternal::Entry;
using QtHelpInternal::PosEntry;
using QtHelpInternal::Term;
using QtHelpInternal::TermInfo;

QDataStream &operator>>(QDataStream &s, Document &l);
QDataStream &operator<<(QDataStream &s, const Document &l);

QT_END_NAMESPACE

#endif  // QHELPSEARCHINDEXDEFAULT_H
