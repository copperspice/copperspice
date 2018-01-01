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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QApplicationArgumentParser_H
#define QApplicationArgumentParser_H

#include <QtCore/QVariant> /* Needed, because we can't forward declare QVariantList. */

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QApplicationArgument;
class QApplicationArgumentParserPrivate;
class QStringList;
template<typename A, typename B> struct QPair;
template<typename T> class QList;
template<typename Value> class QList;

class QApplicationArgumentParser
{
public:
    enum ExitCode
    {
        Success = 0,
        ParseError = 1
    };

    QApplicationArgumentParser(int argc, char **argv);
    QApplicationArgumentParser(const QStringList &input);
    virtual ~QApplicationArgumentParser();
    void addArgument(const QApplicationArgument &argument);
    void setDeclaredArguments(const QList<QApplicationArgument> &arguments);
    QList<QApplicationArgument> declaredArguments() const;

    int count(const QApplicationArgument &argument) const;
    bool has(const QApplicationArgument &argument) const;

    virtual bool parse();
    ExitCode exitCode() const;
    QVariant value(const QApplicationArgument &argument) const;
    QVariantList values(const QApplicationArgument &argument) const;
    void setApplicationDescription(const QString &description);
    void setApplicationVersion(const QString &version);
    virtual void message(const QString &message) const;

protected:
    void setExitCode(ExitCode code);
    void setUsedArguments(const QList<QPair<QApplicationArgument, QVariant> > &arguments);
    QList<QPair<QApplicationArgument, QVariant> > usedArguments() const;
    QStringList input() const;
    virtual QVariant convertToValue(const QApplicationArgument &argument,
                                    const QString &value) const;
    virtual QString typeToName(const QApplicationArgument &argument) const;
    virtual QVariant defaultValue(const QApplicationArgument &argument) const;

private:
    friend class QApplicationArgumentParserPrivate;
    QApplicationArgumentParserPrivate *d;
    Q_DISABLE_COPY(QApplicationArgumentParser)
};

QT_END_NAMESPACE
QT_END_HEADER
#endif
