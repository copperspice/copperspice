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

#ifndef QSTANDARDITEMMODEL_H
#define QSTANDARDITEMMODEL_H

#include <qabstractitemmodel.h>
#include <qbrush.h>
#include <qfont.h>
#include <qicon.h>
#include <qdatastream.h>

#ifndef QT_NO_STANDARDITEMMODEL

#include <qcontainerfwd.h>

class QStandardItemModel;
class QStandardItemPrivate;
class QStandardItemModelPrivate;

class Q_GUI_EXPORT QStandardItem
{
 public:
   enum ItemType {
      Type     = 0,
      UserType = 1000
   };

   QStandardItem();
   explicit QStandardItem(const QString &text);
   QStandardItem(const QIcon &icon, const QString &text);
   explicit QStandardItem(int rows, int columns = 1);
   virtual ~QStandardItem();

   virtual QVariant data(int role = Qt::UserRole + 1) const;
   virtual void setData(const QVariant &value, int role = Qt::UserRole + 1);

   QString text() const {
      return (data(Qt::DisplayRole)).value<QString>();
   }

   inline void setText(const QString &text);

   QIcon icon() const {
      return (data(Qt::DecorationRole)).value<QIcon>();
   }

   inline void setIcon(const QIcon &icon);

#ifndef QT_NO_TOOLTIP
   inline QString toolTip() const {
      return (data(Qt::ToolTipRole)).value<QString>();
   }
   inline void setToolTip(const QString &toolTip);
#endif

#ifndef QT_NO_STATUSTIP
   inline QString statusTip() const {
      return (data(Qt::StatusTipRole)).value<QString>();
   }
   inline void setStatusTip(const QString &statusTip);
#endif

#ifndef QT_NO_WHATSTHIS
   inline QString whatsThis() const {
      return (data(Qt::WhatsThisRole)).value<QString>();
   }

   inline void setWhatsThis(const QString &whatsThis);
#endif

   QSize sizeHint() const {
      return (data(Qt::SizeHintRole)).value<QSize>();
   }
   inline void setSizeHint(const QSize &size);

   QFont font() const {
      return (data(Qt::FontRole)).value<QFont>();
   }
   inline void setFont(const QFont &font);

   Qt::Alignment textAlignment() const {
      return Qt::Alignment((data(Qt::TextAlignmentRole)).value<int>());
   }
   inline void setTextAlignment(Qt::Alignment alignment);

   QBrush background() const {
      return (data(Qt::BackgroundRole)).value<QBrush>();
   }
   inline void setBackground(const QBrush &brush);

   QBrush foreground() const {
      return (data(Qt::ForegroundRole)).value<QBrush>();
   }
   inline void setForeground(const QBrush &brush);

   inline Qt::CheckState checkState() const {
      return Qt::CheckState((data(Qt::CheckStateRole)).value<int>());
   }
   inline void setCheckState(Qt::CheckState state);

   inline QString accessibleText() const {
      return (data(Qt::AccessibleTextRole)).value<QString>();
   }
   inline void setAccessibleText(const QString &accessibleText);

   QString accessibleDescription() const {
      return (data(Qt::AccessibleDescriptionRole)).value<QString>();
   }
   inline void setAccessibleDescription(const QString &accessibleDescription);

   Qt::ItemFlags flags() const;
   void setFlags(Qt::ItemFlags flags);

   bool isEnabled() const {
      return (flags() & Qt::ItemIsEnabled) != 0;
   }
   void setEnabled(bool enabled);

   bool isEditable() const {
      return (flags() & Qt::ItemIsEditable) != 0;
   }
   void setEditable(bool editable);

   bool isSelectable() const {
      return (flags() & Qt::ItemIsSelectable) != 0;
   }
   void setSelectable(bool selectable);

   bool isCheckable() const {
      return (flags() & Qt::ItemIsUserCheckable) != 0;
   }
   void setCheckable(bool checkable);

   bool isAutoTristate() const {
      return (flags() & Qt::ItemIsAutoTristate) != 0;
   }
   void setAutoTristate(bool tristate);

   bool isUserTristate() const {
      return (flags() & Qt::ItemIsUserTristate) != 0;
   }
   void setUserTristate(bool tristate);

#ifndef QT_NO_DRAGANDDROP
   bool isDragEnabled() const {
      return (flags() & Qt::ItemIsDragEnabled) != 0;
   }
   void setDragEnabled(bool dragEnabled);

   bool isDropEnabled() const {
      return (flags() & Qt::ItemIsDropEnabled) != 0;
   }
   void setDropEnabled(bool dropEnabled);
#endif

   QStandardItem *parent() const;
   int row() const;
   int column() const;
   QModelIndex index() const;
   QStandardItemModel *model() const;

