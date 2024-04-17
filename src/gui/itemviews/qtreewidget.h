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

#ifndef QTREEWIDGET_H
#define QTREEWIDGET_H

#include <qtreeview.h>
#include <qtreewidgetitemiterator.h>
#include <qvariant.h>
#include <qvector.h>

#ifndef QT_NO_TREEWIDGET

class QTreeWidget;
class QTreeModel;
class QWidgetItemData;
class QTreeWidgetItemPrivate;
class QTreeWidgetPrivate;

class Q_GUI_EXPORT QTreeWidgetItem
{
 public:
   enum ItemType {
      Type     = 0,
      UserType = 1000
   };

   enum ChildIndicatorPolicy {
      ShowIndicator,
      DontShowIndicator,
      DontShowIndicatorWhenChildless
   };

   explicit QTreeWidgetItem(int type = Type);
   explicit QTreeWidgetItem(const QStringList &strings, int type = Type);

   explicit QTreeWidgetItem(QTreeWidget *view, int type = Type);
   QTreeWidgetItem(QTreeWidget *view, const QStringList &strings, int type = Type);
   QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *previous , int type = Type);

   explicit QTreeWidgetItem(QTreeWidgetItem *parent, int type = Type);
   QTreeWidgetItem(QTreeWidgetItem *parent, const QStringList &strings, int type = Type);
   QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *previous , int type = Type);

   QTreeWidgetItem(const QTreeWidgetItem &other);

   virtual ~QTreeWidgetItem();

   virtual QTreeWidgetItem *clone() const;

   inline QTreeWidget *treeWidget() const {
      return view;
   }

   inline void setSelected(bool select);
   inline bool isSelected() const;

   inline void setHidden(bool hide);
   inline bool isHidden() const;

   inline void setExpanded(bool expand);
   inline bool isExpanded() const;

   inline void setFirstColumnSpanned(bool span);
   inline bool isFirstColumnSpanned() const;

   inline void setDisabled(bool disabled);
   inline bool isDisabled() const;

   void setChildIndicatorPolicy(QTreeWidgetItem::ChildIndicatorPolicy policy);
   QTreeWidgetItem::ChildIndicatorPolicy childIndicatorPolicy() const;

   Qt::ItemFlags flags() const;
   void setFlags(Qt::ItemFlags flags);

   inline QString text(int column) const {
      return data(column, Qt::DisplayRole).toString();
   }

   inline void setText(int column, const QString &text);

   inline QIcon icon(int column) const {
      return data(column, Qt::DecorationRole).value<QIcon>();
   }

   inline void setIcon(int column, const QIcon &icon);

   inline QString statusTip(int column) const {
      return data(column, Qt::StatusTipRole).toString();
   }
   inline void setStatusTip(int column, const QString &statusTip);

#ifndef QT_NO_TOOLTIP
   inline QString toolTip(int column) const {
      return data(column, Qt::ToolTipRole).toString();
   }

   inline void setToolTip(int column, const QString &toolTip);
#endif

#ifndef QT_NO_WHATSTHIS
   inline QString whatsThis(int column) const {
      return data(column, Qt::WhatsThisRole).toString();
   }

   inline void setWhatsThis(int column, const QString &whatsThis);
