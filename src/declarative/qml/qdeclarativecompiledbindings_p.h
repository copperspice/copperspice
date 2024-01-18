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

#ifndef QDECLARATIVECompiledBindings_P_H
#define QDECLARATIVECompiledBindings_P_H

#include <qdeclarativeexpression_p.h>
#include <qdeclarativebinding_p.h>

QT_BEGIN_NAMESPACE

struct QDeclarativeBindingCompilerPrivate;
class QDeclarativeBindingCompiler
{
 public:
   QDeclarativeBindingCompiler();
   ~QDeclarativeBindingCompiler();

   // Returns true if bindings were compiled
   bool isValid() const;

   struct Expression {
      QDeclarativeParser::Object *component;
      QDeclarativeParser::Object *context;
      QDeclarativeParser::Property *property;
      QDeclarativeParser::Variant expression;
      QHash<QString, QDeclarativeParser::Object *> ids;
      QDeclarativeImports imports;
   };

   // -1 on failure, otherwise the binding index to use
   int compile(const Expression &, QDeclarativeEnginePrivate *);

   // Returns the compiled program
   QByteArray program() const;

   static void dump(const QByteArray &);
 private:
   QDeclarativeBindingCompilerPrivate *d;
};

class QDeclarativeCompiledBindingsPrivate;
class QDeclarativeCompiledBindings : public QObject, public QDeclarativeAbstractExpression, public QDeclarativeRefCount
{
 public:
   QDeclarativeCompiledBindings(const char *program, QDeclarativeContextData *context, QDeclarativeRefCount *);
   virtual ~QDeclarativeCompiledBindings();

   QDeclarativeAbstractBinding *configBinding(int index, QObject *target, QObject *scope, int property);

 protected:
   int qt_metacall(QMetaObject::Call, int, void **);

 private:
   Q_DISABLE_COPY(QDeclarativeCompiledBindings)
   Q_DECLARE_PRIVATE(QDeclarativeCompiledBindings)
};

QT_END_NAMESPACE

#endif // QDECLARATIVEBINDINGOPTIMIZATIONS_P_H

