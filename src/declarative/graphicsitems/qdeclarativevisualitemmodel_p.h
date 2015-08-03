/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDECLARATIVEVisualItemModel_P_H
#define QDECLARATIVEVisualItemModel_P_H

#include <qdeclarative.h>
#include <QtCore/qobject.h>
#include <QtCore/qabstractitemmodel.h>

Q_DECLARE_METATYPE(QModelIndex)

QT_BEGIN_NAMESPACE

class QDeclarativeItem;
class QDeclarativeComponent;
class QDeclarativePackage;
class QDeclarativeVisualDataModelPrivate;

class QDeclarativeVisualModel : public QObject
{
   DECL_CS_OBJECT(QDeclarativeVisualModel)

   CS_PROPERTY_READ(count, count)
   CS_PROPERTY_NOTIFY(count, countChanged)

 public:
   virtual ~QDeclarativeVisualModel() {}

   enum ReleaseFlag { Referenced = 0x01, Destroyed = 0x02 };
   using ReleaseFlags = QFlags<ReleaseFlag>;

   virtual int count() const = 0;
   virtual bool isValid() const = 0;
   virtual QDeclarativeItem *item(int index, bool complete = true) = 0;
   virtual ReleaseFlags release(QDeclarativeItem *item) = 0;
   virtual bool completePending() const = 0;
   virtual void completeItem() = 0;
   virtual QString stringValue(int, const QString &) = 0;
   virtual void setWatchedRoles(QList<QByteArray> roles) = 0;

   virtual int indexOf(QDeclarativeItem *item, QObject *objectContext) const = 0;

   CS_SIGNAL_1(Public, void countChanged())
   CS_SIGNAL_2(countChanged)
   CS_SIGNAL_1(Public, void itemsInserted(int index, int count))
   CS_SIGNAL_2(itemsInserted, index, count)
   CS_SIGNAL_1(Public, void itemsRemoved(int index, int count))
   CS_SIGNAL_2(itemsRemoved, index, count)
   CS_SIGNAL_1(Public, void itemsMoved(int from, int to, int count))
   CS_SIGNAL_2(itemsMoved, from, to, count)
   CS_SIGNAL_1(Public, void itemsChanged(int index, int count))
   CS_SIGNAL_2(itemsChanged, index, count)
   CS_SIGNAL_1(Public, void modelReset())
   CS_SIGNAL_2(modelReset)
   CS_SIGNAL_1(Public, void createdItem(int index, QDeclarativeItem *item))
   CS_SIGNAL_2(createdItem, index, item)
   CS_SIGNAL_1(Public, void destroyingItem(QDeclarativeItem *item))
   CS_SIGNAL_2(destroyingItem, item)

 protected:
   QDeclarativeVisualModel(QObject *parent = 0)
      : QObject(parent) {}

 private:
   Q_DISABLE_COPY(QDeclarativeVisualModel)
};

class QDeclarativeVisualItemModelAttached;
class QDeclarativeVisualItemModelPrivate;

class QDeclarativeVisualItemModel : public QDeclarativeVisualModel
{
   DECL_CS_OBJECT(QDeclarativeVisualItemModel)
   Q_DECLARE_PRIVATE(QDeclarativeVisualItemModel)

   CS_PROPERTY_READ(children, children)
   CS_PROPERTY_NOTIFY(children, childrenChanged)
   CS_PROPERTY_DESIGNABLE(children, false)
   CS_CLASSINFO("DefaultProperty", "children")

 public:
   QDeclarativeVisualItemModel(QObject *parent = 0);
   virtual ~QDeclarativeVisualItemModel() {}

   virtual int count() const;
   virtual bool isValid() const;
   virtual QDeclarativeItem *item(int index, bool complete = true);
   virtual ReleaseFlags release(QDeclarativeItem *item);
   virtual bool completePending() const;
   virtual void completeItem();
   virtual QString stringValue(int index, const QString &role);
   virtual void setWatchedRoles(QList<QByteArray>) {}

   virtual int indexOf(QDeclarativeItem *item, QObject *objectContext) const;

   QDeclarativeListProperty<QDeclarativeItem> children();

   static QDeclarativeVisualItemModelAttached *qmlAttachedProperties(QObject *obj);

 public:
   CS_SIGNAL_1(Public, void childrenChanged())
   CS_SIGNAL_2(childrenChanged)

 private:
   Q_DISABLE_COPY(QDeclarativeVisualItemModel)
};


class QDeclarativeVisualDataModel : public QDeclarativeVisualModel
{
   DECL_CS_OBJECT(QDeclarativeVisualDataModel)
   Q_DECLARE_PRIVATE(QDeclarativeVisualDataModel)

   CS_PROPERTY_READ(model, model)
   CS_PROPERTY_WRITE(model, setModel)
   CS_PROPERTY_READ(*delegate, delegate)
   CS_PROPERTY_WRITE(*delegate, setDelegate)
   CS_PROPERTY_READ(part, part)
   CS_PROPERTY_WRITE(part, setPart)
   CS_PROPERTY_READ(*parts, parts)
   CS_PROPERTY_CONSTANT(*parts)
   CS_PROPERTY_READ(rootIndex, rootIndex)
   CS_PROPERTY_WRITE(rootIndex, setRootIndex)
   CS_PROPERTY_NOTIFY(rootIndex, rootIndexChanged)

   CS_CLASSINFO("DefaultProperty", "delegate")

