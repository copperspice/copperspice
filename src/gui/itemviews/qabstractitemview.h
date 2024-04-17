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

#ifndef QABSTRACTITEMVIEW_H
#define QABSTRACTITEMVIEW_H

#include <qabstractscrollarea.h>
#include <qabstractitemmodel.h>
#include <qitemselectionmodel.h>
#include <qabstractitemdelegate.h>

#ifndef QT_NO_ITEMVIEWS

class QMenu;
class QDrag;
class QEvent;
class QAbstractItemViewPrivate;

class Q_GUI_EXPORT QAbstractItemView : public QAbstractScrollArea
{
   GUI_CS_OBJECT(QAbstractItemView)

   GUI_CS_ENUM(SelectionMode)
   GUI_CS_ENUM(SelectionBehavior)
   GUI_CS_ENUM(ScrollHint)
   GUI_CS_ENUM(ScrollMode)
   GUI_CS_ENUM(DragDropMode)

   GUI_CS_ENUM(EditTrigger)
   GUI_CS_FLAG(EditTrigger, EditTriggers)

   GUI_CS_PROPERTY_READ(autoScroll, hasAutoScroll)
   GUI_CS_PROPERTY_WRITE(autoScroll, setAutoScroll)

   GUI_CS_PROPERTY_READ(autoScrollMargin, autoScrollMargin)
   GUI_CS_PROPERTY_WRITE(autoScrollMargin, setAutoScrollMargin)

   GUI_CS_PROPERTY_READ(editTriggers, editTriggers)
   GUI_CS_PROPERTY_WRITE(editTriggers, setEditTriggers)

   GUI_CS_PROPERTY_READ(tabKeyNavigation, tabKeyNavigation)
   GUI_CS_PROPERTY_WRITE(tabKeyNavigation, setTabKeyNavigation)

#ifndef QT_NO_DRAGANDDROP
   GUI_CS_PROPERTY_READ(showDropIndicator, showDropIndicator)
   GUI_CS_PROPERTY_WRITE(showDropIndicator, setDropIndicatorShown)

   GUI_CS_PROPERTY_READ(dragEnabled, dragEnabled)
   GUI_CS_PROPERTY_WRITE(dragEnabled, setDragEnabled)

   GUI_CS_PROPERTY_READ(dragDropOverwriteMode, dragDropOverwriteMode)
   GUI_CS_PROPERTY_WRITE(dragDropOverwriteMode, setDragDropOverwriteMode)

   GUI_CS_PROPERTY_READ(dragDropMode, dragDropMode)
   GUI_CS_PROPERTY_WRITE(dragDropMode, setDragDropMode)

   GUI_CS_PROPERTY_READ(defaultDropAction, defaultDropAction)
   GUI_CS_PROPERTY_WRITE(defaultDropAction, setDefaultDropAction)
#endif

   GUI_CS_PROPERTY_READ(alternatingRowColors, alternatingRowColors)
   GUI_CS_PROPERTY_WRITE(alternatingRowColors, setAlternatingRowColors)
   GUI_CS_PROPERTY_READ(selectionMode, selectionMode)
   GUI_CS_PROPERTY_WRITE(selectionMode, setSelectionMode)

   GUI_CS_PROPERTY_READ(selectionBehavior, selectionBehavior)
   GUI_CS_PROPERTY_WRITE(selectionBehavior, setSelectionBehavior)

   GUI_CS_PROPERTY_READ(iconSize, iconSize)
   GUI_CS_PROPERTY_WRITE(iconSize, setIconSize)
   GUI_CS_PROPERTY_NOTIFY(iconSize, iconSizeChanged)

   GUI_CS_PROPERTY_READ(textElideMode, textElideMode)
   GUI_CS_PROPERTY_WRITE(textElideMode, setTextElideMode)

   GUI_CS_PROPERTY_READ(verticalScrollMode, verticalScrollMode)
   GUI_CS_PROPERTY_WRITE(verticalScrollMode, setVerticalScrollMode)

   GUI_CS_PROPERTY_READ(horizontalScrollMode, horizontalScrollMode)
   GUI_CS_PROPERTY_WRITE(horizontalScrollMode, setHorizontalScrollMode)

