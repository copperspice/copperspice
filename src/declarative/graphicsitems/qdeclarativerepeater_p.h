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

#ifndef QDECLARATIVEREPEATER_P_H
#define QDECLARATIVEREPEATER_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativeRepeaterPrivate;
class QDeclarativeRepeater : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativeRepeater)

   DECL_CS_PROPERTY_READ(model, model)
   DECL_CS_PROPERTY_WRITE(model, setModel)
   DECL_CS_PROPERTY_NOTIFY(model, modelChanged)
   DECL_CS_PROPERTY_READ(*delegate, delegate)
   DECL_CS_PROPERTY_WRITE(*delegate, setDelegate)
   DECL_CS_PROPERTY_NOTIFY(*delegate, delegateChanged)
   DECL_CS_PROPERTY_READ(count, count)
   DECL_CS_PROPERTY_NOTIFY(count, countChanged)

   DECL_CS_CLASSINFO("DefaultProperty", "delegate")

 public:
   QDeclarativeRepeater(QDeclarativeItem *parent = 0);
   virtual ~QDeclarativeRepeater();

   QVariant model() const;
   void setModel(const QVariant &);

   QDeclarativeComponent *delegate() const;
   void setDelegate(QDeclarativeComponent *);

   int count() const;

   DECL_CS_SIGNAL_1(Public, void modelChanged())
   DECL_CS_SIGNAL_2(modelChanged)

   DECL_CS_SIGNAL_1(Public, void delegateChanged())
   DECL_CS_SIGNAL_2(delegateChanged)

   DECL_CS_SIGNAL_1(Public, void countChanged())
   DECL_CS_SIGNAL_2(countChanged)

   DECL_CS_SIGNAL_1(Public, void itemAdded(int index, QDeclarativeItem *item))
   DECL_CS_SIGNAL_2(itemAdded, index, item)
   DECL_CS_REVISION(itemAdded, 1)

   DECL_CS_SIGNAL_1(Public, void itemRemoved(int index, QDeclarativeItem *item))
   DECL_CS_SIGNAL_2(itemRemoved, index, item)
   DECL_CS_REVISION(itemRemoved, 1)

   DECL_CS_INVOKABLE_METHOD_1(Public, QDeclarativeItem *itemAt(int index) const)
   DECL_CS_INVOKABLE_METHOD_2(itemAt)
   DECL_CS_REVISION(itemAt, 1)

 protected:
   virtual void componentComplete();
   QVariant itemChange(GraphicsItemChange change, const QVariant &value);

 private :
   void clear();
   void regenerate();

   DECL_CS_SLOT_1(Private, void itemsInserted(int un_named_arg1, int un_named_arg2))
   DECL_CS_SLOT_2(itemsInserted)

   DECL_CS_SLOT_1(Private, void itemsRemoved(int un_named_arg1, int un_named_arg2))
   DECL_CS_SLOT_2(itemsRemoved)

   DECL_CS_SLOT_1(Private, void itemsMoved(int un_named_arg1, int un_named_arg2, int un_named_arg3))
   DECL_CS_SLOT_2(itemsMoved)

   DECL_CS_SLOT_1(Private, void modelReset())
   DECL_CS_SLOT_2(modelReset)

   Q_DISABLE_COPY(QDeclarativeRepeater)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeRepeater)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeRepeater)

#endif // QDECLARATIVEREPEATER_H
