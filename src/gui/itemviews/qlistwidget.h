/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QLISTWIDGET_H
#define QLISTWIDGET_H

#include <qlistview.h>
#include <qvariant.h>
#include <qvector.h>
#include <qitemselectionmodel.h>

#ifndef QT_NO_LISTWIDGET

class QEvent;
class QListWidget;
class QListModel;
class QWidgetItemData;

class QListWidgetPrivate;
class QListWidgetItemPrivate;

class Q_GUI_EXPORT QListWidgetItem
{
   friend class QListModel;
   friend class QListWidget;

 public:
   enum ItemType { Type = 0, UserType = 1000 };
   explicit QListWidgetItem(QListWidget *view = nullptr, int type = Type);
   explicit QListWidgetItem(const QString &text, QListWidget *view = nullptr, int type = Type);
   explicit QListWidgetItem(const QIcon &icon, const QString &text, QListWidget *view = 0, int type = Type);
   QListWidgetItem(const QListWidgetItem &other);
   virtual ~QListWidgetItem();

   virtual QListWidgetItem *clone() const;

   inline QListWidget *listWidget() const {
      return view;
   }

   inline void setSelected(bool select);
   inline bool isSelected() const;

   inline void setHidden(bool hide);
   inline bool isHidden() const;

   inline Qt::ItemFlags flags() const {
      return itemFlags;
   }
   void setFlags(Qt::ItemFlags flags);

   inline QString text() const {
      return data(Qt::DisplayRole).toString();
   }
   inline void setText(const QString &text);

   inline QIcon icon() const {
      return qvariant_cast<QIcon>(data(Qt::DecorationRole));
   }
   inline void setIcon(const QIcon &icon);

   inline QString statusTip() const {
      return data(Qt::StatusTipRole).toString();
   }
   inline void setStatusTip(const QString &statusTip);

#ifndef QT_NO_TOOLTIP
   inline QString toolTip() const {
      return data(Qt::ToolTipRole).toString();
   }
   inline void setToolTip(const QString &toolTip);
#endif

#ifndef QT_NO_WHATSTHIS
   inline QString whatsThis() const {
      return data(Qt::WhatsThisRole).toString();
   }
   inline void setWhatsThis(const QString &whatsThis);
#endif

   inline QFont font() const {
      return qvariant_cast<QFont>(data(Qt::FontRole));
   }
   inline void setFont(const QFont &font);

   inline int textAlignment() const {
      return data(Qt::TextAlignmentRole).toInt();
   }
   inline void setTextAlignment(int alignment) {
      setData(Qt::TextAlignmentRole, alignment);
   }

   inline QColor backgroundColor() const {
      return qvariant_cast<QColor>(data(Qt::BackgroundColorRole));
   }
   virtual void setBackgroundColor(const QColor &color) {
      setData(Qt::BackgroundColorRole, color);
   }

   inline QBrush background() const {
      return qvariant_cast<QBrush>(data(Qt::BackgroundRole));
   }
   inline void setBackground(const QBrush &brush) {
      setData(Qt::BackgroundRole, brush);
   }

   inline QColor textColor() const {
      return qvariant_cast<QColor>(data(Qt::TextColorRole));
   }
   inline void setTextColor(const QColor &color) {
      setData(Qt::TextColorRole, color);
   }

   inline QBrush foreground() const {
      return qvariant_cast<QBrush>(data(Qt::ForegroundRole));
   }
   inline void setForeground(const QBrush &brush) {
      setData(Qt::ForegroundRole, brush);
   }

   inline Qt::CheckState checkState() const {
      return static_cast<Qt::CheckState>(data(Qt::CheckStateRole).toInt());
   }
   inline void setCheckState(Qt::CheckState state) {
      setData(Qt::CheckStateRole, static_cast<int>(state));
   }

   inline QSize sizeHint() const {
      return qvariant_cast<QSize>(data(Qt::SizeHintRole));
   }
   inline void setSizeHint(const QSize &size) {
      setData(Qt::SizeHintRole, size);
   }

   virtual QVariant data(int role) const;
   virtual void setData(int role, const QVariant &value);

   virtual bool operator<(const QListWidgetItem &other) const;


   virtual void read(QDataStream &in);
   virtual void write(QDataStream &out) const;

