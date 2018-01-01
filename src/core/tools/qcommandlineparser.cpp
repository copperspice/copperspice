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

/*****************************************************
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Copyright (C) 2013 David Faure <faure@kde.org>
*****************************************************/

#include <qcommandlineparser.h>

#include <qcoreapplication.h>
#include <qhash.h>
#include <qvector.h>
#include <stdio.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

typedef QHash<QString, int> NameHash_t;

class QCommandLineParserPrivate
{
 public:
   inline QCommandLineParserPrivate()
      : singleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions),
        builtinVersionOption(false),
        builtinHelpOption(false),
        needsParsing(true) {
   }

   bool parse(const QStringList &args);
   void checkParsed(const char *method);
   QStringList aliases(const QString &name) const;
   QString helpText() const;
   bool registerFoundOption(const QString &optionName);
   bool parseOptionValue(const QString &optionName, const QString &argument,
                         QStringList::const_iterator *argumentIterator,
                         QStringList::const_iterator argsEnd);

   //! Error text set when parse() returns false
   QString errorText;

   //! The command line options used for parsing
   QList<QCommandLineOption> commandLineOptionList;

   //! Hash mapping option names to their offsets in commandLineOptionList and optionArgumentList.
   NameHash_t nameHash;

   //! Option values found (only for options with a value)
   QHash<int, QStringList> optionValuesHash;

   //! Names of options found on the command line.
   QStringList optionNames;

   //! Arguments which did not belong to any option.
   QStringList positionalArgumentList;

   //! Names of options which were unknown.
   QStringList unknownOptionNames;

   //! Application description
   QString description;

   //! Documentation for positional arguments
   struct PositionalArgumentDefinition {
      QString name;
      QString description;
      QString syntax;
   };
   QVector<PositionalArgumentDefinition> positionalArgumentDefinitions;

   //! The parsing mode for "-abc"
   QCommandLineParser::SingleDashWordOptionMode singleDashWordOptionMode;

   //! Whether addVersionOption was called
   bool builtinVersionOption;

   //! Whether addHelpOption was called
   bool builtinHelpOption;

   //! True if parse() needs to be called
   bool needsParsing;
};

