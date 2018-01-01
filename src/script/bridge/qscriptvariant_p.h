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

#ifndef QSCRIPTVARIANT_P_H
#define QSCRIPTVARIANT_P_H

#include <qvariant.h>
#include "qscriptobject_p.h"

QT_BEGIN_NAMESPACE

namespace QScript {

class QVariantDelegate : public QScriptObjectDelegate
{
 public:
   QVariantDelegate(const QVariant &value);
   ~QVariantDelegate();

   virtual bool compareToObject(QScriptObject *, JSC::ExecState *, JSC::JSObject *);

   QVariant &value();
   void setValue(const QVariant &value);

   Type type() const;

 private:
   QVariant m_value;
};

class QVariantPrototype : public QScriptObject
{
 public:
   QVariantPrototype(JSC::ExecState *, WTF::PassRefPtr<JSC::Structure>,
                     JSC::Structure *prototypeFunctionStructure);
};

} // namespace QScript

QT_END_NAMESPACE

#endif