   QListWidgetItem &operator=(const QListWidgetItem &other);

   inline int type() const {
      return rtti;
   }

 private:
   int rtti;
   QVector<void *> dummy;
   QListWidget *view;
   QListWidgetItemPrivate *d;
   Qt::ItemFlags itemFlags;
};

inline void QListWidgetItem::setText(const QString &atext)
{
   setData(Qt::DisplayRole, atext);
}

inline void QListWidgetItem::setIcon(const QIcon &aicon)
{
   setData(Qt::DecorationRole, aicon);
}

inline void QListWidgetItem::setStatusTip(const QString &astatusTip)
{
   setData(Qt::StatusTipRole, astatusTip);
}

#ifndef QT_NO_TOOLTIP
inline void QListWidgetItem::setToolTip(const QString &atoolTip)
{
   setData(Qt::ToolTipRole, atoolTip);
}
#endif

#ifndef QT_NO_WHATSTHIS
inline void QListWidgetItem::setWhatsThis(const QString &awhatsThis)
{
   setData(Qt::WhatsThisRole, awhatsThis);
}
#endif

inline void QListWidgetItem::setFont(const QFont &afont)
{
   setData(Qt::FontRole, afont);
}


Q_GUI_EXPORT QDataStream &operator<<(QDataStream &out, const QListWidgetItem &item);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QListWidgetItem &item);



class Q_GUI_EXPORT QListWidget : public QListView
{
   GUI_CS_OBJECT(QListWidget)

   GUI_CS_PROPERTY_READ(count, count)

   GUI_CS_PROPERTY_READ(currentRow, currentRow)
   GUI_CS_PROPERTY_WRITE(currentRow, cs_setCurrentRow)
   GUI_CS_PROPERTY_NOTIFY(currentRow, currentRowChanged)
   GUI_CS_PROPERTY_USER(currentRow, true)

   GUI_CS_PROPERTY_READ(sortingEnabled, isSortingEnabled)
   GUI_CS_PROPERTY_WRITE(sortingEnabled, setSortingEnabled)

   friend class QListWidgetItem;
   friend class QListModel;

 public:
   explicit QListWidget(QWidget *parent = nullptr);
   ~QListWidget();

   QListWidgetItem *item(int row) const;
   int row(const QListWidgetItem *item) const;
   void insertItem(int row, QListWidgetItem *item);
   void insertItem(int row, const QString &label);
   void insertItems(int row, const QStringList &labels);
   inline void addItem(const QString &label) {
      insertItem(count(), label);
   }
   inline void addItem(QListWidgetItem *item);
   inline void addItems(const QStringList &labels) {
      insertItems(count(), labels);
   }
   QListWidgetItem *takeItem(int row);
   int count() const;

   QListWidgetItem *currentItem() const;
   void setCurrentItem(QListWidgetItem *item);
   void setCurrentItem(QListWidgetItem *item, QItemSelectionModel::SelectionFlags command);

   int currentRow() const;
   void setCurrentRow(int row);
   void setCurrentRow(int row, QItemSelectionModel::SelectionFlags command);

   // wrapper for overloaded method
   inline void cs_setCurrentRow(int row);

   QListWidgetItem *itemAt(const QPoint &p) const;
   inline QListWidgetItem *itemAt(int x, int y) const;
   QRect visualItemRect(const QListWidgetItem *item) const;

   void sortItems(Qt::SortOrder order = Qt::AscendingOrder);
   void setSortingEnabled(bool enable);
   bool isSortingEnabled() const;

   void editItem(QListWidgetItem *item);
   void openPersistentEditor(QListWidgetItem *item);
   void closePersistentEditor(QListWidgetItem *item);

   QWidget *itemWidget(QListWidgetItem *item) const;
   void setItemWidget(QListWidgetItem *item, QWidget *widget);
   inline void removeItemWidget(QListWidgetItem *item);

   bool isItemSelected(const QListWidgetItem *item) const;
   void setItemSelected(const QListWidgetItem *item, bool select);
   QList<QListWidgetItem *> selectedItems() const;
   QList<QListWidgetItem *> findItems(const QString &text, Qt::MatchFlags flags) const;

   bool isItemHidden(const QListWidgetItem *item) const;
   void setItemHidden(const QListWidgetItem *item, bool hide);
   void dropEvent(QDropEvent *event) override;