#endif

   inline QFont font(int column) const {
      return data(column, Qt::FontRole).value<QFont>();
   }

   inline void setFont(int column, const QFont &font);

   inline int textAlignment(int column) const {
      return data(column, Qt::TextAlignmentRole).toInt();
   }

   inline void setTextAlignment(int column, int alignment) {
      setData(column, Qt::TextAlignmentRole, alignment);
   }

   inline QColor backgroundColor(int column) const {
      return data(column, Qt::BackgroundColorRole).value<QColor>();
   }

   inline void setBackgroundColor(int column, const QColor &color) {
      setData(column, Qt::BackgroundColorRole, color);
   }

   inline QBrush background(int column) const {
      return data(column, Qt::BackgroundRole).value<QBrush>();
   }

   inline void setBackground(int column, const QBrush &brush) {
      setData(column, Qt::BackgroundRole, brush);
   }

   inline QColor textColor(int column) const {
      return data(column, Qt::TextColorRole).value<QColor>();
   }

   inline void setTextColor(int column, const QColor &color) {
      setData(column, Qt::TextColorRole, color);
   }

   inline QBrush foreground(int column) const {
      return data(column, Qt::ForegroundRole).value<QBrush>();
   }

   inline void setForeground(int column, const QBrush &brush) {
      setData(column, Qt::ForegroundRole, brush);
   }

   inline Qt::CheckState checkState(int column) const {
      return static_cast<Qt::CheckState>(data(column, Qt::CheckStateRole).toInt());
   }

   inline void setCheckState(int column, Qt::CheckState state) {
      setData(column, Qt::CheckStateRole, state);
   }

   inline QSize sizeHint(int column) const {
      return data(column, Qt::SizeHintRole).value<QSize>();
   }

   inline void setSizeHint(int column, const QSize &size) {
      setData(column, Qt::SizeHintRole, size);
   }

   virtual QVariant data(int column, int role) const;
   virtual void setData(int column, int role, const QVariant &value);

   virtual bool operator<(const QTreeWidgetItem &other) const;

   virtual void read(QDataStream &in);
   virtual void write(QDataStream &out) const;

   QTreeWidgetItem &operator=(const QTreeWidgetItem &other);

   inline QTreeWidgetItem *parent() const {
      return par;
   }

   inline QTreeWidgetItem *child(int index) const {
      if (index < 0 || index >= children.size()) {
         return nullptr;
      }
      executePendingSort();
      return children.at(index);
   }

   inline int childCount() const {
      return children.count();
   }

   inline int columnCount() const {
      return values.count();
   }
   inline int indexOfChild(QTreeWidgetItem *child) const;

   void addChild(QTreeWidgetItem *child);
   void insertChild(int index, QTreeWidgetItem *child);
   void removeChild(QTreeWidgetItem *child);
   QTreeWidgetItem *takeChild(int index);

   void addChildren(const QList<QTreeWidgetItem *> &children);
   void insertChildren(int index, const QList<QTreeWidgetItem *> &children);
   QList<QTreeWidgetItem *> takeChildren();

   inline int type() const {
      return rtti;
   }
   inline void sortChildren(int column, Qt::SortOrder order) {
      sortChildren(column, order, false);
   }

 protected:
   void emitDataChanged();

 private:
   void sortChildren(int column, Qt::SortOrder order, bool climb);
   QVariant childrenCheckState(int column) const;
   void itemChanged();
   void executePendingSort() const;

   int rtti;

   // One item has a vector of column entries. Each column has a vector of (role, value) pairs.
   QVector< QVector<QWidgetItemData>> values;
   QTreeWidget *view;
   QTreeWidgetItemPrivate *d;
   QTreeWidgetItem *par;
   QList<QTreeWidgetItem *> children;
   Qt::ItemFlags itemFlags;

   friend class QTreeModel;
   friend class QTreeWidget;
   friend class QTreeWidgetPrivate;
   friend class QTreeWidgetItemIterator;
   friend class QTreeWidgetItemPrivate;
};

inline void QTreeWidgetItem::setText(int column, const QString &text)
{
   setData(column, Qt::DisplayRole, text);
}

inline void QTreeWidgetItem::setIcon(int column, const QIcon &icon)
{
   setData(column, Qt::DecorationRole, icon);
}

#ifndef QT_NO_STATUSTIP
inline void QTreeWidgetItem::setStatusTip(int column, const QString &statusTip)
{
   setData(column, Qt::StatusTipRole, statusTip);
}
#endif

#ifndef QT_NO_TOOLTIP
inline void QTreeWidgetItem::setToolTip(int column, const QString &toolTip)
{
   setData(column, Qt::ToolTipRole, toolTip);
}
#endif

#ifndef QT_NO_WHATSTHIS
inline void QTreeWidgetItem::setWhatsThis(int column, const QString &whatsThis)
{
   setData(column, Qt::WhatsThisRole, whatsThis);
}
#endif

inline void QTreeWidgetItem::setFont(int column, const QFont &font)
{
   setData(column, Qt::FontRole, font);
}

inline int QTreeWidgetItem::indexOfChild(QTreeWidgetItem *child) const
{
   executePendingSort();
   return children.indexOf(child);
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &out, const QTreeWidgetItem &item);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QTreeWidgetItem &item);

class Q_GUI_EXPORT QTreeWidget : public QTreeView
{
   GUI_CS_OBJECT(QTreeWidget)

   GUI_CS_PROPERTY_READ(columnCount, columnCount)
   GUI_CS_PROPERTY_WRITE(columnCount, setColumnCount)

   GUI_CS_PROPERTY_READ(topLevelItemCount, topLevelItemCount)

 public:
   explicit QTreeWidget(QWidget *parent = nullptr);

   QTreeWidget(const QTreeWidget &) = delete;
   QTreeWidget &operator=(const QTreeWidget &) = delete;

   ~QTreeWidget();

   int columnCount() const;
   void setColumnCount(int columns);

