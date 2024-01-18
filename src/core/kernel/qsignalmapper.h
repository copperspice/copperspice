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

#ifndef QSIGNALMAPPER_H
#define QSIGNALMAPPER_H

#include <qobject.h>
#include <qscopedpointer.h>

#ifndef QT_NO_SIGNALMAPPER
class QSignalMapperPrivate;

class Q_CORE_EXPORT QSignalMapper : public QObject
{
   CORE_CS_OBJECT(QSignalMapper)
   Q_DECLARE_PRIVATE(QSignalMapper)

 public:
   explicit QSignalMapper(QObject *parent = nullptr);

   QSignalMapper(const QSignalMapper &) = delete;
   QSignalMapper &operator=(const QSignalMapper &) = delete;

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

   CORE_CS_SIGNAL_1(Public, void mapped(int index))
   CORE_CS_SIGNAL_OVERLOAD(mapped, (int), index)

   CORE_CS_SIGNAL_1(Public, void mapped(const QString &text))
   CORE_CS_SIGNAL_OVERLOAD(mapped, (const QString &), text)

   CORE_CS_SIGNAL_1(Public, void mapped(QWidget *widget))
   CORE_CS_SIGNAL_OVERLOAD(mapped, (QWidget *), widget)

   CORE_CS_SIGNAL_1(Public, void mapped(QObject *object))
   CORE_CS_SIGNAL_OVERLOAD(mapped, (QObject *), object)

   CORE_CS_SLOT_1(Public, void map())
   CORE_CS_SLOT_OVERLOAD(map, ())

   CORE_CS_SLOT_1(Public, void map(QObject *sender))
   CORE_CS_SLOT_OVERLOAD(map, (QObject *))

 protected:
   QScopedPointer<QSignalMapperPrivate> d_ptr;

 private:
   CORE_CS_SLOT_1(Private, void _q_senderDestroyed())
   CORE_CS_SLOT_2(_q_senderDestroyed)
};

#endif // QT_NO_SIGNALMAPPER

#endif
