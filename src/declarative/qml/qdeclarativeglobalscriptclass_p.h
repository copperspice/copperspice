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

#ifndef QDECLARATIVEGLOBALSCRIPTCLASS_P_H
#define QDECLARATIVEGLOBALSCRIPTCLASS_P_H

#include <QtScript/qscriptclass.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

class QDeclarativeGlobalScriptClass : public QScriptClass
{
 public:
   QDeclarativeGlobalScriptClass(QScriptEngine *);

   virtual QueryFlags queryProperty(const QScriptValue &object,
                                    const QScriptString &name,
                                    QueryFlags flags, uint *id);

   virtual void setProperty(QScriptValue &object, const QScriptString &name,
                            uint id, const QScriptValue &value);

   void explicitSetProperty(const QStringList &, const QList<QScriptValue> &);

   const QScriptValue &staticGlobalObject() const {
      return m_staticGlobalObject;
   }

   const QSet<QString> &illegalNames() const {
      return m_illegalNames;
   }

 private:
   QSet<QString> m_illegalNames;
   QScriptValue m_staticGlobalObject;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEGLOBALSCRIPTCLASS_P_H
