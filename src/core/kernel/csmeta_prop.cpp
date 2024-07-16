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

// do not move these two includes
#include <qobject.h>
#include <csmeta.h>

#include <qmetaobject.h>

QMetaProperty::QMetaProperty(const QString &name, QMetaObject *obj)
   :  m_metaObject(obj), m_name(name),
      m_returnTypeId(typeid(void)), m_returnTypeFuncPtr(nullptr)
{
   m_typeName        = QString();
   m_writeMethodName = QString();

   m_read_able    = false;
   m_write_able   = false;
   m_reset_able   = false;
   m_notify_able  = false;

   m_constant     = false;
   m_final        = false;
   m_revision     = 0;

   m_designJar    = nullptr;
   m_scriptJar    = nullptr;
   m_storedJar    = nullptr;
   m_userJar      = nullptr;

   // register enums
   Qt::staticMetaObject();
}

QMetaEnum QMetaProperty::enumerator() const
{
   return QMetaObject::findEnum(m_returnTypeId);
}

bool QMetaProperty::hasNotifySignal() const
{
   return m_notify_able;
}

bool QMetaProperty::hasStdCppSet() const
{
   // used in CsDesigner
   // return true if the property named X has a setX() method

   // CORE_CS_PROPERTY_WRITE(objectName, setObjectName)
   // void setObjectName(const QString &name);

   bool retval = false;

   if (isWritable()) {
      QString setName = "set" + name();
      setName.replace(3, 1, setName[3].toUpper());

      if (setName == m_writeMethodName) {
         retval = true;

      } else {
         setName.prepend("cs_");

         if (setName == m_writeMethodName) {
            retval = true;
         }
      }
   }

   return retval;
}

bool QMetaProperty::isConstant() const
{
   return m_constant;
}

bool QMetaProperty::isDesignable(const QObject *object) const
{
   bool retval;

   if (m_designJar == nullptr) {
      retval = true;

   } else if (object == nullptr) {

      if (m_designJar->isStatic()) {
         retval = m_designJar->run<bool>(object);
      } else {
         // might be designable so default to yes
         retval = true;
      }

   } else {
      retval = m_designJar->run<bool>(object);
   }

   return retval;
}

bool QMetaProperty::isEnumType() const
{
   bool retval = false;
   QMetaEnum enumObj = this->enumerator();

   if (enumObj.isValid()) {
      retval = true;
   }

   return retval;
}

bool QMetaProperty::isFinal() const
{
   return m_final;
}

bool QMetaProperty::isFlagType() const
{
   bool retval = false;
   QMetaEnum enumObj = this->enumerator();

   if (enumObj.isValid()) {
      retval = enumObj.isFlag();
   }

   return retval;
}

bool QMetaProperty::isReadable() const
{
   return m_read_able;
}

bool QMetaProperty::isResettable() const
{
   return m_reset_able;
}

bool QMetaProperty::isScriptable(const QObject *object) const
{
   bool retval = m_scriptJar;

   if (m_scriptJar == nullptr) {
      retval = true;

   } else if (object == nullptr) {

      if (m_scriptJar->isStatic()) {
         retval = m_scriptJar->run<bool>(object);
      } else {
         // might be scriptable so default to yes
         retval = true;
      }

   } else {
      retval = m_scriptJar->run<bool>(object);
   }

   return retval;
}

bool QMetaProperty::isStored(const QObject *object) const
{
   bool retval = m_storedJar;

   if (m_storedJar == nullptr) {
      retval = true;

   } else if (object == nullptr) {

      if (m_storedJar->isStatic()) {
         retval = m_storedJar->run<bool>(object);
      } else {
         // might be storable so default to yes
         retval = true;
      }

   } else {
      retval = m_storedJar->run<bool>(object);
   }

   return retval;
}

bool QMetaProperty::isUser(const QObject *object) const
{
   bool retval = false;

   if (m_userJar == nullptr) {
      retval = false;

   } else if (object == nullptr) {

      if (m_userJar->isStatic()) {
         retval = m_userJar->run<bool>(object);
      } else {
         // default value
         retval = false;
      }

   } else {
      retval = m_userJar->run<bool>(object);
   }

   return retval;
}

bool QMetaProperty::isValid() const
{
   return m_read_able;
}

bool QMetaProperty::isWritable() const
{
   return m_write_able;
}

const QString &QMetaProperty::name() const
{
   return m_name;
}

void QMetaProperty::loadTypeName() const
{
   if (m_typeName.isEmpty()) {
      // populate m_typeName with a String denoting the return type of the property

      if (m_returnTypeFuncPtr != nullptr) {
         const_cast<QMetaProperty *>(this)->m_typeName = m_returnTypeFuncPtr();
      }
   }
}

