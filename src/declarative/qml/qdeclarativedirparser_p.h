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

#ifndef QDECLARATIVEDIRPARSER_P_H
#define QDECLARATIVEDIRPARSER_P_H

#include <QtCore/QUrl>
#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

class QDeclarativeError;
class QDeclarativeDirParser
{
   Q_DISABLE_COPY(QDeclarativeDirParser)

 public:
   QDeclarativeDirParser();
   ~QDeclarativeDirParser();

   QUrl url() const;
   void setUrl(const QUrl &url);

   QString source() const;
   void setSource(const QString &source);

   QString fileSource() const;
   void setFileSource(const QString &filePath);

   bool isParsed() const;
   bool parse();

   bool hasError() const;
   QList<QDeclarativeError> errors(const QString &uri) const;

   struct Plugin {
      Plugin() {}

      Plugin(const QString &name, const QString &path)
         : name(name), path(path) {}

      QString name;
      QString path;
   };

   struct Component {
      Component()
         : majorVersion(0), minorVersion(0), internal(false) {}

      Component(const QString &typeName, const QString &fileName, int majorVersion, int minorVersion)
         : typeName(typeName), fileName(fileName), majorVersion(majorVersion), minorVersion(minorVersion),
           internal(false) {}

      QString typeName;
      QString fileName;
      int majorVersion;
      int minorVersion;
      bool internal;
   };

   QList<Component> components() const;
   QList<Plugin> plugins() const;

#ifdef QT_CREATOR
   struct TypeInfo {
      TypeInfo() {}
      TypeInfo(const QString &fileName)
         : fileName(fileName) {}

      QString fileName;
   };

   QList<TypeInfo> typeInfos() const;
#endif

 private:
   void reportError(int line, int column, const QString &message);

 private:
   QList<QDeclarativeError> _errors;
   QUrl _url;
   QString _source;
   QString _filePathSouce;
   QList<Component> _components;
   QList<Plugin> _plugins;
#ifdef QT_CREATOR
   QList<TypeInfo> _typeInfos;
#endif
   unsigned _isParsed: 1;
};

typedef QList<QDeclarativeDirParser::Component> QDeclarativeDirComponents;


QT_END_NAMESPACE

#endif // QDECLARATIVEDIRPARSER_P_H