 public:
   GUI_CS_REGISTER_ENUM(
      enum SelectionMode {
         NoSelection,
         SingleSelection,
         MultiSelection,
         ExtendedSelection,
         ContiguousSelection
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum SelectionBehavior {
         SelectItems,
         SelectRows,
         SelectColumns
      };
   )

   enum ScrollHint {
      EnsureVisible,
      PositionAtTop,
      PositionAtBottom,
      PositionAtCenter
   };

   GUI_CS_REGISTER_ENUM(
      enum EditTrigger {
         NoEditTriggers  = 0,
         CurrentChanged  = 1,
         DoubleClicked   = 2,
         SelectedClicked = 4,
         EditKeyPressed  = 8,
         AnyKeyPressed   = 16,
         AllEditTriggers = 31
      };
   )

   using EditTriggers = QFlags<EditTrigger>;

   GUI_CS_REGISTER_ENUM(
      enum ScrollMode {
         ScrollPerItem,
         ScrollPerPixel
      };
   )

#ifndef QT_NO_DRAGANDDROP
   GUI_CS_REGISTER_ENUM(
      enum DragDropMode {
         NoDragDrop,
         DragOnly,
         DropOnly,
         DragDrop,
         InternalMove
      };
   )
#endif

   explicit QAbstractItemView(QWidget *parent = nullptr);

   QAbstractItemView(const QAbstractItemView &) = delete;
   QAbstractItemView &operator=(const QAbstractItemView &) = delete;

   ~QAbstractItemView();

   virtual void setModel(QAbstractItemModel *model);
   QAbstractItemModel *model() const;

   virtual void setSelectionModel(QItemSelectionModel *selectionModel);
   QItemSelectionModel *selectionModel() const;

   void setItemDelegate(QAbstractItemDelegate *delegate);
   QAbstractItemDelegate *itemDelegate() const;

   void setSelectionMode(QAbstractItemView::SelectionMode mode);
   QAbstractItemView::SelectionMode selectionMode() const;

   void setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior);
   QAbstractItemView::SelectionBehavior selectionBehavior() const;

   QModelIndex currentIndex() const;
   QModelIndex rootIndex() const;

   void setEditTriggers(EditTriggers triggers);
   EditTriggers editTriggers() const;

   void setVerticalScrollMode(ScrollMode mode);
   ScrollMode verticalScrollMode() const;

   void setHorizontalScrollMode(ScrollMode mode);
   ScrollMode horizontalScrollMode() const;

   void setAutoScroll(bool enable);
   bool hasAutoScroll() const;

   void setAutoScrollMargin(int margin);
   int autoScrollMargin() const;

   void setTabKeyNavigation(bool enable);
   bool tabKeyNavigation() const;

#ifndef QT_NO_DRAGANDDROP
   void setDropIndicatorShown(bool enable);
   bool showDropIndicator() const;

   void setDragEnabled(bool enable);
   bool dragEnabled() const;

   void setDragDropOverwriteMode(bool overwrite);
   bool dragDropOverwriteMode() const;

   void setDragDropMode(DragDropMode behavior);
   DragDropMode dragDropMode() const;

   void setDefaultDropAction(Qt::DropAction dropAction);
   Qt::DropAction defaultDropAction() const;
#endif

   void setAlternatingRowColors(bool enable);
   bool alternatingRowColors() const;

   void setIconSize(const QSize &size);
   QSize iconSize() const;

   void setTextElideMode(Qt::TextElideMode mode);
   Qt::TextElideMode textElideMode() const;

   virtual void keyboardSearch(const QString &search);

