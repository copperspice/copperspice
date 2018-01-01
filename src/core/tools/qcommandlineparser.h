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

/**********************************************************************
**
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
***********************************************************************/

#ifndef QCOMMANDLINEPARSER_H
#define QCOMMANDLINEPARSER_H

#include <QtCore/qstringlist.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qcommandlineoption.h>

QT_BEGIN_NAMESPACE

class QCommandLineParserPrivate;
class QCoreApplication;

class Q_CORE_EXPORT QCommandLineParser
{
   Q_DECLARE_TR_FUNCTIONS(QCommandLineParser)

 public:
   QCommandLineParser();
   ~QCommandLineParser();

   enum SingleDashWordOptionMode {
      ParseAsCompactedShortOptions,
      ParseAsLongOptions
   };
   void setSingleDashWordOptionMode(SingleDashWordOptionMode parsingMode);

   bool addOption(const QCommandLineOption &commandLineOption);

   QCommandLineOption addVersionOption();
   QCommandLineOption addHelpOption();
   void setApplicationDescription(const QString &description);
   QString applicationDescription() const;
   void addPositionalArgument(const QString &name, const QString &description, const QString &syntax = QString());
   void clearPositionalArguments();

   void process(const QStringList &arguments);
   void process(const QCoreApplication &app);

   bool parse(const QStringList &arguments);
   QString errorText() const;

   bool isSet(const QString &name) const;
   QString value(const QString &name) const;
   QStringList values(const QString &name) const;

   bool isSet(const QCommandLineOption &option) const;
   QString value(const QCommandLineOption &option) const;
   QStringList values(const QCommandLineOption &option) const;

   QStringList positionalArguments() const;
   QStringList optionNames() const;
   QStringList unknownOptionNames() const;

   void showHelp(int exitCode = 0);
   QString helpText() const;

 private:
   Q_DISABLE_COPY(QCommandLineParser)

   QCommandLineParserPrivate *const d;
};

QT_END_NAMESPACE

#endif // QCOMMANDLINEPARSER_H