   QTreeWidgetItem *invisibleRootItem() const;
   QTreeWidgetItem *topLevelItem(int index) const;
   int topLevelItemCount() const;
   void insertTopLevelItem(int index, QTreeWidgetItem *item);
   void addTopLevelItem(QTreeWidgetItem *item);
   QTreeWidgetItem *takeTopLevelItem(int index);
   int indexOfTopLevelItem(QTreeWidgetItem *item) const;

   void insertTopLevelItems(int index, const QList<QTreeWidgetItem *> &items);
   void addTopLevelItems(const QList<QTreeWidgetItem *> &items);

   QTreeWidgetItem *headerItem() const;
   void setHeaderItem(QTreeWidgetItem *item);
   void setHeaderLabels(const QStringList &labels);
   inline void setHeaderLabel(const QString &label);

   QTreeWidgetItem *currentItem() const;
   int currentColumn() const;
   void setCurrentItem(QTreeWidgetItem *item);
   void setCurrentItem(QTreeWidgetItem *item, int column);
   void setCurrentItem(QTreeWidgetItem *item, int column, QItemSelectionModel::SelectionFlags command);

   QTreeWidgetItem *itemAt(const QPoint &p) const;
   inline QTreeWidgetItem *itemAt(int x, int y) const;
   QRect visualItemRect(const QTreeWidgetItem *item) const;

   int sortColumn() const;
   void sortItems(int column, Qt::SortOrder order);

   void editItem(QTreeWidgetItem *item, int column = 0);
   void openPersistentEditor(QTreeWidgetItem *item, int column = 0);
   void closePersistentEditor(QTreeWidgetItem *item, int column = 0);

   QWidget *itemWidget(QTreeWidgetItem *item, int column) const;
   void setItemWidget(QTreeWidgetItem *item, int column, QWidget *widget);
   inline void removeItemWidget(QTreeWidgetItem *item, int column);

   bool isItemSelected(const QTreeWidgetItem *item) const;
   void setItemSelected(const QTreeWidgetItem *item, bool select);
   QList<QTreeWidgetItem *> selectedItems() const;
   QList<QTreeWidgetItem *> findItems(const QString &text, Qt::MatchFlags flags,
      int column = 0) const;

   bool isItemHidden(const QTreeWidgetItem *item) const;
   void setItemHidden(const QTreeWidgetItem *item, bool hide);

   bool isItemExpanded(const QTreeWidgetItem *item) const;
   void setItemExpanded(const QTreeWidgetItem *item, bool expand);

   bool isFirstItemColumnSpanned(const QTreeWidgetItem *item) const;
   void setFirstItemColumnSpanned(const QTreeWidgetItem *item, bool span);

   QTreeWidgetItem *itemAbove(const QTreeWidgetItem *item) const;
   QTreeWidgetItem *itemBelow(const QTreeWidgetItem *item) const;

   void setSelectionModel(QItemSelectionModel *selectionModel) override;

   GUI_CS_SLOT_1(Public, void scrollToItem(const QTreeWidgetItem *item,
         QAbstractItemView::ScrollHint hint = EnsureVisible))
   GUI_CS_SLOT_2(scrollToItem)

   GUI_CS_SLOT_1(Public, void expandItem(const QTreeWidgetItem *item))
   GUI_CS_SLOT_2(expandItem)

   GUI_CS_SLOT_1(Public, void collapseItem(const QTreeWidgetItem *item))
   GUI_CS_SLOT_2(collapseItem)

   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SIGNAL_1(Public, void itemPressed(QTreeWidgetItem *item, int column))
   GUI_CS_SIGNAL_2(itemPressed, item, column)

   GUI_CS_SIGNAL_1(Public, void itemClicked(QTreeWidgetItem *item, int column))
   GUI_CS_SIGNAL_2(itemClicked, item, column)

   GUI_CS_SIGNAL_1(Public, void itemDoubleClicked(QTreeWidgetItem *item, int column))
   GUI_CS_SIGNAL_2(itemDoubleClicked, item, column)

   GUI_CS_SIGNAL_1(Public, void itemActivated(QTreeWidgetItem *item, int column))
   GUI_CS_SIGNAL_2(itemActivated, item, column)

   GUI_CS_SIGNAL_1(Public, void itemEntered(QTreeWidgetItem *item, int column))
   GUI_CS_SIGNAL_2(itemEntered, item, column)

   GUI_CS_SIGNAL_1(Public, void itemChanged(QTreeWidgetItem *item, int column))
   GUI_CS_SIGNAL_2(itemChanged, item, column)