 public:
   QDeclarativeVisualDataModel();
   QDeclarativeVisualDataModel(QDeclarativeContext *, QObject *parent = 0);
   virtual ~QDeclarativeVisualDataModel();

   QVariant model() const;
   void setModel(const QVariant &);

   QDeclarativeComponent *delegate() const;
   void setDelegate(QDeclarativeComponent *);

   QVariant rootIndex() const;
   void setRootIndex(const QVariant &root);

   DECL_CS_INVOKABLE_METHOD_1(Public, QVariant modelIndex(int idx) const)
   DECL_CS_INVOKABLE_METHOD_2(modelIndex)

   DECL_CS_INVOKABLE_METHOD_1(Public, QVariant parentModelIndex() const)
   DECL_CS_INVOKABLE_METHOD_2(parentModel)

   QString part() const;
   void setPart(const QString &);

   int count() const;
   bool isValid() const {
      return delegate() != 0;
   }
   QDeclarativeItem *item(int index, bool complete = true);
   QDeclarativeItem *item(int index, const QByteArray &, bool complete = true);
   ReleaseFlags release(QDeclarativeItem *item);
   bool completePending() const;
   void completeItem();
   virtual QString stringValue(int index, const QString &role);
   virtual void setWatchedRoles(QList<QByteArray> roles);

   int indexOf(QDeclarativeItem *item, QObject *objectContext) const;

   QObject *parts();

 public:
   CS_SIGNAL_1(Public, void createdPackage(int index, QDeclarativePackage *package))
   CS_SIGNAL_2(createdPackage, index, package)
   CS_SIGNAL_1(Public, void destroyingPackage(QDeclarativePackage *package))
   CS_SIGNAL_2(destroyingPackage, package)
   CS_SIGNAL_1(Public, void rootIndexChanged())
   CS_SIGNAL_2(rootIndexChanged)

 private :
   CS_SLOT_1(Private, void _q_itemsChanged(int un_named_arg1, int un_named_arg2, const QList <int> &un_named_arg3))
   CS_SLOT_2(_q_itemsChanged)
   CS_SLOT_1(Private, void _q_itemsInserted(int index, int count))
   CS_SLOT_2(_q_itemsInserted)
   CS_SLOT_1(Private, void _q_itemsRemoved(int index, int count))
   CS_SLOT_2(_q_itemsRemoved)
   CS_SLOT_1(Private, void _q_itemsMoved(int from, int to, int count))
   CS_SLOT_2(_q_itemsMoved)
   CS_SLOT_1(Private, void _q_rowsInserted(const QModelIndex &un_named_arg1, int un_named_arg2, int un_named_arg3))
   CS_SLOT_2(_q_rowsInserted)
   CS_SLOT_1(Private, void _q_rowsRemoved(const QModelIndex &un_named_arg1, int un_named_arg2, int un_named_arg3))
   CS_SLOT_2(_q_rowsRemoved)
   CS_SLOT_1(Private, void _q_rowsMoved(const QModelIndex &un_named_arg1, int un_named_arg2, int un_named_arg3,
                                        const QModelIndex &un_named_arg4, int un_named_arg5))
   CS_SLOT_2(_q_rowsMoved)
   CS_SLOT_1(Private, void _q_dataChanged(const QModelIndex &un_named_arg1, const QModelIndex &un_named_arg2))
   CS_SLOT_2(_q_dataChanged)
   CS_SLOT_1(Private, void _q_layoutChanged())
   CS_SLOT_2(_q_layoutChanged)
   CS_SLOT_1(Private, void _q_modelReset())
   CS_SLOT_2(_q_modelReset)
   CS_SLOT_1(Private, void _q_createdPackage(int index, QDeclarativePackage *package))
   CS_SLOT_2(_q_createdPackage)
   CS_SLOT_1(Private, void _q_destroyingPackage(QDeclarativePackage *package))
   CS_SLOT_2(_q_destroyingPackage)

 private:
   Q_DISABLE_COPY(QDeclarativeVisualDataModel)
};

class QDeclarativeVisualItemModelAttached : public QObject
{
   DECL_CS_OBJECT(QDeclarativeVisualItemModelAttached)

 public:
   QDeclarativeVisualItemModelAttached(QObject *parent)
      : QObject(parent), m_index(0) {}
   ~QDeclarativeVisualItemModelAttached() {
      attachedProperties.remove(parent());
   }

   CS_PROPERTY_READ(index, index)
   CS_PROPERTY_NOTIFY(index, indexChanged)
   int index() const {
      return m_index;
   }
   void setIndex(int idx) {
      if (m_index != idx) {
         m_index = idx;
         emit indexChanged();
      }
   }

   static QDeclarativeVisualItemModelAttached *properties(QObject *obj) {
      QDeclarativeVisualItemModelAttached *rv = attachedProperties.value(obj);
      if (!rv) {
         rv = new QDeclarativeVisualItemModelAttached(obj);
         attachedProperties.insert(obj, rv);
      }
      return rv;
   }


   CS_SIGNAL_1(Public, void indexChanged())
   CS_SIGNAL_2(indexChanged)

   int m_index;
   static QHash<QObject *, QDeclarativeVisualItemModelAttached *> attachedProperties;
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeVisualModel)
QML_DECLARE_TYPE(QDeclarativeVisualItemModel)
QML_DECLARE_TYPEINFO(QDeclarativeVisualItemModel, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPE(QDeclarativeVisualDataModel)

#endif // QDECLARATIVEVISUALDATAMODEL_H
