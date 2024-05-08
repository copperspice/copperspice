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
** Copyright (c) 2013 David Faure <faure@kde.org>
*****************************************************/

#include <qcommandlineparser.h>
#include <qcoreapplication.h>
#include <qhash.h>
#include <qstringparser.h>
#include <qvector.h>

#include <stdio.h>
#include <stdlib.h>

using NameHash_t = QHash<QString, int>;

class QCommandLineParserPrivate
{
 public:
   QCommandLineParserPrivate()
      : singleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions),
        builtinVersionOption(false), builtinHelpOption(false), needsParsing(true)
   { }

   bool parse(const QStringList &args);
   void checkParsed(const char *method);
   QStringList aliases(const QString &name) const;
   QString helpText() const;
   bool registerFoundOption(const QString &optionName);
   bool parseOptionValue(const QString &optionName, const QString &argument,
         QStringList::const_iterator *argumentIterator, QStringList::const_iterator argsEnd);

   QString errorText;

   QList<QCommandLineOption> commandLineOptionList;

   // hash mapping option names to their offsets in commandLineOptionList and optionArgumentList.
   NameHash_t nameHash;

   // option values found (only for options with a value)
   QHash<int, QStringList> optionValuesHash;

   QStringList optionNames;

   // arguments which did not belong to any option
   QStringList positionalArgumentList;

   QStringList unknownOptionNames;
   QString description;

   struct PositionalArgumentDefinition {
      QString name;
      QString description;
      QString syntax;
   };
   QVector<PositionalArgumentDefinition> positionalArgumentDefinitions;

   // parsing mode for "-abc"
   QCommandLineParser::SingleDashWordOptionMode singleDashWordOptionMode;

   // whether addVersionOption was called
   bool builtinVersionOption;

   // whether addHelpOption was called
   bool builtinHelpOption;

   bool needsParsing;
};

QStringList QCommandLineParserPrivate::aliases(const QString &optionName) const
{
   const NameHash_t::const_iterator it = nameHash.find(optionName);

   if (it == nameHash.end()) {
      qWarning("QCommandLineParser::aliases() Option was not defined, %s", csPrintable(optionName));
      return QStringList();
   }

   return commandLineOptionList.at(*it).names();
}

QCommandLineParser::QCommandLineParser()
   : d(new QCommandLineParserPrivate)
{
}

QCommandLineParser::~QCommandLineParser()
{
   delete d;
}

void QCommandLineParser::setSingleDashWordOptionMode(QCommandLineParser::SingleDashWordOptionMode singleDashWordOptionMode)
{
   d->singleDashWordOptionMode = singleDashWordOptionMode;
}

bool QCommandLineParser::addOption(const QCommandLineOption &option)
{
   QStringList list = option.names();

   if (! list.isEmpty()) {
      for (const QString &name : list) {
         if (d->nameHash.contains(name)) {
            return false;
         }
      }

      d->commandLineOptionList.append(option);

      const int offset = d->commandLineOptionList.size() - 1;

      for (const QString &name : list) {
         d->nameHash.insert(name, offset);
      }

      return true;
   }

   return false;
}

QCommandLineOption QCommandLineParser::addVersionOption()
{
   d->builtinVersionOption = true;

   QStringList list = { "v", "version" };
   QCommandLineOption opt(list, tr("Displays version information."));

   addOption(opt);

   return opt;
}

QCommandLineOption QCommandLineParser::addHelpOption()
{
   d->builtinHelpOption = true;

   QStringList list;

#ifdef Q_OS_WIN
   list.append("?");
#endif

   list.append("h");
   list.append("help");

   QCommandLineOption opt(list, tr("Displays this help."));

   addOption(opt);
   return opt;
}

void QCommandLineParser::setApplicationDescription(const QString &description)
{
   d->description = description;
}

QString QCommandLineParser::applicationDescription() const
{
   return d->description;
}

void QCommandLineParser::addPositionalArgument(const QString &name, const QString &description, const QString &syntax)
{
   QCommandLineParserPrivate::PositionalArgumentDefinition arg;
   arg.name = name;
   arg.description = description;
   arg.syntax = syntax.isEmpty() ? name : syntax;
   d->positionalArgumentDefinitions.append(arg);
}

