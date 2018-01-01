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

#ifndef QSTYLEDITEMDELEGATE_H
#define QSTYLEDITEMDELEGATE_H

#include <QtGui/qabstractitemdelegate.h>
#include <QtCore/qstring.h>
#include <QtGui/qpixmap.h>
#include <QtCore/qvariant.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ITEMVIEWS

class QStyledItemDelegatePrivate;
class QItemEditorFactory;

class Q_GUI_EXPORT QStyledItemDelegate : public QAbstractItemDelegate
{
   GUI_CS_OBJECT(QStyledItemDelegate)

 public:
   explicit QStyledItemDelegate(QObject *parent = nullptr);
   ~QStyledItemDelegate();

   // painting
   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
   QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

   // editing
   QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

   void setEditorData(QWidget *editor, const QModelIndex &index) const override;
   void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

   void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

   // editor factory
   QItemEditorFactory *itemEditorFactory() const;
   void setItemEditorFactory(QItemEditorFactory *factory);

   virtual QString displayText(const QVariant &value, const QLocale &locale) const;

 protected:
   virtual void initStyleOption(QStyleOptionViewItem *option,const QModelIndex &index) const;

   bool eventFilter(QObject *object, QEvent *event) override;
   bool editorEvent(QEvent *event, QAbstractItemModel *model,const QStyleOptionViewItem &option, const QModelIndex &index) override;

 private:
   Q_DECLARE_PRIVATE(QStyledItemDelegate)
   Q_DISABLE_COPY(QStyledItemDelegate)

   GUI_CS_SLOT_1(Private, void _q_commitDataAndCloseEditor(QWidget *un_named_arg1))
   GUI_CS_SLOT_2(_q_commitDataAndCloseEditor)

 protected:
   QScopedPointer<QStyledItemDelegatePrivate> d_ptr;

};

#endif // QT_NO_ITEMVIEWS

QT_END_NAMESPACE

#endif // QSTYLEDITEMDELEGATE_H
