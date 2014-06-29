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

#include "qterm_p.h"
#include "qclucene_global_p.h"

#include <CLucene.h>
#include <CLucene/index/IndexReader.h>

QT_BEGIN_NAMESPACE

QCLuceneTermPrivate::QCLuceneTermPrivate()
    : QSharedData()
{
    term = 0;
    deleteCLuceneTerm = true;
}

QCLuceneTermPrivate::QCLuceneTermPrivate(const QCLuceneTermPrivate &other)
    : QSharedData()
{
    term = _CL_POINTER(other.term);
    deleteCLuceneTerm = other.deleteCLuceneTerm;
}

QCLuceneTermPrivate::~QCLuceneTermPrivate()
{
    if (deleteCLuceneTerm)
        _CLDECDELETE(term);
}


QCLuceneTerm::QCLuceneTerm()
    : d(new QCLuceneTermPrivate())
{
    d->term = new lucene::index::Term();
}

QCLuceneTerm::QCLuceneTerm(const QString &field, const QString &text)
    : d(new QCLuceneTermPrivate())
{
    TCHAR *fieldName = QStringToTChar(field);
    TCHAR *termText = QStringToTChar(text);

    d->term = new lucene::index::Term(fieldName, termText);

    delete [] fieldName;
    delete [] termText;
}

QCLuceneTerm::QCLuceneTerm(const QCLuceneTerm &fieldTerm, const QString &text)
    : d(new QCLuceneTermPrivate())
{
    TCHAR *termText = QStringToTChar(text);
    d->term = new lucene::index::Term(fieldTerm.d->term, termText);
    delete [] termText;
}

QCLuceneTerm::~QCLuceneTerm()
{
    // nothing todo
}

QString QCLuceneTerm::field() const
{
    return TCharToQString(d->term->field());
}

QString QCLuceneTerm::text() const
{
    return TCharToQString(d->term->text());
}

void QCLuceneTerm::set(const QString &field, const QString &text)
{
    set(field, text, true);
}

void QCLuceneTerm::set(const QCLuceneTerm &fieldTerm, const QString &text)
{
    set(fieldTerm.field(), text, false);
}

void QCLuceneTerm::set(const QString &field, const QString &text, bool internField)
{
    TCHAR *fieldName = QStringToTChar(field);
    TCHAR *termText = QStringToTChar(text);

    d->term->set(fieldName, termText, internField);

    delete [] fieldName;
    delete [] termText;
}

bool QCLuceneTerm::equals(const QCLuceneTerm &other) const
{
    return d->term->equals(other.d->term);
}

qint32 QCLuceneTerm::compareTo(const QCLuceneTerm &other) const
{
    return quint32(d->term->compareTo(other.d->term));
}

QString QCLuceneTerm::toString() const
{
    return TCharToQString(d->term->toString());
}

quint32 QCLuceneTerm::hashCode() const
{
    return quint32(d->term->hashCode());
}

quint32 QCLuceneTerm::textLength() const
{
    return quint32(d->term->textLength());
}

QT_END_NAMESPACE
