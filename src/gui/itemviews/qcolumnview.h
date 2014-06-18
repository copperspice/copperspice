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

#ifndef QCOLUMNVIEW_H
#define QCOLUMNVIEW_H

#include <QtGui/qabstractitemview.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_COLUMNVIEW

class QColumnViewPrivate;

class Q_GUI_EXPORT QColumnView : public QAbstractItemView
{
   CS_OBJECT(QColumnView)

   GUI_CS_PROPERTY_READ(resizeGripsVisible, resizeGripsVisible)
   GUI_CS_PROPERTY_WRITE(resizeGripsVisible, setResizeGripsVisible)

 public:
   GUI_CS_SIGNAL_1(Public, void updatePreviewWidget(const QModelIndex &index))
   GUI_CS_SIGNAL_2(updatePreviewWidget, index)

   explicit QColumnView(QWidget *parent = 0);
   ~QColumnView();

   // QAbstractItemView overloads
   QModelIndex indexAt(const QPoint &point) const;
   void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
   QSize sizeHint() const;
   QRect visualRect(const QModelIndex &index) const;
   void setModel(QAbstractItemModel *model);
   void setSelectionModel(QItemSelectionModel *selectionModel);
   void setRootIndex(const QModelIndex &index);
   void selectAll();

   // QColumnView functions
   void setResizeGripsVisible(bool visible);
   bool resizeGripsVisible() const;

   QWidget *previewWidget() const;
   void setPreviewWidget(QWidget *widget);

   void setColumnWidths(const QList<int> &list);
   QList<int> columnWidths() const;

 protected:
   QColumnView(QColumnViewPrivate &dd, QWidget *parent = 0);

   // QAbstractItemView overloads
   bool isIndexHidden(const QModelIndex &index) const;
   QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
   void resizeEvent(QResizeEvent *event);
   void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
   QRegion visualRegionForSelection(const QItemSelection &selection) const;
   int horizontalOffset() const;
   int verticalOffset() const;
   void rowsInserted(const QModelIndex &parent, int start, int end);
   void currentChanged(const QModelIndex &current, const QModelIndex &previous);

   // QColumnView functions
   void scrollContentsBy(int dx, int dy);
   virtual QAbstractItemView *createColumn(const QModelIndex &rootIndex);
   void initializeColumn(QAbstractItemView *column) const;

 private:
   Q_DECLARE_PRIVATE(QColumnView)
   Q_DISABLE_COPY(QColumnView)

   GUI_CS_SLOT_1(Private, void _q_gripMoved(int un_named_arg1))
   GUI_CS_SLOT_2(_q_gripMoved)

   GUI_CS_SLOT_1(Private, void _q_changeCurrentColumn())
   GUI_CS_SLOT_2(_q_changeCurrentColumn)

   GUI_CS_SLOT_1(Private, void _q_clicked(const QModelIndex &un_named_arg1))
   GUI_CS_SLOT_2(_q_clicked)
};

#endif // QT_NO_COLUMNVIEW

QT_END_NAMESPACE

#endif // QCOLUMNVIEW_H