   virtual QRect visualRect(const QModelIndex &index) const = 0;
   virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) = 0;
   virtual QModelIndex indexAt(const QPoint &point) const = 0;

   QSize sizeHintForIndex(const QModelIndex &index) const;
   virtual int sizeHintForRow(int row) const;
   virtual int sizeHintForColumn(int column) const;

   void openPersistentEditor(const QModelIndex &index);
   void closePersistentEditor(const QModelIndex &index);

   void setIndexWidget(const QModelIndex &index, QWidget *widget);
   QWidget *indexWidget(const QModelIndex &index) const;

   void setItemDelegateForRow(int row, QAbstractItemDelegate *delegate);
   QAbstractItemDelegate *itemDelegateForRow(int row) const;

   void setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate);
   QAbstractItemDelegate *itemDelegateForColumn(int column) const;

   QAbstractItemDelegate *itemDelegate(const QModelIndex &index) const;

   QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

   GUI_CS_SLOT_1(Public, virtual void reset())
   GUI_CS_SLOT_2(reset)

   GUI_CS_SLOT_1(Public, virtual void setRootIndex(const QModelIndex &index))
   GUI_CS_SLOT_2(setRootIndex)

   GUI_CS_SLOT_1(Public, virtual void doItemsLayout())
   GUI_CS_SLOT_2(doItemsLayout)

   GUI_CS_SLOT_1(Public, virtual void selectAll())
   GUI_CS_SLOT_2(selectAll)

   GUI_CS_SLOT_1(Public, void edit(const QModelIndex &index))
   GUI_CS_SLOT_OVERLOAD(edit, (const QModelIndex &))

   GUI_CS_SLOT_1(Public, void clearSelection())
   GUI_CS_SLOT_2(clearSelection)

   GUI_CS_SLOT_1(Public, void setCurrentIndex(const QModelIndex &index))
   GUI_CS_SLOT_2(setCurrentIndex)

   GUI_CS_SLOT_1(Public, void scrollToTop())
   GUI_CS_SLOT_2(scrollToTop)

   GUI_CS_SLOT_1(Public, void scrollToBottom())
   GUI_CS_SLOT_2(scrollToBottom)

   using QAbstractScrollArea::update;

   GUI_CS_SLOT_1(Public, void update(const QModelIndex &index))
   GUI_CS_SLOT_OVERLOAD(update, (const QModelIndex &))

   GUI_CS_SIGNAL_1(Public, void pressed(const QModelIndex &index))
   GUI_CS_SIGNAL_2(pressed, index)

   GUI_CS_SIGNAL_1(Public, void clicked(const QModelIndex &index))
   GUI_CS_SIGNAL_2(clicked, index)

   GUI_CS_SIGNAL_1(Public, void doubleClicked(const QModelIndex &index))
   GUI_CS_SIGNAL_2(doubleClicked, index)

   GUI_CS_SIGNAL_1(Public, void activated(const QModelIndex &index))
   GUI_CS_SIGNAL_2(activated, index)

   GUI_CS_SIGNAL_1(Public, void entered(const QModelIndex &index))
   GUI_CS_SIGNAL_2(entered, index)

   GUI_CS_SIGNAL_1(Public, void viewportEntered())
   GUI_CS_SIGNAL_2(viewportEntered)

   GUI_CS_SIGNAL_1(Public, void iconSizeChanged(const QSize &size))
   GUI_CS_SIGNAL_2(iconSizeChanged, size)

 protected:
   enum CursorAction {
      MoveUp,
      MoveDown,
      MoveLeft,
      MoveRight,
      MoveHome,
      MoveEnd,
      MovePageUp,
      MovePageDown,
      MoveNext,
      MovePrevious
   };

   enum State {
      NoState,
      DraggingState,
      DragSelectingState,
      EditingState,
      ExpandingState,
      CollapsingState,
      AnimatingState
   };

#ifndef QT_NO_DRAGANDDROP
   enum DropIndicatorPosition {
      OnItem,
      AboveItem,
      BelowItem,
      OnViewport
   };