QMetaMethod QMetaProperty::notifySignal() const
{
   static const QString str;

   int id = notifySignalIndex();

   if (id == -1) {
      return QMetaMethod(str, str, std::vector<QString>(), QMetaMethod::Private, QMetaMethod::Slot,
            QMetaMethod::Attributes(), m_metaObject);

   } else  {
      return m_metaObject->method(id);

   }
}

int QMetaProperty::notifySignalIndex() const
{
   int retval = -1;

   if (m_notify_able) {
      const int count = m_metaObject->methodCount();

      for (int index = 0; index < count; ++index)  {

         QMetaMethod metaMethod = m_metaObject->method(index);
         bool ok = metaMethod.compare(*m_notifyBento);

         if (ok) {
            // found QMetaMethod match
            retval = index;
            break;
         }
      }
   }

   return retval;
}

int QMetaProperty::propertyIndex() const
{
   if (m_metaObject == nullptr) {
      return -1;
   }

   return m_metaObject->indexOfProperty(m_name);
}

QVariant QMetaProperty::read(const QObject *object) const
{
   if (! object || ! m_readJar) {
      return QVariant();
   }

   QVariant retval = m_readJar->runV(object);

   return retval;
}

bool QMetaProperty::reset(QObject *object) const
{
   if (! object || ! m_reset_able) {
      return false;
   }

   bool retval = m_resetJar->runV(object);

   return retval;
}

// internal
int QMetaProperty::revision() const
{
   return m_revision;
}

// internal
void QMetaProperty::setConstant()
{
   m_constant = true;
}

// internal
void QMetaProperty::setFinal()
{
   m_final = true;
}

// internal
void QMetaProperty::setRevision(int value)
{
   m_revision = value;
}

// internal
void QMetaProperty::setTypeName(const QString &typeName)
{
   m_typeName = typeName;
}

QVariant::Type QMetaProperty::type() const
{
   QVariant::Type retval = QVariant::Invalid;
   QMetaEnum enumObj     = this->enumerator();

   loadTypeName();

   if (enumObj.isValid()) {
      // process enum
      QString enumName    = enumObj.scope() + "::" + enumObj.name();
      uint enumMetaTypeId = QVariant::nameToType(enumName);

      if (enumMetaTypeId == QVariant::Invalid) {
         retval = QVariant::Int;
      }

   } else if (! m_typeName.isEmpty()) {
      uint enumMetaTypeId = QVariant::nameToType(m_typeName);

      if (enumMetaTypeId < QVariant::UserType) {
         retval = static_cast<QVariant::Type>(enumMetaTypeId);
      }
   }

   return retval;
}

const QString &QMetaProperty::typeName() const
{
   loadTypeName();

   return m_typeName;
}

uint QMetaProperty::userType() const
{
   uint retval = QVariant::Invalid;
   QMetaEnum enumObj = this->enumerator();

   loadTypeName();

   if (enumObj.isValid()) {
      // process enum
      QString enumName = enumObj.scope() + "::" + enumObj.name();
      retval = QVariant::nameToType(enumName);

   } else if (! m_typeName.isEmpty()) {
      retval = QVariant::nameToType(m_typeName);
   }

   return retval;
}

bool QMetaProperty::write(QObject *object, const QVariant &value) const
{
   if (! object || ! m_write_able || ! m_writeJar) {
      return false;
   }

   return  m_writeJar->runV(object, value);
}

// internal
void QMetaProperty::setReadMethod(std::type_index returnTypeId,
      QString (*returnTypeFuncPtr)(), JarReadAbstract *jarRead)
{
   if (! jarRead) {
      return;
   }

   m_returnTypeId      = returnTypeId;
   m_returnTypeFuncPtr = returnTypeFuncPtr;

   // method is a ptr to the property READ method, store in a SpiceJarRead
   m_readJar    = jarRead;
   m_read_able  = true;
}

void QMetaProperty::setWriteMethod(JarWriteAbstract *method, const QString &methodName)
{
   if (! method)  {
      return;
   }

   m_writeJar    = method;
   m_write_able  = true;

   // used in hasStdCppSet()
   m_writeMethodName = methodName;
}

void QMetaProperty::setDesignable(JarReadAbstract *method)
{
   if (! method) {
      return;
   }

   m_designJar = method;
}

void QMetaProperty::setScriptable(JarReadAbstract *method)
{
   if (! method) {
      return;
   }

   m_scriptJar = method;
}

void QMetaProperty::setStored(JarReadAbstract *method)
{
   if (! method) {
      return;
   }

   m_storedJar = method;
}

void QMetaProperty::setUser(JarReadAbstract *method)
{
   if (! method) {
      return;
   }

   m_userJar = method;
}