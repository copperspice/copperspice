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

#ifndef QITEMDELEGATE_H
#define QITEMDELEGATE_H

#include <qabstractitemdelegate.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qvariant.h>

#ifndef QT_NO_ITEMVIEWS

class QItemDelegatePrivate;
class QItemEditorFactory;

class Q_GUI_EXPORT QItemDelegate : public QAbstractItemDelegate
{
   GUI_CS_OBJECT(QItemDelegate)

   GUI_CS_PROPERTY_READ(clipping, hasClipping)
   GUI_CS_PROPERTY_WRITE(clipping, setClipping)

 public:
   explicit QItemDelegate(QObject *parent = nullptr);

   QItemDelegate(const QItemDelegate &) = delete;
   QItemDelegate &operator=(const QItemDelegate &) = delete;

   ~QItemDelegate();

   bool hasClipping() const;
   void setClipping(bool enabled);

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

 protected:
   virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text) const;

   virtual void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QPixmap &pixmap) const;

   virtual void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const;
   virtual void drawCheck(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, Qt::CheckState state) const;

   void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

   void doLayout(const QStyleOptionViewItem &option, QRect *checkRect, QRect *iconRect, QRect *textRect, bool hint) const;

   QRect rect(const QStyleOptionViewItem &option, const QModelIndex &index, int role) const;

   bool eventFilter(QObject *object, QEvent *event) override;
   bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

   QStyleOptionViewItem setOptions(const QModelIndex &index, const QStyleOptionViewItem &option) const;

   QPixmap decoration(const QStyleOptionViewItem &option, const QVariant &variant) const;
   QPixmap *selected(const QPixmap &pixmap, const QPalette &palette, bool enabled) const;

   QRect doCheck(const QStyleOptionViewItem &option, const QRect &bounding, const QVariant &variant) const;
   QRect textRectangle(QPainter *painter, const QRect &rect, const QFont &font, const QString &text) const;

 private:
   Q_DECLARE_PRIVATE(QItemDelegate)
};

#endif // QT_NO_ITEMVIEWS

#endif
