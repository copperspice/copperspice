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

#ifndef QDATAWIDGETMAPPER_H
#define QDATAWIDGETMAPPER_H

#include <QtCore/qobject.h>
#include <qabstractitemdelegate.h>
#include <QScopedPointer>

#ifndef QT_NO_DATAWIDGETMAPPER

QT_BEGIN_NAMESPACE

class QAbstractItemModel;
class QModelIndex;
class QDataWidgetMapperPrivate;

class Q_GUI_EXPORT QDataWidgetMapper: public QObject
{
   GUI_CS_OBJECT(QDataWidgetMapper)

   GUI_CS_ENUM(SubmitPolicy)
   GUI_CS_PROPERTY_READ(currentIndex, currentIndex)
   GUI_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   GUI_CS_PROPERTY_NOTIFY(currentIndex, currentIndexChanged)
   GUI_CS_PROPERTY_READ(orientation, orientation)
   GUI_CS_PROPERTY_WRITE(orientation, setOrientation)
   GUI_CS_PROPERTY_READ(submitPolicy, submitPolicy)
   GUI_CS_PROPERTY_WRITE(submitPolicy, setSubmitPolicy)

 public:
   QDataWidgetMapper(QObject *parent = nullptr);
   ~QDataWidgetMapper();

   void setModel(QAbstractItemModel *model);
   QAbstractItemModel *model() const;

   void setItemDelegate(QAbstractItemDelegate *delegate);
   QAbstractItemDelegate *itemDelegate() const;

   void setRootIndex(const QModelIndex &index);
   QModelIndex rootIndex() const;

   void setOrientation(Qt::Orientation aOrientation);
   Qt::Orientation orientation() const;

   enum SubmitPolicy { AutoSubmit, ManualSubmit };
   void setSubmitPolicy(SubmitPolicy policy);
   SubmitPolicy submitPolicy() const;

   void addMapping(QWidget *widget, int section);
   void addMapping(QWidget *widget, int section, const QByteArray &propertyName);
   void removeMapping(QWidget *widget);
   int mappedSection(QWidget *widget) const;
   QByteArray mappedPropertyName(QWidget *widget) const;
   QWidget *mappedWidgetAt(int section) const;
   void clearMapping();

   int currentIndex() const;

   GUI_CS_SLOT_1(Public, void revert())
   GUI_CS_SLOT_2(revert)
   GUI_CS_SLOT_1(Public, bool submit())
   GUI_CS_SLOT_2(submit)

   GUI_CS_SLOT_1(Public, void toFirst())
   GUI_CS_SLOT_2(toFirst)
   GUI_CS_SLOT_1(Public, void toLast())
   GUI_CS_SLOT_2(toLast)
   GUI_CS_SLOT_1(Public, void toNext())
   GUI_CS_SLOT_2(toNext)
   GUI_CS_SLOT_1(Public, void toPrevious())
   GUI_CS_SLOT_2(toPrevious)
   GUI_CS_SLOT_1(Public, virtual void setCurrentIndex(int index))
   GUI_CS_SLOT_2(setCurrentIndex)
   GUI_CS_SLOT_1(Public, void setCurrentModelIndex(const QModelIndex &index))
   GUI_CS_SLOT_2(setCurrentModelIndex)

   GUI_CS_SIGNAL_1(Public, void currentIndexChanged(int index))
   GUI_CS_SIGNAL_2(currentIndexChanged, index)

 protected:
   QScopedPointer<QDataWidgetMapperPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QDataWidgetMapper)
   Q_DISABLE_COPY(QDataWidgetMapper)

   GUI_CS_SLOT_1(Private, void _q_dataChanged(const QModelIndex &un_named_arg1, const QModelIndex &un_named_arg2))
   GUI_CS_SLOT_2(_q_dataChanged)

   GUI_CS_SLOT_1(Private, void _q_commitData(QWidget *un_named_arg1))
   GUI_CS_SLOT_2(_q_commitData)

   GUI_CS_SLOT_1(Private, void _q_closeEditor(QWidget *un_named_arg1, QAbstractItemDelegate::EndEditHint un_named_arg2))
   GUI_CS_SLOT_2(_q_closeEditor)

   GUI_CS_SLOT_1(Private, void _q_modelDestroyed())
   GUI_CS_SLOT_2(_q_modelDestroyed)
};

QT_END_NAMESPACE

#endif // QT_NO_DATAWIDGETMAPPER
#endif

