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
#include <qstringlist.h>
#include <qstringparser.h>

QMetaEnum::QMetaEnum(const QString8 &name, const QString8 &scope, bool isFlag)
   : m_name(name), m_scope(scope), m_flag(isFlag)
{
}

QMetaEnum::QMetaEnum()
{
   m_name  = QString8();
   m_scope = QString8();
}

bool QMetaEnum::isFlag() const
{
   return m_flag;
}

bool QMetaEnum::isValid() const
{
   return m_name.isEmpty();
}

const QString8 &QMetaEnum::key(int index) const
{
   if (index < 0 || index >= m_data.size() ) {
      static QString8 retval;
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

int QMetaEnum::keyToValue(const QString8 &key) const
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

int QMetaEnum::keysToValue(const QString8 &keys) const
{
   int value = 0;
   QList<QString8> list = keys.split('|');

   for (auto elem : list) {
      value |= keyToValue(elem.trimmed());
   }

   return value;
}

const QString8 &QMetaEnum::name() const
{
   return m_name;
}

// internal
void QMetaEnum::setData(QMap<QString8, int> valueMap)
{
   m_data = valueMap;
}

const QString8 &QMetaEnum::scope() const
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

const QString8 &QMetaEnum::valueToKey(int value) const
{
   for (auto elem = m_data.begin(); elem != m_data.end(); ++elem) {

      if (elem.value() == value) {
         return elem.key();
      }
   }

   static const QString8 retval;

   return retval;
}

QString8 QMetaEnum::valueToKeys(int value) const
{
   QString8 keys;

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