void QCommandLineParser::clearPositionalArguments()
{
   d->positionalArgumentDefinitions.clear();
}

bool QCommandLineParser::parse(const QStringList &arguments)
{
   return d->parse(arguments);
}

QString QCommandLineParser::errorText() const
{
   if (! d->errorText.isEmpty()) {
      return d->errorText;
   }

   if (d->unknownOptionNames.count() == 1) {
      return tr("Unknown option '%1'.").formatArg(d->unknownOptionNames.first());
   }

   if (d->unknownOptionNames.count() > 1) {
      return tr("Unknown options: %1.").formatArg(d->unknownOptionNames.join(QString(", ")));
   }

   return QString();
}

void QCommandLineParser::process(const QStringList &arguments)
{
   if (! d->parse(arguments)) {
      fprintf(stderr, "%s\n", csPrintable(errorText()));
      ::exit(EXIT_FAILURE);
   }

   if (d->builtinVersionOption && isSet("version")) {
      showVersion();
   }

   if (d->builtinHelpOption && isSet("help")) {
      showHelp(EXIT_SUCCESS);
   }
}

void QCommandLineParser::process(const QCoreApplication &)
{
   // QCoreApplication::arguments() is static
   // user must pass the parameter to ensure the QCoreApplication has been constructed

   process(QCoreApplication::arguments());
}

void QCommandLineParserPrivate::checkParsed(const char *method)
{
   if (needsParsing) {
      qWarning("QCommandLineParser::checkParsed() Call process() or parse() before %s", method);
   }
}

bool QCommandLineParserPrivate::registerFoundOption(const QString &optionName)
{
   if (nameHash.contains(optionName)) {
      optionNames.append(optionName);
      return true;

   } else {
      unknownOptionNames.append(optionName);
      return false;
   }
}

bool QCommandLineParserPrivate::parseOptionValue(const QString &optionName, const QString &argument,
      QStringList::const_iterator *argumentIterator, QStringList::const_iterator argsEnd)
{
   const QChar assignChar('=');
   const NameHash_t::const_iterator nameHashIt = nameHash.constFind(optionName);

   if (nameHashIt != nameHash.constEnd()) {
      const int assignPos = argument.indexOf(assignChar);
      const NameHash_t::mapped_type optionOffset = *nameHashIt;
      const bool withValue = !commandLineOptionList.at(optionOffset).valueName().isEmpty();

      if (withValue) {
         if (assignPos == -1) {
            ++(*argumentIterator);

            if (*argumentIterator == argsEnd) {
               errorText = QCommandLineParser::tr("Missing value after '%1'.").formatArg(argument);
               return false;
            }

            optionValuesHash[optionOffset].append(*(*argumentIterator));
         } else {
            optionValuesHash[optionOffset].append(argument.mid(assignPos + 1));
         }
      } else {
         if (assignPos != -1) {
            errorText = QCommandLineParser::tr("Unexpected value after '%1'.").formatArg(argument.left(assignPos));
            return false;
         }
      }
   }

   return true;
}

