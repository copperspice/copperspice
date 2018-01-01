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

#include <qobject.h>
#include <csmeta.h>
#include <qmetaobject.h>

QMetaProperty::QMetaProperty(const char *name, QMetaObject *obj)
{
   m_name         = name;
   m_metaObject   = obj;
   m_typeName     = 0;

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
}

QMetaEnum QMetaProperty::enumerator() const
{
   QMetaEnum enumObj;

   int index = m_metaObject->indexOfEnumerator(m_typeName);

   if (index > 0) {
      enumObj = m_metaObject->enumerator(index);
   }

   return enumObj;
}

bool QMetaProperty::hasNotifySignal() const
{
   return m_notify_able;
}

bool QMetaProperty::hasStdCppSet() const
{
   //  internal & undocumented, used to bypass issues in Qt
   return false;
}

bool QMetaProperty::isConstant() const
{
   return m_constant;
}

bool QMetaProperty::isDesignable(const QObject *object) const
{
   bool retval;

   if (! m_designJar) {
      return false;
   }

   retval = m_designJar->run<bool>(object);

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

   if (! m_scriptJar) {
      return false;
   }

   retval = m_scriptJar->run<bool>(object);

   return retval;
}

bool QMetaProperty::isStored(const QObject *object) const
{
   bool retval = m_storedJar;

   if (! m_storedJar) {
      return false;
   }

   retval = m_storedJar->run<bool>(object);

   return retval;
}

bool QMetaProperty::isUser(const QObject *object) const
{
   bool retval = false;

   if (! m_userJar) {
      return false;
   }

   retval = m_userJar->run<bool>(object);

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

const char *QMetaProperty::name() const
{
   return m_name;
}

QMetaMethod QMetaProperty::notifySignal() const
{
   int id = notifySignalIndex();

   if (id == -1) {
      return QMetaMethod("", "", QList<QByteArray>(), QMetaMethod::Private, QMetaMethod::Slot,
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
   if (! m_metaObject) {
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
void QMetaProperty::setTypeName(const char *typeName)
{
   m_typeName = typeName;
}

QVariant::Type QMetaProperty::type() const
{
   QVariant::Type retval = QVariant::UserType;
   QMetaEnum enumObj = this->enumerator();

   if (enumObj.isValid()) {
      // process enum
      QByteArray enumName = QByteArray(enumObj.scope()) + "::" + enumObj.name();

      int enumMetaTypeId = QMetaType::type(enumName.constData());

      if (enumMetaTypeId == 0) {
         retval = QVariant::Int;
      }

   } else if (m_typeName) {
      retval = QVariant::nameToType(m_typeName);

   }

   return retval;
}

const char *QMetaProperty::typeName() const
{
   return m_typeName;
}

int QMetaProperty::userType() const
{
   int retval = QVariant::UserType;
   QMetaEnum enumObj = this->enumerator();

   if (enumObj.isValid()) {
      // process enum
      QByteArray enumName = QByteArray(enumObj.scope()) + "::" + enumObj.name();
      retval = QMetaType::type(enumName.constData());

   } else if (m_typeName) {
      retval = QVariant::nameToType(m_typeName);

      if (retval == QVariant::UserType) {
         retval = QMetaType::type(m_typeName);
      }
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

// ** internal
void QMetaProperty::setReadMethod(const char *typeName, JarReadAbstract *jarRead)
{
   if (! jarRead) {
      return;
   }

   // typeName is the return type
   this->setTypeName(typeName);

   // method is a ptr to the property READ method,store in a SpiceJarRead
   m_readJar    = jarRead;
   m_read_able  = true;
}

void QMetaProperty::setWriteMethod(JarWriteAbstract *method)
{
   if (! method)  {
      return;
   }

   m_writeJar   = method;
   m_write_able = true;
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