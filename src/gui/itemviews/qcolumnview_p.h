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

#ifndef QCOLUMNVIEW_P_H
#define QCOLUMNVIEW_P_H

#include "qcolumnview.h"

#ifndef QT_NO_QCOLUMNVIEW

#include <qabstractitemview_p.h>

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qpropertyanimation.h>
#include <QtGui/qabstractitemdelegate.h>
#include <QtGui/qabstractitemview.h>
#include <QtGui/qitemdelegate.h>
#include <qlistview.h>
#include <qevent.h>
#include <qscrollbar.h>

QT_BEGIN_NAMESPACE

class QColumnViewPreviewColumn : public QAbstractItemView
{

 public:
   QColumnViewPreviewColumn(QWidget *parent) : QAbstractItemView(parent), previewWidget(0) {
   }

   void setPreviewWidget(QWidget *widget) {
      previewWidget = widget;
      setMinimumWidth(previewWidget->minimumWidth());
   }

   void resizeEvent(QResizeEvent *event) {
      if (!previewWidget) {
         return;
      }
      previewWidget->resize(
         qMax(previewWidget->minimumWidth(), event->size().width()),
         previewWidget->height());
      QSize p = viewport()->size();
      QSize v = previewWidget->size();
      horizontalScrollBar()->setRange(0, v.width() - p.width());
      horizontalScrollBar()->setPageStep(p.width());
      verticalScrollBar()->setRange(0, v.height() - p.height());
      verticalScrollBar()->setPageStep(p.height());

      QAbstractScrollArea::resizeEvent(event);
   }

   QRect visualRect(const QModelIndex &) const {
      return QRect();
   }
   void scrollTo(const QModelIndex &, ScrollHint) {
   }
   QModelIndex indexAt(const QPoint &) const {
      return QModelIndex();
   }
   QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers) {
      return QModelIndex();
   }
   int horizontalOffset () const {
      return 0;
   }
   int verticalOffset () const {
      return 0;
   }
   QRegion visualRegionForSelection(const QItemSelection &) const {
      return QRegion();
   }
   bool isIndexHidden(const QModelIndex &) const {
      return false;
   }
   void setSelection(const QRect &, QItemSelectionModel::SelectionFlags) {
   }
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
   void _q_columnsInserted(const QModelIndex &parent, int start, int end);

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

/*!
 * This is a delegate that will paint the triangle
 */
class QColumnViewDelegate : public QItemDelegate
{

 public:
   explicit QColumnViewDelegate(QObject *parent = 0) : QItemDelegate(parent) {}
   ~QColumnViewDelegate() {}

   void paint(QPainter *painter,
              const QStyleOptionViewItem &option,
              const QModelIndex &index) const;
};
#endif // QT_NO_QCOLUMNVIEW


QT_END_NAMESPACE
#endif //QCOLUMNVIEW_P_H

