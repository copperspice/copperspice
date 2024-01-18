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

#ifndef QLISTMODELINTERFACE_P_H
#define QLISTMODELINTERFACE_P_H

#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class Q_DECLARATIVE_PRIVATE_EXPORT QListModelInterface : public QObject
{
   DECL_CS_OBJECT(QListModelInterface)

 public:
   QListModelInterface(QObject *parent = nullptr) : QObject(parent) {}
   virtual ~QListModelInterface() {}

   virtual int count() const = 0;
   virtual QVariant data(int index, int role) const = 0;

   virtual QList<int> roles() const = 0;
   virtual QString toString(int role) const = 0;

   DECL_CS_SIGNAL_1(Public, void itemsInserted(int index, int count))
   DECL_CS_SIGNAL_2(itemsInserted, index, count)

   DECL_CS_SIGNAL_1(Public, void itemsRemoved(int index, int count))
   DECL_CS_SIGNAL_2(itemsRemoved, index, count)

   DECL_CS_SIGNAL_1(Public, void itemsMoved(int from, int to, int count))
   DECL_CS_SIGNAL_2(itemsMoved, from, to, count)

   DECL_CS_SIGNAL_1(Public, void itemsChanged(int index, int count, const QList <int> &roles))
   DECL_CS_SIGNAL_2(itemsChanged, index, count, roles)

 protected:
   QListModelInterface(QObject *parent)
      : QObject(parent) {}
};

QT_END_NAMESPACE

#endif //QTREEMODELINTERFACE_H
