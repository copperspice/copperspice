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

#include "qtokenizer_p.h"
#include "qclucene_global_p.h"

#include <CLucene.h>
#include <CLucene/analysis/AnalysisHeader.h>

QT_BEGIN_NAMESPACE

QCLuceneTokenizer::QCLuceneTokenizer()
    : QCLuceneTokenStream()
{
    // nothing todo
}

QCLuceneTokenizer::QCLuceneTokenizer(const QCLuceneReader &reader)
    : QCLuceneTokenStream()
    , reader(reader)
{
    // nothing todo
}

QCLuceneTokenizer::~QCLuceneTokenizer()
{
    close();
}

void QCLuceneTokenizer::close()
{
    d->tokenStream->close();
}

bool QCLuceneTokenizer::next(QCLuceneToken &token)
{
    return d->tokenStream->next(token.d->token);
}


QCLuceneStandardTokenizer::QCLuceneStandardTokenizer(const QCLuceneReader &reader)
    : QCLuceneTokenizer(reader)
{
    d->tokenStream = 
        new lucene::analysis::standard::StandardTokenizer(reader.d->reader);
}

QCLuceneStandardTokenizer::~QCLuceneStandardTokenizer()
{
    // nothing todo
}

bool QCLuceneStandardTokenizer::readApostrophe(const QString &string, 
                                               QCLuceneToken &token)
{
    lucene::analysis::standard::StandardTokenizer *stdTokenizer = 
        static_cast<lucene::analysis::standard::StandardTokenizer*> (d->tokenStream);

    if (stdTokenizer == 0)
        return false;

    TCHAR* value = QStringToTChar(string);
    lucene::util::StringBuffer buffer(value);
    bool retValue = stdTokenizer->ReadApostrophe(&buffer, token.d->token);
    delete [] value;
    
    return retValue;
}

bool QCLuceneStandardTokenizer::readAt(const QString &string, QCLuceneToken &token)
{
    lucene::analysis::standard::StandardTokenizer *stdTokenizer = 
        static_cast<lucene::analysis::standard::StandardTokenizer*> (d->tokenStream);

    if (stdTokenizer == 0)
        return false;

    TCHAR* value = QStringToTChar(string);
    lucene::util::StringBuffer buffer(value);
    bool retValue = stdTokenizer->ReadAt(&buffer, token.d->token);
    delete [] value;
    
    return retValue;
}

bool QCLuceneStandardTokenizer::readCompany(const QString &string, 
                                            QCLuceneToken &token)
{
    lucene::analysis::standard::StandardTokenizer *stdTokenizer = 
        static_cast<lucene::analysis::standard::StandardTokenizer*> (d->tokenStream);

    if (stdTokenizer == 0)
        return false;

    TCHAR* value = QStringToTChar(string);
    lucene::util::StringBuffer buffer(value);
    bool retValue = stdTokenizer->ReadCompany(&buffer, token.d->token);
    delete [] value;
    
    return retValue;
}

QT_END_NAMESPACE
