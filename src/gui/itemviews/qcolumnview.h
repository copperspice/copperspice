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

#ifndef QCOLUMNVIEW_H
#define QCOLUMNVIEW_H

#include <qabstractitemview.h>

#ifndef QT_NO_COLUMNVIEW

class QColumnViewPrivate;

class Q_GUI_EXPORT QColumnView : public QAbstractItemView
{
   GUI_CS_OBJECT(QColumnView)

   GUI_CS_PROPERTY_READ(resizeGripsVisible, resizeGripsVisible)
   GUI_CS_PROPERTY_WRITE(resizeGripsVisible, setResizeGripsVisible)

 public:
   explicit QColumnView(QWidget *parent = nullptr);

   QColumnView(const QColumnView &) = delete;
   QColumnView &operator=(const QColumnView &) = delete;

   ~QColumnView();

   // QAbstractItemView overloads
   QModelIndex indexAt(const QPoint &point) const override;
   void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
   QSize sizeHint() const override;
   QRect visualRect(const QModelIndex &index) const override;
   void setModel(QAbstractItemModel *model) override;
   void setSelectionModel(QItemSelectionModel *newSelectionModel) override;
   void setRootIndex(const QModelIndex &index) override;
   void selectAll() override;

   // QColumnView functions
   void setResizeGripsVisible(bool visible);
   bool resizeGripsVisible() const;

   QWidget *previewWidget() const;
   void setPreviewWidget(QWidget *widget);

   void setColumnWidths(const QList<int> &list);
   QList<int> columnWidths() const;

   GUI_CS_SIGNAL_1(Public, void updatePreviewWidget(const QModelIndex &index))
   GUI_CS_SIGNAL_2(updatePreviewWidget, index)

 protected:
   QColumnView(QColumnViewPrivate &dd, QWidget *parent = nullptr);

   // QAbstractItemView overloads
   bool isIndexHidden(const QModelIndex &index) const override;
   QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
   void resizeEvent(QResizeEvent *event) override;
   void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) override;

   QRegion visualRegionForSelection(const QItemSelection &selection) const override;
   int horizontalOffset() const override;
   int verticalOffset() const override;
   void rowsInserted(const QModelIndex &parent, int start, int end) override;
   void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

   // QColumnView functions
   void scrollContentsBy(int dx, int dy) override;
   virtual QAbstractItemView *createColumn(const QModelIndex &index);
   void initializeColumn(QAbstractItemView *column) const;

 private:
   Q_DECLARE_PRIVATE(QColumnView)

   GUI_CS_SLOT_1(Private, void _q_gripMoved(int offset))
   GUI_CS_SLOT_2(_q_gripMoved)

   GUI_CS_SLOT_1(Private, void _q_changeCurrentColumn())
   GUI_CS_SLOT_2(_q_changeCurrentColumn)

   GUI_CS_SLOT_1(Private, void _q_clicked(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_clicked)
};

#endif // QT_NO_COLUMNVIEW

#endif // QCOLUMNVIEW_H

