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

#include "private/qdeclarativedirparser_p.h"
#include "qdeclarativeerror.h"
#include <qdeclarativeglobal_p.h>

#include <QTextStream>
#include <QFile>
#include <QDebug>

QT_BEGIN_NAMESPACE

QDeclarativeDirParser::QDeclarativeDirParser()
   : _isParsed(false)
{
}

QDeclarativeDirParser::~QDeclarativeDirParser()
{
}

QUrl QDeclarativeDirParser::url() const
{
   return _url;
}

void QDeclarativeDirParser::setUrl(const QUrl &url)
{
   _url = url;
}

QString QDeclarativeDirParser::fileSource() const
{
   return _filePathSouce;
}

void QDeclarativeDirParser::setFileSource(const QString &filePath)
{
   _filePathSouce = filePath;
}

QString QDeclarativeDirParser::source() const
{
   return _source;
}

void QDeclarativeDirParser::setSource(const QString &source)
{
   _isParsed = false;
   _source = source;
}

bool QDeclarativeDirParser::isParsed() const
{
   return _isParsed;
}

bool QDeclarativeDirParser::parse()
{
   if (_isParsed) {
      return true;
   }

   _isParsed = true;
   _errors.clear();
   _plugins.clear();
   _components.clear();

   if (_source.isEmpty() && !_filePathSouce.isEmpty()) {
      QFile file(_filePathSouce);
      if (!QDeclarative_isFileCaseCorrect(_filePathSouce)) {
         QDeclarativeError error;
         error.setDescription(QString::fromUtf8("cannot load module \"$$URI$$\": File name case mismatch for \"%1\"").arg(
                                 _filePathSouce));
         _errors.prepend(error);
         return false;
      } else if (file.open(QFile::ReadOnly)) {
         _source = QString::fromUtf8(file.readAll());
      } else {
         QDeclarativeError error;
         error.setDescription(QString::fromUtf8("module \"$$URI$$\" definition \"%1\" not readable").arg(_filePathSouce));
         _errors.prepend(error);
         return false;
      }
   }

   QTextStream stream(&_source);
   int lineNumber = 0;

   forever {
      ++lineNumber;

      const QString line = stream.readLine();
      if (line.isNull())
      {
         break;
      }

      QString sections[3];
      int sectionCount = 0;

      int index = 0;
      const int length = line.length();

      while (index != length)
      {
         const QChar ch = line.at(index);

         if (ch.isSpace()) {
            do {
               ++index;
            } while (index != length && line.at(index).isSpace());

         } else if (ch == QLatin1Char('#')) {
            // recognized a comment
            break;

         } else {
            const int start = index;

            do {
               ++index;
            } while (index != length && !line.at(index).isSpace());

            const QString lexeme = line.mid(start, index - start);

            if (sectionCount >= 3) {
               reportError(lineNumber, start, QLatin1String("unexpected token"));

            } else {
               sections[sectionCount++] = lexeme;
            }
         }
      }

      if (sectionCount == 0)
      {
         continue; // no sections, no party.

      } else if (sections[0] == QLatin1String("plugin"))
      {
         if (sectionCount < 2) {
            reportError(lineNumber, -1,
                        QString::fromUtf8("plugin directive requires 2 arguments, but %1 were provided").arg(sectionCount + 1));

            continue;
         }

         const Plugin entry(sections[1], sections[2]);

         _plugins.append(entry);

      } else if (sections[0] == QLatin1String("internal"))
      {
         if (sectionCount != 3) {
            reportError(lineNumber, -1,
                        QString::fromUtf8("internal types require 2 arguments, but %1 were provided").arg(sectionCount + 1));
            continue;
         }
         Component entry(sections[1], sections[2], -1, -1);
         entry.internal = true;
         _components.append(entry);
      } else if (sections[0] == QLatin1String("typeinfo"))
      {
         if (sectionCount != 2) {
            reportError(lineNumber, -1,
                        QString::fromUtf8("typeinfo requires 1 argument, but %1 were provided").arg(sectionCount - 1));
            continue;
         }
#ifdef QT_CREATOR
         TypeInfo typeInfo(sections[1]);
         _typeInfos.append(typeInfo);
#endif

      } else if (sectionCount == 2)
      {
         // No version specified (should only be used for relative qmldir files)
         const Component entry(sections[0], sections[1], -1, -1);
         _components.append(entry);
      } else if (sectionCount == 3)
      {
         const QString &version = sections[1];
         const int dotIndex = version.indexOf(QLatin1Char('.'));

         if (dotIndex == -1) {
            reportError(lineNumber, -1, QLatin1String("expected '.'"));
         } else if (version.indexOf(QLatin1Char('.'), dotIndex + 1) != -1) {
            reportError(lineNumber, -1, QLatin1String("unexpected '.'"));
         } else {
            bool validVersionNumber = false;
            const int majorVersion = version.left(dotIndex).toInt(&validVersionNumber);

            if (validVersionNumber) {
               const int minorVersion = version.mid(dotIndex + 1).toInt(&validVersionNumber);

               if (validVersionNumber) {
                  const Component entry(sections[0], sections[2], majorVersion, minorVersion);

                  _components.append(entry);
               }
            }
         }
      } else {
         reportError(lineNumber, -1,
         QString::fromUtf8("a component declaration requires 3 arguments, but %1 were provided").arg(sectionCount + 1));
      }
   }

   return hasError();
}

void QDeclarativeDirParser::reportError(int line, int column, const QString &description)
{
   QDeclarativeError error;
   error.setUrl(_url);
   error.setLine(line);
   error.setColumn(column);
   error.setDescription(description);
   _errors.append(error);
}

bool QDeclarativeDirParser::hasError() const
{
   if (! _errors.isEmpty()) {
      return true;
   }

   return false;
}

QList<QDeclarativeError> QDeclarativeDirParser::errors(const QString &uri) const
{
   QList<QDeclarativeError> errors = _errors;
   for (int i = 0; i < errors.size(); ++i) {
      QDeclarativeError &e = errors[i];
      QString description = e.description();
      description.replace(QLatin1String("$$URI$$"), uri);
      e.setDescription(description);
   }
   return errors;
}

QList<QDeclarativeDirParser::Plugin> QDeclarativeDirParser::plugins() const
{
   return _plugins;
}

QList<QDeclarativeDirParser::Component> QDeclarativeDirParser::components() const
{
   return _components;
}

#ifdef QT_CREATOR
QList<QDeclarativeDirParser::TypeInfo> QDeclarativeDirParser::typeInfos() const
{
   return _typeInfos;
}
#endif

QT_END_NAMESPACE
