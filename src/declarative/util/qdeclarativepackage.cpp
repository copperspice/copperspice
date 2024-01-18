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

#include "private/qdeclarativepackage_p.h"
#include <private/qdeclarativeguard_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass Package QDeclarativePackage
    \ingroup qml-working-with-data
    \brief Package provides a collection of named items.

    The Package class is used in conjunction with
    VisualDataModel to enable delegates with a shared context
    to be provided to multiple views.

    Any item within a Package may be assigned a name via the
    \l{Package::name}{Package.name} attached property.

    The example below creates a Package containing two named items;
    \e list and \e grid.  The third element in the package (the \l Rectangle) is parented to whichever
    delegate it should appear in.  This allows an item to move
    between views.

    \snippet examples/declarative/modelviews/package/Delegate.qml 0

    These named items are used as the delegates by the two views who
    reference the special \l{VisualDataModel::parts} property to select
    a model which provides the chosen delegate.

    \snippet examples/declarative/modelviews/package/view.qml 0

    \sa {declarative/modelviews/package}{Package example}, {demos/declarative/photoviewer}{Photo Viewer demo}, QtDeclarative
*/

/*!
    \qmlattachedproperty string Package::name
    This attached property holds the name of an item within a Package.
*/


class QDeclarativePackagePrivate
{
 public:
   QDeclarativePackagePrivate() {}

   struct DataGuard : public QDeclarativeGuard<QObject> {
      DataGuard(QObject *obj, QList<DataGuard> *l) : list(l) {
         (QDeclarativeGuard<QObject> &)*this = obj;
      }
      QList<DataGuard> *list;
      void objectDestroyed(QObject *) {
         // we assume priv will always be destroyed after objectDestroyed calls
         list->removeOne(*this);
      }
   };

   QList<DataGuard> dataList;
   static void data_append(QDeclarativeListProperty<QObject> *prop, QObject *o) {
      QList<DataGuard> *list = static_cast<QList<DataGuard> *>(prop->data);
      list->append(DataGuard(o, list));
   }
   static void data_clear(QDeclarativeListProperty<QObject> *prop) {
      QList<DataGuard> *list = static_cast<QList<DataGuard> *>(prop->data);
      list->clear();
   }
   static QObject *data_at(QDeclarativeListProperty<QObject> *prop, int index) {
      QList<DataGuard> *list = static_cast<QList<DataGuard> *>(prop->data);
      return list->at(index);
   }
   static int data_count(QDeclarativeListProperty<QObject> *prop) {
      QList<DataGuard> *list = static_cast<QList<DataGuard> *>(prop->data);
      return list->count();
   }
};

QHash<QObject *, QDeclarativePackageAttached *> QDeclarativePackageAttached::attached;

QDeclarativePackageAttached::QDeclarativePackageAttached(QObject *parent)
   : QObject(parent)
{
   attached.insert(parent, this);
}

QDeclarativePackageAttached::~QDeclarativePackageAttached()
{
   attached.remove(parent());
}

QString QDeclarativePackageAttached::name() const
{
   return _name;
}

void QDeclarativePackageAttached::setName(const QString &n)
{
   _name = n;
}

QDeclarativePackage::QDeclarativePackage(QObject *parent)
   : QObject(*(new QDeclarativePackagePrivate), parent)
{
}

QDeclarativePackage::~QDeclarativePackage()
{
   Q_D(QDeclarativePackage);
   for (int ii = 0; ii < d->dataList.count(); ++ii) {
      QObject *obj = d->dataList.at(ii);
      obj->setParent(this);
   }
}

QDeclarativeListProperty<QObject> QDeclarativePackage::data()
{
   Q_D(QDeclarativePackage);
   return QDeclarativeListProperty<QObject>(this, &d->dataList, QDeclarativePackagePrivate::data_append,
          QDeclarativePackagePrivate::data_count,
          QDeclarativePackagePrivate::data_at,
          QDeclarativePackagePrivate::data_clear);
}

bool QDeclarativePackage::hasPart(const QString &name)
{
   Q_D(QDeclarativePackage);
   for (int ii = 0; ii < d->dataList.count(); ++ii) {
      QObject *obj = d->dataList.at(ii);
      QDeclarativePackageAttached *a = QDeclarativePackageAttached::attached.value(obj);
      if (a && a->name() == name) {
         return true;
      }
   }
   return false;
}

QObject *QDeclarativePackage::part(const QString &name)
{
   Q_D(QDeclarativePackage);
   if (name.isEmpty() && !d->dataList.isEmpty()) {
      return d->dataList.at(0);
   }

   for (int ii = 0; ii < d->dataList.count(); ++ii) {
      QObject *obj = d->dataList.at(ii);
      QDeclarativePackageAttached *a = QDeclarativePackageAttached::attached.value(obj);
      if (a && a->name() == name) {
         return obj;
      }
   }

   if (name == QLatin1String("default") && !d->dataList.isEmpty()) {
      return d->dataList.at(0);
   }

   return 0;
}

QDeclarativePackageAttached *QDeclarativePackage::qmlAttachedProperties(QObject *o)
{
   return new QDeclarativePackageAttached(o);
}



QT_END_NAMESPACE