   GUI_CS_SIGNAL_1(Public, void itemExpanded(QTreeWidgetItem *item))
   GUI_CS_SIGNAL_2(itemExpanded, item)

   GUI_CS_SIGNAL_1(Public, void itemCollapsed(QTreeWidgetItem *item))
   GUI_CS_SIGNAL_2(itemCollapsed, item)

   GUI_CS_SIGNAL_1(Public, void currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous))
   GUI_CS_SIGNAL_2(currentItemChanged, current, previous)

   GUI_CS_SIGNAL_1(Public, void itemSelectionChanged())
   GUI_CS_SIGNAL_2(itemSelectionChanged)

 protected:
   bool event(QEvent *event) override;
   virtual QStringList mimeTypes() const;
   virtual QMimeData *mimeData(const QList<QTreeWidgetItem *> &items) const;
   virtual bool dropMimeData(QTreeWidgetItem *parent, int index,
      const QMimeData *data, Qt::DropAction action);

   virtual Qt::DropActions supportedDropActions() const;

   QModelIndex indexFromItem(QTreeWidgetItem *item, int column = 0) const;
   QTreeWidgetItem *itemFromIndex(const QModelIndex &index) const;
   void dropEvent(QDropEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QTreeWidget)

   void setModel(QAbstractItemModel *model) override;

   friend class QTreeModel;
   friend class QTreeWidgetItem;

   GUI_CS_SLOT_1(Private, void _q_emitItemPressed(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitItemPressed)

   GUI_CS_SLOT_1(Private, void _q_emitItemClicked(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitItemClicked)

   GUI_CS_SLOT_1(Private, void _q_emitItemDoubleClicked(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitItemDoubleClicked)

   GUI_CS_SLOT_1(Private, void _q_emitItemActivated(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitItemActivated)

   GUI_CS_SLOT_1(Private, void _q_emitItemEntered(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitItemEntered)

   GUI_CS_SLOT_1(Private, void _q_emitItemChanged(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitItemChanged)

   GUI_CS_SLOT_1(Private, void _q_emitItemExpanded(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitItemExpanded)

   GUI_CS_SLOT_1(Private, void _q_emitItemCollapsed(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_emitItemCollapsed)

   GUI_CS_SLOT_1(Private, void _q_emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current))
   GUI_CS_SLOT_2(_q_emitCurrentItemChanged)

   GUI_CS_SLOT_1(Private, void _q_sort())
   GUI_CS_SLOT_2(_q_sort)

   GUI_CS_SLOT_1(Private, void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight))
   GUI_CS_SLOT_2(_q_dataChanged)

   GUI_CS_SLOT_1(Private, void _q_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected))
   GUI_CS_SLOT_2(_q_selectionChanged)
};

inline void QTreeWidget::removeItemWidget(QTreeWidgetItem *item, int column)
{
   setItemWidget(item, column, nullptr);
}

inline QTreeWidgetItem *QTreeWidget::itemAt(int x, int y) const
{
   return itemAt(QPoint(x, y));
}

inline void QTreeWidget::setHeaderLabel(const QString &label)
{
   setHeaderLabels(QStringList(label));
}

inline void QTreeWidgetItem::setSelected(bool select)
{
   if (view) {
      view->setItemSelected(this, select);
   }
}

inline bool QTreeWidgetItem::isSelected() const
{
   return (view ? view->isItemSelected(this) : false);
}

inline void QTreeWidgetItem::setHidden(bool hide)
{
   if (view) {
      view->setItemHidden(this, hide);
   }
}

inline bool QTreeWidgetItem::isHidden() const
{
   return (view ? view->isItemHidden(this) : false);
}

inline void QTreeWidgetItem::setExpanded(bool expand)
{
   if (view) {
      view->setItemExpanded(this, expand);
   }
}

inline bool QTreeWidgetItem::isExpanded() const
{
   return (view ? view->isItemExpanded(this) : false);
}

inline void QTreeWidgetItem::setFirstColumnSpanned(bool span)
{
   if (view) {
      view->setFirstItemColumnSpanned(this, span);
   }
}

inline bool QTreeWidgetItem::isFirstColumnSpanned() const
{
   return (view ? view->isFirstItemColumnSpanned(this) : false);
}

inline void QTreeWidgetItem::setDisabled(bool disabled)
{
   setFlags(disabled ? (flags() & ~Qt::ItemIsEnabled) : flags() | Qt::ItemIsEnabled);
}

inline bool QTreeWidgetItem::isDisabled() const
{
   return ! (flags() & Qt::ItemIsEnabled);
}

#endif // QT_NO_TREEWIDGET

#endif // QTREEWIDGET_H
