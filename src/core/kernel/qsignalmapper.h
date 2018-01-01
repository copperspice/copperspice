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

#ifndef QSIGNALMAPPER_H
#define QSIGNALMAPPER_H

#include <QtCore/qobject.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SIGNALMAPPER
class QSignalMapperPrivate;

class Q_CORE_EXPORT QSignalMapper : public QObject
{
   CORE_CS_OBJECT(QSignalMapper)
   Q_DECLARE_PRIVATE(QSignalMapper)

 public:
   explicit QSignalMapper(QObject *parent = nullptr);
   ~QSignalMapper();

   void setMapping(QObject *sender, int id);
   void setMapping(QObject *sender, const QString &text);
   void setMapping(QObject *sender, QWidget *widget);
   void setMapping(QObject *sender, QObject *object);
   void removeMappings(QObject *sender);

   QObject *mapping(int id) const;
   QObject *mapping(const QString &text) const;
   QObject *mapping(QWidget *widget) const;
   QObject *mapping(QObject *object) const;

   CORE_CS_SIGNAL_1(Public, void mapped(int un_named_arg1))
   CORE_CS_SIGNAL_OVERLOAD(mapped, (int), un_named_arg1)

   CORE_CS_SIGNAL_1(Public, void mapped(const QString &un_named_arg1))
   CORE_CS_SIGNAL_OVERLOAD(mapped, (const QString &), un_named_arg1)

   CORE_CS_SIGNAL_1(Public, void mapped(QWidget *un_named_arg1))
   CORE_CS_SIGNAL_OVERLOAD(mapped, (QWidget *), un_named_arg1)

   CORE_CS_SIGNAL_1(Public, void mapped(QObject *un_named_arg1))
   CORE_CS_SIGNAL_OVERLOAD(mapped, (QObject *), un_named_arg1)

   CORE_CS_SLOT_1(Public, void map())
   CORE_CS_SLOT_OVERLOAD(map, ())

   CORE_CS_SLOT_1(Public, void map(QObject *sender))
   CORE_CS_SLOT_OVERLOAD(map, (QObject *))

 protected:
   QScopedPointer<QSignalMapperPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QSignalMapper)

   CORE_CS_SLOT_1(Private, void _q_senderDestroyed())
   CORE_CS_SLOT_2(_q_senderDestroyed)
};

#endif // QT_NO_SIGNALMAPPER

QT_END_NAMESPACE

#endif // QSIGNALMAPPER_H
