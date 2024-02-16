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

QMetaMethod::QMetaMethod(const QString &typeName, const QString &signature, std::vector<QString> paramNames,
      QMetaMethod::Access access, QMetaMethod::MethodType methodType,
      QMetaMethod::Attributes attributes, QMetaObject *obj)
   : m_typeName(typeName), m_signature(signature), m_paramNames(paramNames.begin(), paramNames.end()),
     m_access(access), m_methodType(methodType), m_attributes(attributes), m_metaObject(obj)
{
   m_bento     = nullptr;
   m_tag       = QString();
   m_revision  = 0;
}

QMetaMethod::QMetaMethod()
{
   m_typeName   = QString();
   // m_signature
   // m_paramNames
   m_access     = Private;
   m_methodType = Method;
   m_attributes = Attributes(0);
   m_metaObject = nullptr;

   m_bento      = nullptr;
   m_tag        = QString();
   m_revision   = 0;
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

bool QMetaMethod::isValid() const
{
   if (m_metaObject == nullptr) {
      return false;
   }

   return true;
}

int QMetaMethod::methodIndex() const
{
   if (m_metaObject == nullptr) {
      return -1;
   }

   return m_metaObject->indexOfMethod(m_signature);
}

QMetaMethod::MethodType QMetaMethod::methodType() const
{
   return m_methodType;
}

const QString QMetaMethod::name() const
{
   QString retval = m_signature;

   int pos = m_signature.indexOf("(");
   retval  = m_signature.left(pos);

   return retval;
}

int QMetaMethod::parameterCount() const
{
   return m_paramNames.size();
}

uint QMetaMethod::parameterType(int index) const
{
   QList<QString> types = parameterTypes();
   QString typeName     = types[index];

   uint retval = QVariant::nameToType(typeName);

   return retval;
}

QList<QString> QMetaMethod::parameterNames() const
{
   return m_paramNames;
}

QList<QString> QMetaMethod::parameterTypes() const
{
   QList<QString> retval;

   QString::const_iterator iter = m_signature.begin();
   QChar32 letter;

   while (iter != m_signature.end())  {
      letter = *iter;

      if (letter == '(') {
         break;
      }

      ++iter;
   }

   ++iter;

   QString word;

   int angleLevel    = 0;
   int bracketLevel  = 0;
   int parenLevel    = 0;

   while (iter != m_signature.end())  {
      letter = *iter;

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

      ++iter;
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

const QString &QMetaMethod::methodSignature() const
{
   return m_signature;
}

void QMetaMethod::setRevision(int revision)
{
   m_revision = revision;
}

void QMetaMethod::setTag(const QString &data)
{
   m_tag = data;
}

const QString &QMetaMethod::tag() const
{
   return m_tag;
}

const QString &QMetaMethod::typeName() const
{
   return m_typeName;
}
