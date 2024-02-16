/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

/*****************************************************
** Copyright (c) 2013 Laszlo Papp <lpapp@kde.org>
*****************************************************/

#ifndef QCOMMANDLINEPARSER_H
#define QCOMMANDLINEPARSER_H

#include <qcoreapplication.h>
#include <qcommandlineoption.h>
#include <qstringlist.h>
class QCommandLineParserPrivate;
class QCoreApplication;

class Q_CORE_EXPORT QCommandLineParser
{
   Q_DECLARE_TR_FUNCTIONS(QCommandLineParser)

 public:
   enum SingleDashWordOptionMode {
      ParseAsCompactedShortOptions,
      ParseAsLongOptions
   };

   QCommandLineParser();

   QCommandLineParser(const QCommandLineParser &) = delete;
   QCommandLineParser &operator=(const QCommandLineParser &) = delete;

   ~QCommandLineParser();

   void setSingleDashWordOptionMode(SingleDashWordOptionMode parsingMode);

   bool addOption(const QCommandLineOption &option);

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
   QString value(const QString &optionName) const;
   QStringList values(const QString &optionName) const;

   bool isSet(const QCommandLineOption &option) const;
   QString value(const QCommandLineOption &option) const;
   QStringList values(const QCommandLineOption &option) const;

   QStringList positionalArguments() const;
   QStringList optionNames() const;
   QStringList unknownOptionNames() const;

   void showVersion();
   void showHelp(int exitCode = 0);
   QString helpText() const;

 private:
   QCommandLineParserPrivate *const d;
};

#endif
