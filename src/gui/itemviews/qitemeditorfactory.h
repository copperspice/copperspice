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

#ifndef QITEMEDITORFACTORY_H
#define QITEMEDITORFACTORY_H

#include <QtCore/qmetaobject.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ITEMVIEWS

class QWidget;

class Q_GUI_EXPORT QItemEditorCreatorBase
{
 public:
   virtual ~QItemEditorCreatorBase() {}

   virtual QWidget *createWidget(QWidget *parent) const = 0;
   virtual QByteArray valuePropertyName() const = 0;
};

template <class T>
class QItemEditorCreator : public QItemEditorCreatorBase
{
 public:
   inline QItemEditorCreator(const QByteArray &valuePropertyName);
   inline QWidget *createWidget(QWidget *parent) const {
      return new T(parent);
   }
   inline QByteArray valuePropertyName() const {
      return propertyName;
   }

 private:
   QByteArray propertyName;
};

template <class T>
class QStandardItemEditorCreator: public QItemEditorCreatorBase
{
 public:
   inline QStandardItemEditorCreator()
      : propertyName(T::staticMetaObject.userProperty().name()) {
   }
   inline QWidget *createWidget(QWidget *parent) const {
      return new T(parent);
   }
   inline QByteArray valuePropertyName() const {
      return propertyName;
   }

 private:
   QByteArray propertyName;
};


template <class T>
Q_INLINE_TEMPLATE QItemEditorCreator<T>::QItemEditorCreator(const QByteArray &avaluePropertyName)
   : propertyName(avaluePropertyName) {}

class Q_GUI_EXPORT QItemEditorFactory
{
 public:
   inline QItemEditorFactory() {}
   virtual ~QItemEditorFactory();

   virtual QWidget *createEditor(QVariant::Type type, QWidget *parent) const;
   virtual QByteArray valuePropertyName(QVariant::Type type) const;

   void registerEditor(QVariant::Type type, QItemEditorCreatorBase *creator);

   static const QItemEditorFactory *defaultFactory();
   static void setDefaultFactory(QItemEditorFactory *factory);

 private:
   QHash<QVariant::Type, QItemEditorCreatorBase *> creatorMap;
};

#endif // QT_NO_ITEMVIEWS

QT_END_NAMESPACE

#endif // QITEMEDITORFACTORY_H