   int rowCount() const;
   void setRowCount(int rows);
   int columnCount() const;
   void setColumnCount(int columns);

   bool hasChildren() const;
   QStandardItem *child(int row, int column = 0) const;

   void setChild(int row, int column, QStandardItem *item);
   inline void setChild(int row, QStandardItem *item);

   void insertRow(int row, const QList<QStandardItem *> &items);
   void insertColumn(int column, const QList<QStandardItem *> &items);
   void insertRows(int row, const QList<QStandardItem *> &items);
   void insertRows(int row, int count);
   void insertColumns(int column, int count);

   void removeRow(int row);
   void removeColumn(int column);
   void removeRows(int row, int count);
   void removeColumns(int column, int count);

   inline void appendRow(const QList<QStandardItem *> &items);
   inline void appendRows(const QList<QStandardItem *> &items);
   inline void appendColumn(const QList<QStandardItem *> &items);
   inline void insertRow(int row, QStandardItem *item);
   inline void appendRow(QStandardItem *item);

   QStandardItem *takeChild(int row, int column = 0);
   QList<QStandardItem *> takeRow(int row);
   QList<QStandardItem *> takeColumn(int column);

   void sortChildren(int column, Qt::SortOrder order = Qt::AscendingOrder);

   virtual QStandardItem *clone() const;
   virtual int type() const;

#ifndef QT_NO_DATASTREAM
   virtual void read(QDataStream &in);
   virtual void write(QDataStream &out) const;
#endif

   virtual bool operator<(const QStandardItem &other) const;

 protected:
   QStandardItem(const QStandardItem &other);
   QStandardItem(QStandardItemPrivate &dd);
   QStandardItem &operator=(const QStandardItem &other);
   QScopedPointer<QStandardItemPrivate> d_ptr;

   void emitDataChanged();

 private:
   Q_DECLARE_PRIVATE(QStandardItem)
   friend class QStandardItemModelPrivate;
   friend class QStandardItemModel;
};

inline void QStandardItem::setText(const QString &text)
{
   setData(text, Qt::DisplayRole);
}

inline void QStandardItem::setIcon(const QIcon &icon)
{
   setData(icon, Qt::DecorationRole);
}

#ifndef QT_NO_TOOLTIP
inline void QStandardItem::setToolTip(const QString &toolTip)
{
   setData(toolTip, Qt::ToolTipRole);
}
#endif

#ifndef QT_NO_STATUSTIP
inline void QStandardItem::setStatusTip(const QString &statusTip)
{
   setData(statusTip, Qt::StatusTipRole);
}
#endif

#ifndef QT_NO_WHATSTHIS
inline void QStandardItem::setWhatsThis(const QString &whatsThis)
{
   setData(whatsThis, Qt::WhatsThisRole);
}
#endif

inline void QStandardItem::setSizeHint(const QSize &sizeHint)
{
   setData(sizeHint, Qt::SizeHintRole);
}

inline void QStandardItem::setFont(const QFont &font)
{
   setData(font, Qt::FontRole);
}

inline void QStandardItem::setTextAlignment(Qt::Alignment textAlignment)
{
   setData(int(textAlignment), Qt::TextAlignmentRole);
}

inline void QStandardItem::setBackground(const QBrush &brush)
{
   setData(brush, Qt::BackgroundRole);
}

inline void QStandardItem::setForeground(const QBrush &brush)
{
   setData(brush, Qt::ForegroundRole);
}

inline void QStandardItem::setCheckState(Qt::CheckState checkState)
{
   setData(checkState, Qt::CheckStateRole);
}

inline void QStandardItem::setAccessibleText(const QString &accessibleText)
{
   setData(accessibleText, Qt::AccessibleTextRole);
}

inline void QStandardItem::setAccessibleDescription(const QString &accessibleDescription)
{
   setData(accessibleDescription, Qt::AccessibleDescriptionRole);
}

inline void QStandardItem::setChild(int row, QStandardItem *item)
{
   setChild(row, 0, item);
}

inline void QStandardItem::appendRow(const QList<QStandardItem *> &items)
{
   insertRow(rowCount(), items);
}

inline void QStandardItem::appendRows(const QList<QStandardItem *> &items)
{
   insertRows(rowCount(), items);
}

inline void QStandardItem::appendColumn(const QList<QStandardItem *> &items)
{
   insertColumn(columnCount(), items);
}

inline void QStandardItem::insertRow(int row, QStandardItem *item)
{
   insertRow(row, QList<QStandardItem *>() << item);
}

inline void QStandardItem::appendRow(QStandardItem *item)
{
   insertRow(rowCount(),item);
}

