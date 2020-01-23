/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QDECLARATIVELISTSCRIPTCLASS_P_H
#define QDECLARATIVELISTSCRIPTCLASS_P_H

#include <qscriptdeclarativeclass_p.h>
#include <qdeclarativelist.h>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QDeclarativeListScriptClass : public QScriptDeclarativeClass
{
 public:
   QDeclarativeListScriptClass(QDeclarativeEngine *);
   ~QDeclarativeListScriptClass();

   QScriptValue newList(QObject *, int, int);
   QScriptValue newList(const QDeclarativeListProperty<QObject> &, int);

 protected:
   virtual QScriptClass::QueryFlags queryProperty(Object *, const Identifier &,
         QScriptClass::QueryFlags flags);
   virtual Value property(Object *, const Identifier &);
   virtual QVariant toVariant(Object *, bool *ok);

 private:
   PersistentIdentifier m_lengthId;
   QDeclarativeEngine *engine;

   quint32 lastIndex;
};

QT_END_NAMESPACE

#endif // QDECLARATIVELISTSCRIPTCLASS_P_H

