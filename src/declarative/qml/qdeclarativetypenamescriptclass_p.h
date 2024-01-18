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

#ifndef QDECLARATIVETYPENAMESCRIPTCLASS_P_H
#define QDECLARATIVETYPENAMESCRIPTCLASS_P_H

#include <qdeclarativeengine_p.h>
#include <qscriptdeclarativeclass_p.h>
#include <QtScript/qscriptclass.h>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QDeclarativeType;
class QDeclarativeTypeNameCache;
class QDeclarativeTypeNameScriptClass : public QScriptDeclarativeClass
{
 public:
   QDeclarativeTypeNameScriptClass(QDeclarativeEngine *);
   ~QDeclarativeTypeNameScriptClass();

   enum TypeNameMode { IncludeEnums, ExcludeEnums };
   QScriptValue newObject(QObject *, QDeclarativeType *, TypeNameMode = IncludeEnums);
   QScriptValue newObject(QObject *, QDeclarativeTypeNameCache *, TypeNameMode = IncludeEnums);

 protected:
   virtual QScriptClass::QueryFlags queryProperty(Object *, const Identifier &,
         QScriptClass::QueryFlags flags);

   virtual Value property(Object *, const Identifier &);
   virtual void setProperty(Object *, const Identifier &name, const QScriptValue &);

 private:
   QDeclarativeEngine *engine;
   QObject *object;
   QDeclarativeType *type;
   quint32 enumValue;
};

QT_END_NAMESPACE

#endif // QDECLARATIVETYPENAMESCRIPTCLASS_P_H

