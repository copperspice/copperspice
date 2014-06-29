/****************************************************************************
**
** Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team.
** All rights reserved.
**
** Portion Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#include "qreader_p.h"
#include "qclucene_global_p.h"

#include <CLucene.h>
#include <CLucene/util/Reader.h>

QT_BEGIN_NAMESPACE

QCLuceneReaderPrivate::QCLuceneReaderPrivate()
    : QSharedData()
{
    reader = 0;
    deleteCLuceneReader = true;
}

QCLuceneReaderPrivate::QCLuceneReaderPrivate(const QCLuceneReaderPrivate &other)
    : QSharedData()
{
    reader = _CL_POINTER(other.reader);
    deleteCLuceneReader = other.deleteCLuceneReader;
}

QCLuceneReaderPrivate::~QCLuceneReaderPrivate()
{
    if (deleteCLuceneReader)
        _CLDECDELETE(reader);
}

QCLuceneReader::QCLuceneReader()
    : d(new QCLuceneReaderPrivate())
{
    // nothing todo
}

QCLuceneReader::~QCLuceneReader()
{
    // nothing todo
}


QCLuceneStringReader::QCLuceneStringReader(const QString &value)
    : QCLuceneReader()
    , string(QStringToTChar(value))
{
    d->reader = new lucene::util::StringReader(string);
}

QCLuceneStringReader::QCLuceneStringReader(const QString &value, qint32 length)
    : QCLuceneReader()
    , string(QStringToTChar(value))
{
    d->reader = new lucene::util::StringReader(string, int32_t(length));
}

QCLuceneStringReader::QCLuceneStringReader(const QString &value, qint32 length,
                                           bool copyData)
    : QCLuceneReader()
    , string(QStringToTChar(value))
{
    d->reader = new lucene::util::StringReader(string, int32_t(length), copyData);
}

QCLuceneStringReader::~QCLuceneStringReader()
{
    delete [] string;
}


QCLuceneFileReader::QCLuceneFileReader(const QString &path, const QString &encoding,
                                       qint32 cacheLength, qint32 cacheBuffer)
    : QCLuceneReader()
{
    const QByteArray tmpPath = path.toLocal8Bit();
    const QByteArray tmpEncoding = encoding.toAscii();
    d->reader = new lucene::util::FileReader(tmpPath.constData(),
        tmpEncoding.constData(), int32_t(cacheLength), int32_t(cacheBuffer));
}

QCLuceneFileReader::~QCLuceneFileReader()
{
    // nothing todo
}

QT_END_NAMESPACE
