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

#include "private/qdeclarativelistaccessor_p.h"

#include <qdeclarativemetatype_p.h>

#include <QtCore/qstringlist.h>
#include <QtCore/qdebug.h>

// ### Remove me
#include <qdeclarativeengine_p.h>

QT_BEGIN_NAMESPACE

QDeclarativeListAccessor::QDeclarativeListAccessor()
   : m_type(Invalid)
{
}

QDeclarativeListAccessor::~QDeclarativeListAccessor()
{
}

QVariant QDeclarativeListAccessor::list() const
{
   return d;
}

void QDeclarativeListAccessor::setList(const QVariant &v, QDeclarativeEngine *engine)
{
   d = v;

   QDeclarativeEnginePrivate *enginePrivate = engine ? QDeclarativeEnginePrivate::get(engine) : 0;

   if (!d.isValid()) {
      m_type = Invalid;
   } else if (d.userType() == QVariant::StringList) {
      m_type = StringList;
   } else if (d.userType() == QMetaType::QVariantList) {
      m_type = VariantList;
   } else if (d.canConvert(QVariant::Int)) {
      m_type = Integer;
   } else if ((!enginePrivate && QDeclarativeMetaType::isQObject(d.userType())) ||
              (enginePrivate && enginePrivate->isQObject(d.userType()))) {
      QObject *data = enginePrivate ? enginePrivate->toQObject(v) : QDeclarativeMetaType::toQObject(v);
      d = QVariant::fromValue(data);
      m_type = Instance;
   } else if (d.userType() == qMetaTypeId<QDeclarativeListReference>()) {
      m_type = ListProperty;
   } else {
      m_type = Instance;
   }
}

int QDeclarativeListAccessor::count() const
{
   switch (m_type) {
      case StringList:
         return qvariant_cast<QStringList>(d).count();
      case VariantList:
         return qvariant_cast<QVariantList>(d).count();
      case ListProperty:
         return ((QDeclarativeListReference *)d.constData())->count();
      case Instance:
         return 1;
      case Integer:
         return d.toInt();
      default:
      case Invalid:
         return 0;
   }
}

QVariant QDeclarativeListAccessor::at(int idx) const
{
   Q_ASSERT(idx >= 0 && idx < count());
   switch (m_type) {
      case StringList:
         return QVariant::fromValue(qvariant_cast<QStringList>(d).at(idx));
      case VariantList:
         return qvariant_cast<QVariantList>(d).at(idx);
      case ListProperty:
         return QVariant::fromValue(((QDeclarativeListReference *)d.constData())->at(idx));
      case Instance:
         return d;
      case Integer:
         return QVariant(idx);
      default:
      case Invalid:
         return QVariant();
   }
}

bool QDeclarativeListAccessor::isValid() const
{
   return m_type != Invalid;
}

QT_END_NAMESPACE
