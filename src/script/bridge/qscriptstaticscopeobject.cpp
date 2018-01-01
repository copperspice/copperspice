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

#include "config.h"
#include "qscriptstaticscopeobject_p.h"

namespace JSC {
ASSERT_CLASS_FITS_IN_CELL(QT_PREPEND_NAMESPACE(QScriptStaticScopeObject));
}

QT_BEGIN_NAMESPACE

/*!
  \class QScriptStaticScopeObject
  \internal

    Represents a static scope object.

    This class allows the VM to determine at JS script compile time whether
    the object has a given property or not. If the object has the property,
    a fast, index-based read/write operation will be used. If the object
    doesn't have the property, the compiler knows it can safely skip this
    object when dynamically resolving the property. Either way, this can
    greatly improve performance.

  \sa QScriptContext::pushScope()
*/

const JSC::ClassInfo QScriptStaticScopeObject::info = { "QScriptStaticScopeObject", 0, 0, 0 };

/*!
    Creates a static scope object with a fixed set of undeletable properties.

    It's not possible to add new properties to the object after construction.
*/
QScriptStaticScopeObject::QScriptStaticScopeObject(WTF::NonNullPassRefPtr<JSC::Structure> structure,
      int propertyCount, const PropertyInfo *props)
   : JSC::JSVariableObject(structure, new Data(/*canGrow=*/false))
{
   int index = growRegisterArray(propertyCount);
   for (int i = 0; i < propertyCount; ++i, --index) {
      const PropertyInfo &prop = props[i];
      JSC::SymbolTableEntry entry(index, prop.attributes);
      symbolTable().add(prop.identifier.ustring().rep(), entry);
      registerAt(index) = prop.value;
   }
}

/*!
    Creates an empty static scope object.

    Properties can be added to the object after construction, either by
    calling QScriptValue::setProperty(), or by pushing the object on the
    scope chain; variable declarations ("var" statements) and function
    declarations in JavaScript will create properties on the scope object.

    Note that once the scope object has been used in a closure and the
    resulting function has been compiled, it's no longer safe to add
    properties to the scope object (because the VM will bypass this
    object the next time the function is executed).
*/
QScriptStaticScopeObject::QScriptStaticScopeObject(WTF::NonNullPassRefPtr<JSC::Structure> structure)
   : JSC::JSVariableObject(structure, new Data(/*canGrow=*/true))
{
}

QScriptStaticScopeObject::~QScriptStaticScopeObject()
{
   delete d_ptr();
}

bool QScriptStaticScopeObject::getOwnPropertySlot(JSC::ExecState *, const JSC::Identifier &propertyName,
      JSC::PropertySlot &slot)
{
   return symbolTableGet(propertyName, slot);
}

bool QScriptStaticScopeObject::getOwnPropertyDescriptor(JSC::ExecState *, const JSC::Identifier &propertyName,
      JSC::PropertyDescriptor &descriptor)
{
   return symbolTableGet(propertyName, descriptor);
}

void QScriptStaticScopeObject::putWithAttributes(JSC::ExecState *exec, const JSC::Identifier &propertyName,
      JSC::JSValue value, unsigned attributes)
{
   if (symbolTablePutWithAttributes(propertyName, value, attributes)) {
      return;
   }
   Q_ASSERT(d_ptr()->canGrow);
   addSymbolTableProperty(propertyName, value, attributes);
}

void QScriptStaticScopeObject::put(JSC::ExecState *exec, const JSC::Identifier &propertyName, JSC::JSValue value,
                                   JSC::PutPropertySlot &)
{
   if (symbolTablePut(propertyName, value)) {
      return;
   }
   Q_ASSERT(d_ptr()->canGrow);
   addSymbolTableProperty(propertyName, value, /*attributes=*/0);
}

bool QScriptStaticScopeObject::deleteProperty(JSC::ExecState *, const JSC::Identifier &)
{
   return false;
}

void QScriptStaticScopeObject::markChildren(JSC::MarkStack &markStack)
{
   JSC::Register *registerArray = d_ptr()->registerArray.get();
   if (!registerArray) {
      return;
   }
   markStack.appendValues(reinterpret_cast<JSC::JSValue *>(registerArray), d_ptr()->registerArraySize);
}

void QScriptStaticScopeObject::addSymbolTableProperty(const JSC::Identifier &name, JSC::JSValue value,
      unsigned attributes)
{
   int index = growRegisterArray(1);
   JSC::SymbolTableEntry newEntry(index, attributes | JSC::DontDelete);
   symbolTable().add(name.ustring().rep(), newEntry);
   registerAt(index) = value;
}

/*!
  Grows the register array by \a count elements, and returns the offset of
  the newly added elements (note that the register file grows downwards,
  starting at index -1).
*/
int QScriptStaticScopeObject::growRegisterArray(int count)
{
   size_t oldSize = d_ptr()->registerArraySize;
   size_t newSize = oldSize + count;
   JSC::Register *registerArray = new JSC::Register[newSize];
   if (d_ptr()->registerArray) {
      memcpy(registerArray + count, d_ptr()->registerArray.get(), oldSize * sizeof(JSC::Register));
   }
   setRegisters(registerArray + newSize, registerArray);
   d_ptr()->registerArraySize = newSize;
   return -oldSize - 1;
}

QT_END_NAMESPACE
