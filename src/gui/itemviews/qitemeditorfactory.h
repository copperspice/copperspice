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

#ifndef QITEMEDITORFACTORY_H
#define QITEMEDITORFACTORY_H

#include <qmetaobject.h>
#include <qbytearray.h>
#include <qhash.h>
#include <qvariant.h>

#ifndef QT_NO_ITEMVIEWS

class QWidget;

class Q_GUI_EXPORT QItemEditorCreatorBase
{
 public:
   virtual ~QItemEditorCreatorBase();

   virtual QWidget *createWidget(QWidget *parent) const = 0;
   virtual QString valuePropertyName() const = 0;
};

template <class T>
class QItemEditorCreator : public QItemEditorCreatorBase
{
 public:
   inline explicit QItemEditorCreator(const QString &valuePropertyName);

   inline QWidget *createWidget(QWidget *parent) const override {
      return new T(parent);
   }

   inline QString valuePropertyName() const override {
      return propertyName;
   }

 private:
   QString propertyName;
};

template <class T>
class QStandardItemEditorCreator: public QItemEditorCreatorBase
{
 public:
   inline QStandardItemEditorCreator()
      : propertyName(T::staticMetaObject.userProperty().name()) {
   }

   inline QWidget *createWidget(QWidget *parent) const override {
      return new T(parent);
   }

   inline QString valuePropertyName() const override {
      return propertyName;
   }

 private:
   QString propertyName;
};

template <class T>
QItemEditorCreator<T>::QItemEditorCreator(const QString &valuePropertyName)
   : propertyName(valuePropertyName)
{
}

class Q_GUI_EXPORT QItemEditorFactory
{
 public:
   inline QItemEditorFactory() {}
   virtual ~QItemEditorFactory();

   virtual QWidget *createEditor(QVariant::Type type, QWidget *parent) const;
   virtual QString valuePropertyName(QVariant::Type type) const;

   void registerEditor(QVariant::Type type, QItemEditorCreatorBase *creator);

   static const QItemEditorFactory *defaultFactory();
   static void setDefaultFactory(QItemEditorFactory *factory);

 private:
   QHash<QVariant::Type, QItemEditorCreatorBase *> creatorMap;
};

#endif // QT_NO_ITEMVIEWS



#endif // QITEMEDITORFACTORY_H
