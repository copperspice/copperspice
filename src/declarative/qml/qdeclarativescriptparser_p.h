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

#ifndef QDECLARATIVESCRIPTPARSER_P_H
#define QDECLARATIVESCRIPTPARSER_P_H

#include <qdeclarativeerror.h>
#include <qdeclarativeparser_p.h>
#include <QtCore/QList>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class QByteArray;

class QDeclarativeScriptParserJsASTData;
class QDeclarativeScriptParser
{
 public:
   class Import
   {
    public:
      Import() : type(Library) {}

      enum Type { Library, File, Script, Implicit }; //Implicit is only used internally
      Type type;

      QString uri;
      QString qualifier;
      QString version;

      QDeclarativeParser::LocationSpan location;
   };

   class TypeReference
   {
    public:
      TypeReference(int typeId, const QString &typeName) : id(typeId), name(typeName) {}

      int id;
      // type as it has been referenced in Qml
      QString name;
      // objects in parse tree referencing the type
      QList<QDeclarativeParser::Object *> refObjects;
   };

   QDeclarativeScriptParser();
   ~QDeclarativeScriptParser();

   bool parse(const QByteArray &data, const QUrl &url = QUrl());

   QList<TypeReference *> referencedTypes() const;

   QDeclarativeParser::Object *tree() const;
   QList<Import> imports() const;

   void clear();

   QList<QDeclarativeError> errors() const;

   class JavaScriptMetaData
   {
    public:
      JavaScriptMetaData()
         : pragmas(QDeclarativeParser::Object::ScriptBlock::None) {}

      QDeclarativeParser::Object::ScriptBlock::Pragmas pragmas;
      QList<Import> imports;
   };

   static QDeclarativeParser::Object::ScriptBlock::Pragmas extractPragmas(QString &);
   static JavaScriptMetaData extractMetaData(QString &);


   // ### private:
   TypeReference *findOrCreateType(const QString &name);
   void setTree(QDeclarativeParser::Object *tree);

   void setScriptFile(const QString &filename) {
      _scriptFile = filename;
   }
   QString scriptFile() const {
      return _scriptFile;
   }

   // ### private:
   QList<QDeclarativeError> _errors;

   QDeclarativeParser::Object *root;
   QList<Import> _imports;
   QList<TypeReference *> _refTypes;
   QString _scriptFile;
   QDeclarativeScriptParserJsASTData *data;
};

QT_END_NAMESPACE

#endif // QDECLARATIVESCRIPTPARSER_P_H