bool QCommandLineParserPrivate::parse(const QStringList &args)
{
   needsParsing = false;
   bool error   = false;

   const QString doubleDashString(QString("--"));
   const QChar dashChar('-');
   const QChar assignChar('=');

   bool doubleDashFound = false;

   errorText.clear();
   positionalArgumentList.clear();
   optionNames.clear();
   unknownOptionNames.clear();
   optionValuesHash.clear();

   if (args.isEmpty()) {
      qWarning("QCommandLineParser::parse() Argument list can not be empty");
      return false;
   }

   QStringList::const_iterator argumentIterator = args.begin();
   ++argumentIterator; // skip executable name

   for (; argumentIterator != args.end() ; ++argumentIterator) {
      QString argument = *argumentIterator;

      if (doubleDashFound) {
         positionalArgumentList.append(argument);

      } else if (argument.startsWith(doubleDashString)) {
         if (argument.length() > 2) {
            QString optionName = argument.mid(2).section(assignChar, 0, 0);

            if (registerFoundOption(optionName)) {
               if (! parseOptionValue(optionName, argument, &argumentIterator, args.end())) {
                  error = true;
               }

            } else {
               error = true;
            }

         } else {
            doubleDashFound = true;
         }

      } else if (argument.startsWith(dashChar)) {
         if (argument.size() == 1) { // single dash ("stdin")
            positionalArgumentList.append(argument);
            continue;
         }

         switch (singleDashWordOptionMode) {
            case QCommandLineParser::ParseAsCompactedShortOptions: {
               QString optionName;
               bool valueFound = false;

               for (int pos = 1 ; pos < argument.size(); ++pos) {
                  optionName = argument.mid(pos, 1);

                  if (!registerFoundOption(optionName)) {
                     error = true;
                  } else {
                     const NameHash_t::const_iterator nameHashIt = nameHash.constFind(optionName);
                     Q_ASSERT(nameHashIt != nameHash.constEnd()); // checked by registerFoundOption
                     const NameHash_t::mapped_type optionOffset = *nameHashIt;
                     const bool withValue = !commandLineOptionList.at(optionOffset).valueName().isEmpty();

                     if (withValue) {
                        if (pos + 1 < argument.size()) {
                           if (argument.at(pos + 1) == assignChar) {
                              ++pos;
                           }

                           optionValuesHash[optionOffset].append(argument.mid(pos + 1));
                           valueFound = true;
                        }

                        break;
                     }

                     if (pos + 1 < argument.size() && argument.at(pos + 1) == assignChar) {
                        break;
                     }
                  }
               }

               if (! valueFound && !parseOptionValue(optionName, argument, &argumentIterator, args.end())) {
                  error = true;
               }

               break;
            }

            case QCommandLineParser::ParseAsLongOptions: {
               const QString optionName = argument.mid(1).section(assignChar, 0, 0);

               if (registerFoundOption(optionName)) {
                  if (! parseOptionValue(optionName, argument, &argumentIterator, args.end())) {
                     error = true;
                  }

               } else {
                  error = true;
               }

               break;
            }
         }
      } else {
         positionalArgumentList.append(argument);
      }

      if (argumentIterator == args.end()) {
         break;
      }
   }

   return !error;
}

bool QCommandLineParser::isSet(const QString &name) const
{
   d->checkParsed("isSet");

   if (d->optionNames.contains(name)) {
      return true;
   }

   const QStringList aliases = d->aliases(name);

   for (const QString &optionName : d->optionNames) {
      if (aliases.contains(optionName)) {
         return true;
      }
   }

   return false;
}

QString QCommandLineParser::value(const QString &optionName) const
{
   d->checkParsed("value");
   const QStringList valueList = values(optionName);

   if (! valueList.isEmpty()) {
      return valueList.last();
   }

   return QString();
}

QStringList QCommandLineParser::values(const QString &optionName) const
{
   d->checkParsed("values");
   const NameHash_t::const_iterator it = d->nameHash.find(optionName);

   if (it != d->nameHash.cend()) {
      const int optionOffset = *it;
      QStringList values = d->optionValuesHash.value(optionOffset);

      if (values.isEmpty()) {
         values = d->commandLineOptionList.at(optionOffset).defaultValues();
      }

      return values;
   }

   qWarning("QCommandLineParser::values() Option not defined, %s", csPrintable(optionName));
   return QStringList();
}

bool QCommandLineParser::isSet(const QCommandLineOption &option) const
{
   return isSet(option.names().first());
}

QString QCommandLineParser::value(const QCommandLineOption &option) const
{
   return value(option.names().first());
}

QStringList QCommandLineParser::values(const QCommandLineOption &option) const
{
   return values(option.names().first());
}

QStringList QCommandLineParser::positionalArguments() const
{
   d->checkParsed("positionalArguments");
   return d->positionalArgumentList;
}

QStringList QCommandLineParser::optionNames() const
{
   d->checkParsed("optionNames");
   return d->optionNames;
}

QStringList QCommandLineParser::unknownOptionNames() const
{
   d->checkParsed("unknownOptionNames");
   return d->unknownOptionNames;
}

