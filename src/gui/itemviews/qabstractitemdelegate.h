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

#ifndef QABSTRACTITEMDELEGATE_H
#define QABSTRACTITEMDELEGATE_H

#include <qobject.h>
#include <qstyleoption.h>

#ifndef QT_NO_ITEMVIEWS

class QPainter;
class QModelIndex;
class QAbstractItemModel;
class QAbstractItemView;
class QHelpEvent;
class QAbstractItemDelegatePrivate;

class Q_GUI_EXPORT QAbstractItemDelegate : public QObject
{
   GUI_CS_OBJECT(QAbstractItemDelegate)
   Q_DECLARE_PRIVATE(QAbstractItemDelegate)

 public:

   enum EndEditHint {
      NoHint,
      EditNextItem,
      EditPreviousItem,
      SubmitModelCache,
      RevertModelCache
   };

   explicit QAbstractItemDelegate(QObject *parent = nullptr);

   QAbstractItemDelegate(const QAbstractItemDelegate &) = delete;
   QAbstractItemDelegate &operator=(const QAbstractItemDelegate &) = delete;

   virtual ~QAbstractItemDelegate();

   // painting
   virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;

   virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const = 0;

   // editing
   virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

   virtual void destroyEditor(QWidget *editor, const QModelIndex &index) const;
   virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;

   virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

   virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

   // for non-widget editors
   virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
      const QModelIndex &index);

   static QString elidedText(const QFontMetrics &fontMetrics, int width, Qt::TextElideMode mode, const QString &text);

   //
   virtual bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option,
      const QModelIndex &index);

   virtual QVector<int> paintingRoles() const;

   GUI_CS_SIGNAL_1(Public, void commitData(QWidget *editor))
   GUI_CS_SIGNAL_2(commitData, editor)

   GUI_CS_SIGNAL_1(Public, void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint = NoHint))
   GUI_CS_SIGNAL_2(closeEditor, editor, hint)

   GUI_CS_SIGNAL_1(Public, void sizeHintChanged(const QModelIndex &index))
   GUI_CS_SIGNAL_2(sizeHintChanged, index)

 protected:
   QAbstractItemDelegate(QAbstractItemDelegatePrivate &, QObject *parent = nullptr);
   QScopedPointer<QAbstractItemDelegatePrivate> d_ptr;

 private:
   GUI_CS_SLOT_1(Private, void _q_commitDataAndCloseEditor(QWidget *))
   GUI_CS_SLOT_2(_q_commitDataAndCloseEditor)
};

#endif // QT_NO_ITEMVIEWS


#endif // QABSTRACTITEMDELEGATE_H
