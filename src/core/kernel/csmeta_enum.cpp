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

#include <QByteArray>
#include <QStringList>

QMetaEnum::QMetaEnum(const char *name, const char *scope, bool isFlag)
{
   m_name  = name;
   m_scope = scope;
   m_flag  = isFlag;
}

QMetaEnum::QMetaEnum()
{
   m_name  = 0;
   m_scope = 0;
}

bool QMetaEnum::isFlag() const
{
   return m_flag;
}

bool QMetaEnum::isValid() const
{
   return (this->name() != 0);
}

const char *QMetaEnum::key(int index) const
{
   if (index < 0 || index >= m_data.size() ) {
      return 0;
   }

   auto elem = m_data.begin();
   elem += index;

   const char *retval = elem.key().constData();

   return retval;
}

int QMetaEnum::keyCount() const
{
   int count = m_data.size();
   return count;
}

int QMetaEnum::keyToValue(const char *key) const
{
   if (! key) {
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

int QMetaEnum::keysToValue(const char *keys) const
{
   int value = 0;

   QByteArray temp = keys;
   QList<QByteArray> tList = temp.split('|');

   for (auto elem = tList.begin(); elem != tList.end(); ++elem) {
      temp  = elem->trimmed();
      value |= keyToValue(temp.constData());
   }

   return value;
}

const char *QMetaEnum::name() const
{
   return m_name;
}

// internal
void QMetaEnum::setData(QMap<QByteArray, int> valueMap)
{
   m_data = valueMap;
}

const char *QMetaEnum::scope() const
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

const char *QMetaEnum::valueToKey(int value) const
{
   const char *retval = 0;

   for (auto elem = m_data.begin(); elem != m_data.end(); ++elem) {

      if (elem.value() == value) {
         retval = elem.key().constData();
         break;
      }
   }

   return retval;
}

QByteArray QMetaEnum::valueToKeys(int value) const
{
   QByteArray keys = "";

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

