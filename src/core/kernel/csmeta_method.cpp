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

QMetaMethod::QMetaMethod(const char *typeName, const QByteArray &signature, QList<QByteArray> paramNames,
                         QMetaMethod::Access access, QMetaMethod::MethodType methodType, 
                         QMetaMethod::Attributes attributes, QMetaObject *obj)
{
   m_typeName   = typeName;
   m_signature  = signature;
   m_paramNames = paramNames;
   m_access     = access;
   m_methodType = methodType;
   m_attributes = attributes;
   m_metaObject = obj;

   m_bento     = nullptr;
   m_tag       = "";
   m_revision  = 0;
}

QMetaMethod::QMetaMethod()
{
   m_typeName   = "";
   // m_signature
   // m_paramNames
   m_access     = Private;
   m_methodType = Method;
   m_attributes = Attributes(0);
   m_metaObject = nullptr;

   m_bento = nullptr;
   m_tag   = "";
}

QMetaMethod::Access QMetaMethod::access() const
{
   return m_access;
}

QMetaMethod::Attributes QMetaMethod::attributes() const
{
   return m_attributes;
}

bool QMetaMethod::compare(const CsSignal::Internal::BentoAbstract &method) const
{
   bool ok = false;

   if (m_bento) {

      if (*m_bento == method) {
         ok = true;
      }
   }

   return ok;
}

const CSBentoAbstract *QMetaMethod::getBentoBox() const
{
   return m_bento;
}

const QMetaObject *QMetaMethod::getMetaObject() const
{
   return m_metaObject;
}

int QMetaMethod::methodIndex() const
{
   if (! m_metaObject) {
      return -1;
   }

   const char *method = m_signature.constData();

   return m_metaObject->indexOfMethod(method);
}

QMetaMethod::MethodType QMetaMethod::methodType() const
{
   return m_methodType;
}

QByteArray QMetaMethod::name() const
{
   QByteArray retval = m_signature;

   int pos = m_signature.indexOf("(");
   retval  = m_signature.left(pos);

   return retval;
}

int QMetaMethod::parameterCount() const
{
   return m_paramNames.size();
}

int QMetaMethod::parameterType(int index) const
{
   QList<QByteArray> types = parameterTypes();
   QByteArray typeName = types[index];

   int retval = QMetaType::type(typeName.constData());

   return retval;
}

QList<QByteArray> QMetaMethod::parameterNames() const
{
   return m_paramNames;
}

QList<QByteArray> QMetaMethod::parameterTypes() const
{
   QList<QByteArray> retval;

   const char *temp = m_signature.constData();
   char letter;

   while (*temp)  {
      letter = *temp;

      if (letter == '(') {
         break;
      }

      ++temp;
   }

   ++temp;

   QByteArray word;
   int angleLevel    = 0;
   int bracketLevel  = 0;
   int parenLevel    = 0;

   while (*temp)  {
      letter = *temp;

      // A
      if (letter == '<') {
         ++angleLevel;

      } else if (letter == '>')  {
         --angleLevel;

      } else if (letter == '[')  {
         ++bracketLevel;

      } else if (letter == ']')  {
         --bracketLevel;

      } else if (letter == '(')  {
         ++parenLevel;

      } else if (letter == ')')  {
         --parenLevel;

      }

      // B
      if ((angleLevel == 0 && bracketLevel == 0 && parenLevel == 0) && letter == ',') {
         retval.append(word);
         word = "";

      } else {
         word += letter;

      }

      ++temp;
   }

   // remove last letter
   word.chop(1);

   if (! word.isEmpty() ) {
      retval.append(word);
   }

   return retval;
}

int QMetaMethod::revision() const
{
   return m_revision;
}

void QMetaMethod::setBentoBox(const CSBentoAbstract *method)
{
   m_bento = method;
}

QByteArray QMetaMethod::methodSignature() const
{
   return m_signature;
}

void QMetaMethod::setRevision(int revision)
{
   m_revision = revision;
}

void QMetaMethod::setTag(const char *data)
{
   m_tag = data;
}

const char *QMetaMethod::tag() const
{
   return m_tag;
}

const char *QMetaMethod::typeName() const
{
   return m_typeName;
}