class Q_GUI_EXPORT QStandardItemModel : public QAbstractItemModel
{
   GUI_CS_OBJECT(QStandardItemModel)

   GUI_CS_PROPERTY_READ(sortRole, sortRole)
   GUI_CS_PROPERTY_WRITE(sortRole, setSortRole)

 public:
   explicit QStandardItemModel(QObject *parent = nullptr);
   QStandardItemModel(int rows, int columns, QObject *parent = nullptr);

   QStandardItemModel(const QStandardItemModel &) = delete;
   QStandardItemModel &operator=(const QStandardItemModel &) = delete;

   ~QStandardItemModel();

   void setItemRoleNames(const QMultiHash<int, QString> &roleNames);
   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex parent(const QModelIndex &child) const override;

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;
   bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

   QVariant headerData(int section, Qt::Orientation orientation,
      int role = Qt::DisplayRole) const override;

   bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
      int role = Qt::EditRole) override;

   bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
   bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
   bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex())override;
   bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

   Qt::ItemFlags flags(const QModelIndex &index) const override;
   Qt::DropActions supportedDropActions() const override;

   QMap<int, QVariant> itemData(const QModelIndex &index) const override;
   bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;

   void clear();
   using QObject::parent;

   void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

   QStandardItem *itemFromIndex(const QModelIndex &index) const;
   QModelIndex indexFromItem(const QStandardItem *item) const;

   QStandardItem *item(int row, int column = 0) const;
   void setItem(int row, int column, QStandardItem *item);
   inline void setItem(int row, QStandardItem *item);
   QStandardItem *invisibleRootItem() const;

   QStandardItem *horizontalHeaderItem(int column) const;
   void setHorizontalHeaderItem(int column, QStandardItem *item);
   QStandardItem *verticalHeaderItem(int row) const;
   void setVerticalHeaderItem(int row, QStandardItem *item);

   void setHorizontalHeaderLabels(const QStringList &labels);
   void setVerticalHeaderLabels(const QStringList &labels);

   void setRowCount(int rows);
   void setColumnCount(int columns);

   void appendRow(const QList<QStandardItem *> &items);
   void appendColumn(const QList<QStandardItem *> &items);
   inline void appendRow(QStandardItem *item);

   void insertRow(int row, const QList<QStandardItem *> &items);
   void insertColumn(int column, const QList<QStandardItem *> &items);
   inline void insertRow(int row, QStandardItem *item);

   inline bool insertRow(int row, const QModelIndex &parent = QModelIndex());
   inline bool insertColumn(int column, const QModelIndex &parent = QModelIndex());

   QStandardItem *takeItem(int row, int column = 0);
   QList<QStandardItem *> takeRow(int row);
   QList<QStandardItem *> takeColumn(int column);

   QStandardItem *takeHorizontalHeaderItem(int column);
   QStandardItem *takeVerticalHeaderItem(int row);

   const QStandardItem *itemPrototype() const;
   void setItemPrototype(const QStandardItem *item);

   QList<QStandardItem *> findItems(const QString &text, Qt::MatchFlags flags = Qt::MatchExactly, int column = 0) const;

   int sortRole() const;
   void setSortRole(int role);

   QStringList mimeTypes() const override;
   QMimeData *mimeData(const QModelIndexList &indexes) const override;
   bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

   GUI_CS_SIGNAL_1(Public, void itemChanged(QStandardItem *item))
   GUI_CS_SIGNAL_2(itemChanged, item)

 protected:
   QStandardItemModel(QStandardItemModelPrivate &dd, QObject *parent = nullptr);

 private:
   friend class QStandardItemPrivate;
   friend class QStandardItem;

   Q_DECLARE_PRIVATE(QStandardItemModel)

   GUI_CS_SLOT_1(Private, void _q_emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight))
   GUI_CS_SLOT_2(_q_emitItemChanged)
};

inline void QStandardItemModel::setItem(int row, QStandardItem *item)
{
   setItem(row, 0, item);
}

inline void QStandardItemModel::appendRow(QStandardItem *item)
{
   appendRow(QList<QStandardItem *>() << item);
}

inline void QStandardItemModel::insertRow(int row, QStandardItem *item)
{
   insertRow(row, QList<QStandardItem *>() << item);
}

inline bool QStandardItemModel::insertRow(int row, const QModelIndex &parent)
{
   return QAbstractItemModel::insertRow(row, parent);
}

inline bool QStandardItemModel::insertColumn(int column, const QModelIndex &parent)
{
   return QAbstractItemModel::insertColumn(column, parent);
}

Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QStandardItem &item);
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &out, const QStandardItem &item);

#endif // QT_NO_STANDARDITEMMODEL

#endif //QSTANDARDITEMMODEL_H