QStringList QCommandLineParserPrivate::aliases(const QString &optionName) const
{
   const NameHash_t::const_iterator it = nameHash.find(optionName);
   if (it == nameHash.end()) {
      qWarning("QCommandLineParser: option not defined: \"%s\"", qPrintable(optionName));
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

void QCommandLineParser::setSingleDashWordOptionMode(QCommandLineParser::SingleDashWordOptionMode
      singleDashWordOptionMode)
{
   d->singleDashWordOptionMode = singleDashWordOptionMode;
}

bool QCommandLineParser::addOption(const QCommandLineOption &option)
{
   QStringList optionNames = option.names();

   if (!optionNames.isEmpty()) {
      for (const QString & name : optionNames) {
         if (d->nameHash.contains(name)) {
            return false;
         }
      }

      d->commandLineOptionList.append(option);

      const int offset = d->commandLineOptionList.size() - 1;
      for (const QString & name : optionNames)
      d->nameHash.insert(name, offset);

      return true;
   }

   return false;
}

/*!
    Adds the \c{-v} / \c{--version} option, which displays the version string of the application.

    This option is handled automatically by QCommandLineParser.

    You can set the actual version string by using QCoreApplication::setApplicationVersion().

    Returns the option instance, which can be used to call isSet().
*/
QCommandLineOption QCommandLineParser::addVersionOption()
{
   d->builtinVersionOption = true;
   QCommandLineOption opt(QStringList() << QString("v") << QString("version"), tr("Displays version information."));
   addOption(opt);
   return opt;
}

QCommandLineOption QCommandLineParser::addHelpOption()
{
   d->builtinHelpOption = true;
   QCommandLineOption opt(QStringList()
#ifdef Q_OS_WIN
                          << QString("?")
#endif
                          << QString("h")
                          << QString("help"), tr("Displays this help."));
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
   if (!d->errorText.isEmpty()) {
      return d->errorText;
   }
   if (d->unknownOptionNames.count() == 1) {
      return tr("Unknown option '%1'.").arg(d->unknownOptionNames.first());
   }
   if (d->unknownOptionNames.count() > 1) {
      return tr("Unknown options: %1.").arg(d->unknownOptionNames.join(QString(", ")));
   }
   return QString();
}

void QCommandLineParser::process(const QStringList &arguments)
{
   if (!d->parse(arguments)) {
      fprintf(stderr, "%s\n", qPrintable(errorText()));
      ::exit(EXIT_FAILURE);
   }

   if (d->builtinVersionOption && isSet(QString("version"))) {
      printf("%s %s\n", qPrintable(QCoreApplication::applicationName()), qPrintable(QCoreApplication::applicationVersion()));
      ::exit(EXIT_SUCCESS);
   }

   if (d->builtinHelpOption && isSet(QString("help"))) {
      showHelp(EXIT_SUCCESS);
   }
}

void QCommandLineParser::process(const QCoreApplication &app)
{
   // QCoreApplication::arguments() is static, but the app instance must exist so we require it as parameter
   Q_UNUSED(app);
   process(QCoreApplication::arguments());
}

void QCommandLineParserPrivate::checkParsed(const char *method)
{
   if (needsParsing) {
      qWarning("QCommandLineParser: call process() or parse() before %s", method);
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
   const QLatin1Char assignChar('=');
   const NameHash_t::const_iterator nameHashIt = nameHash.constFind(optionName);
   if (nameHashIt != nameHash.constEnd()) {
      const int assignPos = argument.indexOf(assignChar);
      const NameHash_t::mapped_type optionOffset = *nameHashIt;
      const bool withValue = !commandLineOptionList.at(optionOffset).valueName().isEmpty();
      if (withValue) {
         if (assignPos == -1) {
            ++(*argumentIterator);
            if (*argumentIterator == argsEnd) {
               errorText = QCommandLineParser::tr("Missing value after '%1'.").arg(argument);
               return false;
            }
            optionValuesHash[optionOffset].append(*(*argumentIterator));
         } else {
            optionValuesHash[optionOffset].append(argument.mid(assignPos + 1));
         }
      } else {
         if (assignPos != -1) {
            errorText = QCommandLineParser::tr("Unexpected value after '%1'.").arg(argument.left(assignPos));
            return false;
         }
      }
   }
   return true;
}

bool QCommandLineParserPrivate::parse(const QStringList &args)
{
   needsParsing = false;
   bool error = false;

   const QString     doubleDashString(QString("--"));
   const QLatin1Char dashChar('-');
   const QLatin1Char assignChar('=');

   bool doubleDashFound = false;
   errorText.clear();
   positionalArgumentList.clear();
   optionNames.clear();
   unknownOptionNames.clear();
   optionValuesHash.clear();

   if (args.isEmpty()) {
      qWarning("QCommandLineParser: argument list cannot be empty, it should contain at least the executable name");
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
               if (!parseOptionValue(optionName, argument, &argumentIterator, args.end())) {
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
               if (!valueFound && !parseOptionValue(optionName, argument, &argumentIterator, args.end())) {
                  error = true;
               }
               break;
            }
            case QCommandLineParser::ParseAsLongOptions: {
               const QString optionName = argument.mid(1).section(assignChar, 0, 0);
               if (registerFoundOption(optionName)) {
                  if (!parseOptionValue(optionName, argument, &argumentIterator, args.end())) {
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

/*!
    Checks whether the option \a name was passed to the application.

    Returns true if the option \a name was set, false otherwise.

    This is the recommended way to check for options with no values.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the options names are treated as being
    equivalent. If the name is not recognized or that option was not present,
    false is returned.

    Example:
    \snippet code/src_corelib_tools_qcommandlineparser.cpp 0
 */

bool QCommandLineParser::isSet(const QString &name) const
{
   d->checkParsed("isSet");
   if (d->optionNames.contains(name)) {
      return true;
   }
   const QStringList aliases = d->aliases(name);
   for (const QString & optionName : d->optionNames) {
      if (aliases.contains(optionName)) {
         return true;
      }
   }
   return false;
}

/*!
    Returns the option value found for the given option name \a optionName, or
    an empty string if not found.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the option names are treated as being
    equivalent. If the name is not recognized or that option was not present, an
    empty string is returned.

    For options found by the parser, the last value found for
    that option is returned. If the option wasn't specified on the command line,
    the default value is returned.

    An empty string is returned if the option does not take a value.

    \sa values()
 */

QString QCommandLineParser::value(const QString &optionName) const
{
   d->checkParsed("value");
   const QStringList valueList = values(optionName);

   if (!valueList.isEmpty()) {
      return valueList.last();
   }

   return QString();
}

/*!
    Returns a list of option values found for the given option name \a
    optionName, or an empty list if not found.

    The name provided can be any long or short name of any option that was
    added with \c addOption(). All the options names are treated as being
    equivalent. If the name is not recognized or that option was not present, an
    empty list is returned.

    For options found by the parser, the list will contain an entry for
    each time the option was encountered by the parser. If the option wasn't
    specified on the command line, the default values are returned.

    An empty list is returned if the option does not take a value.

    \sa value()
 */

QStringList QCommandLineParser::values(const QString &optionName) const
{
   d->checkParsed("values");
   const NameHash_t::const_iterator it = d->nameHash.find(optionName);
   if (it != d->nameHash.end()) {
      const int optionOffset = *it;
      QStringList values = d->optionValuesHash.value(optionOffset);
      if (values.isEmpty()) {
         values = d->commandLineOptionList.at(optionOffset).defaultValues();
      }
      return values;
   }

   qWarning("QCommandLineParser: option not defined: \"%s\"", qPrintable(optionName));
   return QStringList();
}

/*!
    \overload
    Returns true if the \a option was set, false otherwise.
*/
bool QCommandLineParser::isSet(const QCommandLineOption &option) const
{
   return isSet(option.names().first());
}

/*!
    \overload
    Returns the option value found for the given \a option, or
    an empty string if not found.
*/
QString QCommandLineParser::value(const QCommandLineOption &option) const
{
   return value(option.names().first());
}

/*!
    \overload
    Returns a list of option values found for the given \a option,
    or an empty list if not found.
*/
QStringList QCommandLineParser::values(const QCommandLineOption &option) const
{
   return values(option.names().first());
}

/*!
    Returns a list of positional arguments.

    These are all of the arguments that were not recognized as part of an
    option.
 */

QStringList QCommandLineParser::positionalArguments() const
{
   d->checkParsed("positionalArguments");
   return d->positionalArgumentList;
}

/*!
    Returns a list of option names that were found.

    This returns a list of all the recognized option names found by the
    parser, in the order in which they were found. For any long options
    that were in the form {--option=value}, the value part will have been
    dropped.

    The names in this list do not include the preceding dash characters.
    Names may appear more than once in this list if they were encountered
    more than once by the parser.

    Any entry in the list can be used with \c value() or with
    \c values() to get any relevant option values.
 */

QStringList QCommandLineParser::optionNames() const
{
   d->checkParsed("optionNames");
   return d->optionNames;
}

/*!
    Returns a list of unknown option names.

    This list will include both long an short name options that were not
    recognized. For any long options that were in the form {--option=value},
    the value part will have been dropped and only the long name is added.

    The names in this list do not include the preceding dash characters.
    Names may appear more than once in this list if they were encountered
    more than once by the parser.

    \sa optionNames()
 */

QStringList QCommandLineParser::unknownOptionNames() const
{
   d->checkParsed("unknownOptionNames");
   return d->unknownOptionNames;
}

/*!
    Displays the help information, and exits the application.
    This is automatically triggered by the --help option, but can also
    be used to display the help when the user is not invoking the
    application correctly.
    The exit code is set to \a exitCode. It should be set to 0 if the
    user requested to see the help, and to any other value in case of
    an error.

    \sa helpText()
*/
void QCommandLineParser::showHelp(int exitCode)
{
   fprintf(stdout, "%s", qPrintable(d->helpText()));
   ::exit(exitCode);
}

/*!
    Returns a string containing the complete help information.

    \sa showHelp()
*/
QString QCommandLineParser::helpText() const
{
   return d->helpText();
}

static QString wrapText(const QString &names, int longestOptionNameString, const QString &description)
{
   const QLatin1Char nl('\n');
   QString text = QString("  ") + names.leftJustified(longestOptionNameString) + QLatin1Char(' ');
   const int indent = text.length();
   int lineStart = 0;
   int lastBreakable = -1;
   const int max = 79 - indent;
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
         //qDebug() << "breakAt=" << description.at(breakAt) << "breakAtSpace=" << breakAtSpace << lineStart << "to" << breakAt << description.mid(lineStart, numChars);
         if (lineStart > 0) {
            text += QString(indent, QLatin1Char(' '));
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
   const QLatin1Char nl('\n');
   QString text;
   const QString exeName = QCoreApplication::instance()->arguments().first();
   QString usage = exeName;
   if (!commandLineOptionList.isEmpty()) {
      usage += QLatin1Char(' ');
      usage += QCommandLineParser::tr("[options]");
   }

   for (const PositionalArgumentDefinition & arg : positionalArgumentDefinitions) {
      usage += QLatin1Char(' ');
      usage += arg.syntax;
   }

   text += QCommandLineParser::tr("Usage: %1").arg(usage) + nl;
   if (!description.isEmpty()) {
      text += description + nl;
   }
   text += nl;
   if (!commandLineOptionList.isEmpty()) {
      text += QCommandLineParser::tr("Options:") + nl;
   }

   QStringList optionNameList;
   int longestOptionNameString = 0;

   for (const QCommandLineOption & option : commandLineOptionList) {
      QStringList optionNames;

      for (const QString & optionName : option.names()) {
         if (optionName.length() == 1) {
            optionNames.append(QLatin1Char('-') + optionName);
         } else {
            optionNames.append(QString("--") + optionName);
         }
      }
      QString optionNamesString = optionNames.join(QString(", "));
      if (!option.valueName().isEmpty()) {
         optionNamesString += QString(" <") + option.valueName() + QLatin1Char('>');
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
      for (const PositionalArgumentDefinition & arg : positionalArgumentDefinitions) {
         text += wrapText(arg.name, longestOptionNameString, arg.description);
      }
   }
   return text;
}

QT_END_NAMESPACE
