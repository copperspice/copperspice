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

#ifndef QCOLUMNVIEW_P_H
#define QCOLUMNVIEW_P_H

#include <qcolumnview.h>

#ifndef QT_NO_QCOLUMNVIEW

#include <qabstractitemview_p.h>

#include <qabstractitemmodel.h>
#include <qpropertyanimation.h>
#include <qabstractitemdelegate.h>
#include <qabstractitemview.h>
#include <qitemdelegate.h>
#include <qlistview.h>
#include <qevent.h>
#include <qscrollbar.h>

class QColumnViewPreviewColumn : public QAbstractItemView
{
 public:
   QColumnViewPreviewColumn(QWidget *parent)
      : QAbstractItemView(parent), previewWidget(nullptr)
   {
   }

   void setPreviewWidget(QWidget *widget) {
      previewWidget = widget;
      setMinimumWidth(previewWidget->minimumWidth());
   }

   void resizeEvent(QResizeEvent *event) override {
      if (! previewWidget) {
         return;
      }

      previewWidget->resize( qMax(previewWidget->minimumWidth(), event->size().width()),
         previewWidget->height());

      QSize p = viewport()->size();
      QSize v = previewWidget->size();
      horizontalScrollBar()->setRange(0, v.width() - p.width());
      horizontalScrollBar()->setPageStep(p.width());
      verticalScrollBar()->setRange(0, v.height() - p.height());
      verticalScrollBar()->setPageStep(p.height());

      QAbstractScrollArea::resizeEvent(event);
   }

   void scrollContentsBy(int dx, int dy) override {
      if (!previewWidget) {
         return;
      }
      scrollDirtyRegion(dx, dy);
      viewport()->scroll(dx, dy);
      QAbstractItemView::scrollContentsBy(dx, dy);
   }
   QRect visualRect(const QModelIndex &) const override {
      return QRect();
   }

   void scrollTo(const QModelIndex &, ScrollHint)  override {
   }

   QModelIndex indexAt(const QPoint &) const override {
      return QModelIndex();
   }

   QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers) override {
      return QModelIndex();
   }

   int horizontalOffset () const override {
      return 0;
   }
   int verticalOffset () const override {
      return 0;
   }

   QRegion visualRegionForSelection(const QItemSelection &) const override {
      return QRegion();
   }

   bool isIndexHidden(const QModelIndex &) const override {
      return false;
   }

   void setSelection(const QRect &, QItemSelectionModel::SelectionFlags) override { }

 private:
   QWidget *previewWidget;
};

class QColumnViewPrivate : public QAbstractItemViewPrivate
{
   Q_DECLARE_PUBLIC(QColumnView)

 public:
   QColumnViewPrivate();
   ~QColumnViewPrivate();

   void initialize();

   QAbstractItemView *createColumn(const QModelIndex &index, bool show);

   void updateScrollbars();
   void closeColumns(const QModelIndex &parent = QModelIndex(), bool build = false);
   void doLayout();
   void setPreviewWidget(QWidget *widget);
   void checkColumnCreation(const QModelIndex &parent);

   void _q_gripMoved(int offset);
   void _q_changeCurrentColumn();
   void _q_clicked(const QModelIndex &index);
   void _q_columnsInserted(const QModelIndex &parent, int start, int end) override;

   QList<QAbstractItemView *> columns;
   QVector<int> columnSizes; // used during init and corner moving
   bool showResizeGrips;
   int offset;

#ifndef QT_NO_ANIMATION
   QPropertyAnimation currentAnimation;
#endif

   QWidget *previewWidget;
   QAbstractItemView *previewColumn;
};

class QColumnViewDelegate : public QItemDelegate
{
 public:
   explicit QColumnViewDelegate(QObject *parent = nullptr) : QItemDelegate(parent) {}
   ~QColumnViewDelegate() {}

   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
#endif // QT_NO_QCOLUMNVIEW

#endif //QCOLUMNVIEW_P_H