void QCommandLineParser::showVersion()
{
   printf("%s %s\n", csPrintable(QCoreApplication::applicationName()),
      csPrintable(QCoreApplication::applicationVersion()));
   ::exit(EXIT_SUCCESS);
}


void QCommandLineParser::showHelp(int exitCode)
{
   printf("%s", csPrintable(d->helpText()));
   ::exit(exitCode);
}

QString QCommandLineParser::helpText() const
{
   return d->helpText();
}

static QString wrapText(const QString &names, int longestOptionNameString, const QString &description)
{
   const QChar nl('\n');

   QString text = QString("  ") + names.leftJustified(longestOptionNameString) + QChar(' ');
   const int indent = text.length();

   int lineStart     = 0;
   int lastBreakable = -1;
   const int max     = 79 - indent;
   int x = 0;
   const int len = description.length();

   for (int i = 0; i < len; ++i) {
      ++x;
      const QChar c = description.at(i);

      if (c.isSpace()) {
         lastBreakable = i;
      }

      int breakAt = -1;
      int nextLineStart = -1;

      if (x > max && lastBreakable != -1) {
         // time to break and we know where
         breakAt = lastBreakable;
         nextLineStart = lastBreakable + 1;

      } else if ((x > max - 1 && lastBreakable == -1) || i == len - 1) {
         // time to break but found nowhere [-> break here], or end of last line
         breakAt = i + 1;
         nextLineStart = breakAt;

      } else if (c == nl) {
         // forced break
         breakAt = i;
         nextLineStart = i + 1;
      }

      if (breakAt != -1) {
         const int numChars = breakAt - lineStart;

         if (lineStart > 0) {
            text += QString(indent, QChar(' '));
         }

         text += description.mid(lineStart, numChars) + nl;
         x = 0;
         lastBreakable = -1;
         lineStart = nextLineStart;

         if (lineStart < len && description.at(lineStart).isSpace()) {
            ++lineStart;   // don't start a line with a space
         }

         i = lineStart;
      }
   }

   return text;
}

QString QCommandLineParserPrivate::helpText() const
{
   const QChar nl('\n');
   QString text;

   const QString exeName = QCoreApplication::instance()->arguments().first();
   QString usage = exeName;

   if (! commandLineOptionList.isEmpty()) {
      usage += QChar(' ');
      usage += QCommandLineParser::tr("[options]");
   }

   for (const PositionalArgumentDefinition &arg : positionalArgumentDefinitions) {
      usage += QChar(' ');
      usage += arg.syntax;
   }

   text += QCommandLineParser::tr("Usage: %1").formatArg(usage) + nl;

   if (! description.isEmpty()) {
      text += description + nl;
   }

   text += nl;

   if (! commandLineOptionList.isEmpty()) {
      text += QCommandLineParser::tr("Options:") + nl;
   }

   QStringList optionNameList;
   int longestOptionNameString = 0;

   for (const QCommandLineOption &option : commandLineOptionList) {
      QStringList tmpOptionNames;

      for (const QString &optionName : option.names()) {
         if (optionName.length() == 1) {
            tmpOptionNames.append(QChar('-') + optionName);
         } else {
            tmpOptionNames.append(QString("--") + optionName);
         }
      }

      QString optionNamesString = tmpOptionNames.join(QString(", "));

      if (! option.valueName().isEmpty()) {
         optionNamesString += QString(" <") + option.valueName() + QChar('>');
      }

      optionNameList.append(optionNamesString);
      longestOptionNameString = qMax(longestOptionNameString, optionNamesString.length());
   }

   ++longestOptionNameString;

   for (int i = 0; i < commandLineOptionList.count(); ++i) {
      const QCommandLineOption &option = commandLineOptionList.at(i);
      text += wrapText(optionNameList.at(i), longestOptionNameString, option.description());
   }

   if (!positionalArgumentDefinitions.isEmpty()) {
      if (!commandLineOptionList.isEmpty()) {
         text += nl;
      }

      text += QCommandLineParser::tr("Arguments:") + nl;

      for (const PositionalArgumentDefinition &arg : positionalArgumentDefinitions) {
         text += wrapText(arg.name, longestOptionNameString, arg.description);
      }
   }

   return text;
}
