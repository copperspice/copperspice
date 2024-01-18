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

#ifndef QDECLARATIVEWATCHER_P_H
#define QDECLARATIVEWATCHER_P_H

#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qpair.h>
#include <QtCore/qhash.h>
#include <QtCore/qset.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QDeclarativeWatchProxy;
class QDeclarativeExpression;
class QDeclarativeContext;
class QMetaProperty;

class QDeclarativeWatcher : public QObject
{
   DECL_CS_OBJECT(QDeclarativeWatcher)
 public:
   QDeclarativeWatcher(QObject * = 0);

   bool addWatch(int id, quint32 objectId);
   bool addWatch(int id, quint32 objectId, const QByteArray &property);
   bool addWatch(int id, quint32 objectId, const QString &expr);

   void removeWatch(int id);

 public:
   DECL_CS_SIGNAL_1(Public, void propertyChanged(int id, int objectId, const QMetaProperty &property, const QVariant &value))
   DECL_CS_SIGNAL_2(propertyChanged, id, objectId, property, value)

 private:
   friend class QDeclarativeWatchProxy;
   void addPropertyWatch(int id, QObject *object, quint32 objectId, const QMetaProperty &property);

   QHash<int, QList<QPointer<QDeclarativeWatchProxy> > > m_proxies;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEWATCHER_P_H