   GUI_CS_SLOT_1(Public, void scrollToItem(const QListWidgetItem *item, QAbstractItemView::ScrollHint hint = EnsureVisible))
   GUI_CS_SLOT_2(scrollToItem)

   GUI_CS_SLOT_1(Public, void clear())
   GUI_CS_SLOT_2(clear)

   GUI_CS_SIGNAL_1(Public, void itemPressed(QListWidgetItem *item))
   GUI_CS_SIGNAL_2(itemPressed, item)

   GUI_CS_SIGNAL_1(Public, void itemClicked(QListWidgetItem *item))
   GUI_CS_SIGNAL_2(itemClicked, item)

   GUI_CS_SIGNAL_1(Public, void itemDoubleClicked(QListWidgetItem *item))
   GUI_CS_SIGNAL_2(itemDoubleClicked, item)

   GUI_CS_SIGNAL_1(Public, void itemActivated(QListWidgetItem *item))
   GUI_CS_SIGNAL_2(itemActivated, item)

   GUI_CS_SIGNAL_1(Public, void itemEntered(QListWidgetItem *item))
   GUI_CS_SIGNAL_2(itemEntered, item)

   GUI_CS_SIGNAL_1(Public, void itemChanged(QListWidgetItem *item))
   GUI_CS_SIGNAL_2(itemChanged, item)

   GUI_CS_SIGNAL_1(Public, void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous))
   GUI_CS_SIGNAL_2(currentItemChanged, current, previous)

   GUI_CS_SIGNAL_1(Public, void currentTextChanged(const QString &currentText))
   GUI_CS_SIGNAL_2(currentTextChanged, currentText)

   GUI_CS_SIGNAL_1(Public, void currentRowChanged(int currentRow))
   GUI_CS_SIGNAL_2(currentRowChanged, currentRow)

   GUI_CS_SIGNAL_1(Public, void itemSelectionChanged())
   GUI_CS_SIGNAL_2(itemSelectionChanged)

 protected:
   bool event(QEvent *e) override;
   virtual QStringList mimeTypes() const;
   virtual QMimeData *mimeData(const QList<QListWidgetItem *> &items) const;

#ifndef QT_NO_DRAGANDDROP
   virtual bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
   virtual Qt::DropActions supportedDropActions() const;
#endif
   QList<QListWidgetItem *> items(const QMimeData *data) const;

   QModelIndex indexFromItem(QListWidgetItem *item) const;
   QListWidgetItem *itemFromIndex(const QModelIndex &index) const;

 private:
   void setModel(QAbstractItemModel *model) override;
   Qt::SortOrder sortOrder() const;

   Q_DECLARE_PRIVATE(QListWidget)
   Q_DISABLE_COPY(QListWidget)

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

   GUI_CS_SLOT_1(Private, void _q_emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &current))
   GUI_CS_SLOT_2(_q_emitCurrentItemChanged)

   GUI_CS_SLOT_1(Private, void _q_sort())
   GUI_CS_SLOT_2(_q_sort)

   GUI_CS_SLOT_1(Private, void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight))
   GUI_CS_SLOT_2(_q_dataChanged)

};

void QListWidget::cs_setCurrentRow(int row)
{
   setCurrentRow(row);
}

void QListWidget::removeItemWidget(QListWidgetItem *aItem)
{
   setItemWidget(aItem, nullptr);
}

void QListWidget::addItem(QListWidgetItem *aitem)
{
   insertItem(count(), aitem);
}

QListWidgetItem *QListWidget::itemAt(int ax, int ay) const
{
   return itemAt(QPoint(ax, ay));
}

void QListWidgetItem::setSelected(bool aselect)
{
   if (view) {
      view->setItemSelected(this, aselect);
   }
}

bool QListWidgetItem::isSelected() const
{
   return (view ? view->isItemSelected(this) : false);
}

void QListWidgetItem::setHidden(bool ahide)
{
   if (view) {
      view->setItemHidden(this, ahide);
   }
}

bool QListWidgetItem::isHidden() const
{
   return (view ? view->isItemHidden(this) : false);
}


#endif // QT_NO_LISTWIDGET

#endif // QLISTWIDGET_H
