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

#include <qsignalmapper.h>

#ifndef QT_NO_SIGNALMAPPER

#include <qhash.h>

class QSignalMapperPrivate
{
   Q_DECLARE_PUBLIC(QSignalMapper)

 public:
   virtual ~QSignalMapperPrivate()
   { }

   void _q_senderDestroyed() {
      Q_Q(QSignalMapper);
      q->removeMappings(q->sender());
   }

   QHash<QObject *, int> intHash;
   QHash<QObject *, QString> stringHash;
   QHash<QObject *, QWidget *> widgetHash;
   QHash<QObject *, QObject *> objectHash;

 protected:
   QSignalMapper *q_ptr;
};

void QSignalMapper::_q_senderDestroyed()
{
   Q_D(QSignalMapper);
   d->_q_senderDestroyed();
}

QSignalMapper::QSignalMapper(QObject *parent)
   : QObject(parent), d_ptr(new QSignalMapperPrivate)
{
   d_ptr->q_ptr = this;
}

QSignalMapper::~QSignalMapper()
{
}

void QSignalMapper::setMapping(QObject *sender, int id)
{
   Q_D(QSignalMapper);
   d->intHash.insert(sender, id);
   connect(sender, &QObject::destroyed, this, &QSignalMapper::_q_senderDestroyed);
}

void QSignalMapper::setMapping(QObject *sender, const QString &text)
{
   Q_D(QSignalMapper);
   d->stringHash.insert(sender, text);
   connect(sender, &QObject::destroyed, this, &QSignalMapper::_q_senderDestroyed);
}

void QSignalMapper::setMapping(QObject *sender, QWidget *widget)
{
   Q_D(QSignalMapper);
   d->widgetHash.insert(sender, widget);
   connect(sender, &QObject::destroyed, this, &QSignalMapper::_q_senderDestroyed);
}

void QSignalMapper::setMapping(QObject *sender, QObject *object)
{
   Q_D(QSignalMapper);
   d->objectHash.insert(sender, object);
   connect(sender, &QObject::destroyed, this, &QSignalMapper::_q_senderDestroyed);
}

QObject *QSignalMapper::mapping(int id) const
{
   Q_D(const QSignalMapper);
   return d->intHash.key(id);
}

QObject *QSignalMapper::mapping(const QString &id) const
{
   Q_D(const QSignalMapper);
   return d->stringHash.key(id);
}

QObject *QSignalMapper::mapping(QWidget *widget) const
{
   Q_D(const QSignalMapper);
   return d->widgetHash.key(widget);
}

QObject *QSignalMapper::mapping(QObject *object) const
{
   Q_D(const QSignalMapper);
   return d->objectHash.key(object);
}

void QSignalMapper::removeMappings(QObject *sender)
{
   Q_D(QSignalMapper);

   d->intHash.remove(sender);
   d->stringHash.remove(sender);
   d->widgetHash.remove(sender);
   d->objectHash.remove(sender);
}

void QSignalMapper::map()
{
   map(sender());
}

void QSignalMapper::map(QObject *sender)
{
   Q_D(QSignalMapper);

   if (d->intHash.contains(sender)) {
      emit mapped(d->intHash.value(sender));
   }

   if (d->stringHash.contains(sender)) {
      emit mapped(d->stringHash.value(sender));
   }

   if (d->widgetHash.contains(sender)) {
      emit mapped(d->widgetHash.value(sender));
   }

   if (d->objectHash.contains(sender)) {
      emit mapped(d->objectHash.value(sender));
   }
}

#endif
