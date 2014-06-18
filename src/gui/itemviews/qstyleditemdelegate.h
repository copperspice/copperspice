/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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
   CS_OBJECT(QStyledItemDelegate)

 public:
   explicit QStyledItemDelegate(QObject *parent = 0);
   ~QStyledItemDelegate();

   // painting
   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
   QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

   // editing
   QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

   void setEditorData(QWidget *editor, const QModelIndex &index) const;
   void setModelData(QWidget *editor,
                     QAbstractItemModel *model,
                     const QModelIndex &index) const;

   void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

   // editor factory
   QItemEditorFactory *itemEditorFactory() const;
   void setItemEditorFactory(QItemEditorFactory *factory);

   virtual QString displayText(const QVariant &value, const QLocale &locale) const;

 protected:
   virtual void initStyleOption(QStyleOptionViewItem *option,
                                const QModelIndex &index) const;

   bool eventFilter(QObject *object, QEvent *event);
   bool editorEvent(QEvent *event, QAbstractItemModel *model,
                    const QStyleOptionViewItem &option, const QModelIndex &index);

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
