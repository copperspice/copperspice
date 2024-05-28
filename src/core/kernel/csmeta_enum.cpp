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
#include <qstringlist.h>
#include <qstringparser.h>

QMetaEnum::QMetaEnum(const QString &name, const QString &scope, bool isFlag)
   : m_name(name), m_scope(scope), m_flag(isFlag)
{
}

QMetaEnum::QMetaEnum()
{
   m_name  = QString();
   m_scope = QString();
   m_flag  = false;
}

bool QMetaEnum::isFlag() const
{
   return m_flag;
}

bool QMetaEnum::isValid() const
{
   return ! m_name.isEmpty();
}

const QString &QMetaEnum::key(int index) const
{
   if (index < 0 || index >= m_data.size() ) {

      if (m_data.isEmpty()) {
         qWarning("QMetaEnum::key() Enum %s may not be registered", csPrintable(m_name));
      }

      static QString retval;

      return retval;
   }

   auto elem = m_data.begin();
   elem += index;

   return elem.key();
}

int QMetaEnum::keyCount() const
{
   int count = m_data.size();
   return count;
}

int QMetaEnum::keyToValue(const QString &key) const
{
   if (key.isEmpty()) {
      return -1;
   }

   int retval;
   auto elem = m_data.find(key);

   if (elem == m_data.end()) {
      retval = -1;

   } else {
      retval = elem.value();

   }

   return retval;
}

int QMetaEnum::keysToValue(const QString &keys) const
{
   int value = 0;
   QList<QString> list = keys.split('|');

   for (auto elem : list) {
      value |= keyToValue(elem.trimmed());
   }

   return value;
}

const QString &QMetaEnum::name() const
{
   return m_name;
}

// internal
void QMetaEnum::setData(QMap<QString, int> valueMap)
{
   m_data = valueMap;
}

const QString &QMetaEnum::scope() const
{
   return m_scope;
}

int QMetaEnum::value(int index) const
{
   if (index < 0 || index >= m_data.size() ) {
      return -1;
   }

   auto elem = m_data.begin();
   elem += index;

   return elem.value();
}

const QString &QMetaEnum::valueToKey(int value) const
{
   for (auto elem = m_data.begin(); elem != m_data.end(); ++elem) {

      if (elem.value() == value) {
         return elem.key();
      }
   }

   static const QString retval;

   return retval;
}

QString QMetaEnum::valueToKeys(int value) const
{
   QString keys;

   for (auto elem = m_data.begin(); elem != m_data.end(); ++elem) {

      if (elem.value() & value) {

         if (keys.isEmpty()) {
            keys = elem.key();

         } else  {
            keys = keys + '|' + elem.key();

         }
      }
   }

   return keys;
}