#endif

   GUI_CS_SLOT_1(Protected, virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
         const QVector<int> &roles = QVector<int>()))
   GUI_CS_SLOT_2(dataChanged)

   GUI_CS_SLOT_1(Protected, virtual void rowsInserted(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(rowsInserted)

   GUI_CS_SLOT_1(Protected, virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(rowsAboutToBeRemoved)

   GUI_CS_SLOT_1(Protected, virtual void selectionChanged(const QItemSelection &selected,
         const QItemSelection &deselected))
   GUI_CS_SLOT_2(selectionChanged)

   GUI_CS_SLOT_1(Protected, virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous))
   GUI_CS_SLOT_2(currentChanged)

   GUI_CS_SLOT_1(Protected, virtual void updateEditorData())
   GUI_CS_SLOT_2(updateEditorData)

   GUI_CS_SLOT_1(Protected, virtual void updateEditorGeometries())
   GUI_CS_SLOT_2(updateEditorGeometries)

   GUI_CS_SLOT_1(Protected, virtual void updateGeometries())
   GUI_CS_SLOT_2(updateGeometries)

   GUI_CS_SLOT_1(Protected, virtual void verticalScrollbarAction(int action))
   GUI_CS_SLOT_2(verticalScrollbarAction)

   GUI_CS_SLOT_1(Protected, virtual void horizontalScrollbarAction(int action))
   GUI_CS_SLOT_2(horizontalScrollbarAction)

   GUI_CS_SLOT_1(Protected, virtual void verticalScrollbarValueChanged(int value))
   GUI_CS_SLOT_2(verticalScrollbarValueChanged)

   GUI_CS_SLOT_1(Protected, virtual void horizontalScrollbarValueChanged(int value))
   GUI_CS_SLOT_2(horizontalScrollbarValueChanged)

   GUI_CS_SLOT_1(Protected, virtual void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint))
   GUI_CS_SLOT_2(closeEditor)

   GUI_CS_SLOT_1(Protected, virtual void commitData(QWidget *editor))
   GUI_CS_SLOT_2(commitData)

   GUI_CS_SLOT_1(Protected, virtual void editorDestroyed(QObject *editor))
   GUI_CS_SLOT_2(editorDestroyed)

   QAbstractItemView(QAbstractItemViewPrivate &, QWidget *parent = nullptr);

   void setHorizontalStepsPerItem(int steps);
   int horizontalStepsPerItem() const;
   void setVerticalStepsPerItem(int steps);
   int verticalStepsPerItem() const;

   virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) = 0;

   virtual int horizontalOffset() const = 0;
   virtual int verticalOffset() const = 0;

   virtual bool isIndexHidden(const QModelIndex &index) const = 0;

   virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) = 0;
   virtual QRegion visualRegionForSelection(const QItemSelection &selection) const = 0;
   virtual QModelIndexList selectedIndexes() const;

   virtual bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);

   virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event = nullptr) const;

   virtual QStyleOptionViewItem viewOptions() const;

   State state() const;
   void setState(State state);

   void scheduleDelayedItemsLayout();
   void executeDelayedItemsLayout();

   void setDirtyRegion(const QRegion &region);
   void scrollDirtyRegion(int dx, int dy);
   QPoint dirtyRegionOffset() const;

   void startAutoScroll();
   void stopAutoScroll();
   void doAutoScroll();

   bool focusNextPrevChild(bool next) override;
   bool event(QEvent *event) override;
   bool viewportEvent(QEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dropEvent(QDropEvent *event) override;

   DropIndicatorPosition dropIndicatorPosition() const;

   virtual void startDrag(Qt::DropActions supportedActions);
#endif

   void focusInEvent(QFocusEvent *event) override;
   void focusOutEvent(QFocusEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void timerEvent(QTimerEvent *event) override;
   void inputMethodEvent(QInputMethodEvent *event) override;

   QSize viewportSizeHint() const override;

 private:
   Q_DECLARE_PRIVATE(QAbstractItemView)

   GUI_CS_SLOT_1(Private, void _q_columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_columnsAboutToBeRemoved)

   GUI_CS_SLOT_1(Private, void _q_columnsRemoved(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_columnsRemoved)

   GUI_CS_SLOT_1(Private, void _q_columnsInserted(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_columnsInserted)

   GUI_CS_SLOT_1(Private, void _q_columnsMoved(const QModelIndex &source, int sourceStart, int sourceEnd,
         const QModelIndex &destination, int destinationStart))
   GUI_CS_SLOT_2(_q_columnsMoved)

   GUI_CS_SLOT_1(Private, void _q_rowsInserted(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_rowsInserted)

   GUI_CS_SLOT_1(Private, void _q_rowsRemoved(const QModelIndex &parent, int start, int end))
   GUI_CS_SLOT_2(_q_rowsRemoved)

   GUI_CS_SLOT_1(Private, void _q_rowsMoved(const QModelIndex &source, int sourceStart, int sourceEnd,
         const QModelIndex &destination, int destinationStart))
   GUI_CS_SLOT_2(_q_rowsMoved)

   void _q_modelDestroyed();
   void _q_layoutChanged();
   void _q_headerDataChanged();

#ifndef QT_NO_GESTURES
   GUI_CS_SLOT_1(Private, void _q_scrollerStateChanged())
   GUI_CS_SLOT_2(_q_scrollerStateChanged)
#endif

   friend class QTreeView;
   friend class QTreeViewPrivate;
   friend class QListModeViewBase;
   friend class QListViewPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractItemView::EditTriggers)

#endif // QT_NO_ITEMVIEWS

#endif
