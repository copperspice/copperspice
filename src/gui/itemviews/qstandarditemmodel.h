/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qbrush.h>
#include <QtGui/qfont.h>
#include <QtGui/qicon.h>

#ifndef QT_NO_DATASTREAM
#include <QtCore/qdatastream.h>
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_STANDARDITEMMODEL

class QStandardItemModel;
class QStandardItemPrivate;
class QStandardItemModelPrivate;

template <class T> class QList;

class Q_GUI_EXPORT QStandardItem
{

 public:
   QStandardItem();
   QStandardItem(const QString &text);
   QStandardItem(const QIcon &icon, const QString &text);
   explicit QStandardItem(int rows, int columns = 1);
   virtual ~QStandardItem();

   virtual QVariant data(int role = Qt::UserRole + 1) const;
   virtual void setData(const QVariant &value, int role = Qt::UserRole + 1);

   inline QString text() const {
      return qvariant_cast<QString>(data(Qt::DisplayRole));
   }
   inline void setText(const QString &text);

   inline QIcon icon() const {
      return qvariant_cast<QIcon>(data(Qt::DecorationRole));
   }
   inline void setIcon(const QIcon &icon);

#ifndef QT_NO_TOOLTIP
   inline QString toolTip() const {
      return qvariant_cast<QString>(data(Qt::ToolTipRole));
   }
   inline void setToolTip(const QString &toolTip);
#endif

#ifndef QT_NO_STATUSTIP
   inline QString statusTip() const {
      return qvariant_cast<QString>(data(Qt::StatusTipRole));
   }
   inline void setStatusTip(const QString &statusTip);
#endif

#ifndef QT_NO_WHATSTHIS
   inline QString whatsThis() const {
      return qvariant_cast<QString>(data(Qt::WhatsThisRole));
   }
   inline void setWhatsThis(const QString &whatsThis);
#endif

   inline QSize sizeHint() const {
      return qvariant_cast<QSize>(data(Qt::SizeHintRole));
   }
   inline void setSizeHint(const QSize &sizeHint);

   inline QFont font() const {
      return qvariant_cast<QFont>(data(Qt::FontRole));
   }
   inline void setFont(const QFont &font);

   inline Qt::Alignment textAlignment() const {
      return Qt::Alignment(qvariant_cast<int>(data(Qt::TextAlignmentRole)));
   }
   inline void setTextAlignment(Qt::Alignment textAlignment);

   inline QBrush background() const {
      return qvariant_cast<QBrush>(data(Qt::BackgroundRole));
   }
   inline void setBackground(const QBrush &brush);

   inline QBrush foreground() const {
      return qvariant_cast<QBrush>(data(Qt::ForegroundRole));
   }
   inline void setForeground(const QBrush &brush);

   inline Qt::CheckState checkState() const {
      return Qt::CheckState(qvariant_cast<int>(data(Qt::CheckStateRole)));
   }
   inline void setCheckState(Qt::CheckState checkState);

   inline QString accessibleText() const {
      return qvariant_cast<QString>(data(Qt::AccessibleTextRole));
   }
   inline void setAccessibleText(const QString &accessibleText);

   inline QString accessibleDescription() const {
      return qvariant_cast<QString>(data(Qt::AccessibleDescriptionRole));
   }
   inline void setAccessibleDescription(const QString &accessibleDescription);

   Qt::ItemFlags flags() const;
   void setFlags(Qt::ItemFlags flags);

   inline bool isEnabled() const {
      return (flags() & Qt::ItemIsEnabled) != 0;
   }
   void setEnabled(bool enabled);

   inline bool isEditable() const {
      return (flags() & Qt::ItemIsEditable) != 0;
   }
   void setEditable(bool editable);

   inline bool isSelectable() const {
      return (flags() & Qt::ItemIsSelectable) != 0;
   }
   void setSelectable(bool selectable);

   inline bool isCheckable() const {
      return (flags() & Qt::ItemIsUserCheckable) != 0;
   }
   void setCheckable(bool checkable);

   inline bool isTristate() const {
      return (flags() & Qt::ItemIsTristate) != 0;
   }
   void setTristate(bool tristate);

#ifndef QT_NO_DRAGANDDROP
   inline bool isDragEnabled() const {
      return (flags() & Qt::ItemIsDragEnabled) != 0;
   }
   void setDragEnabled(bool dragEnabled);

   inline bool isDropEnabled() const {
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

   enum ItemType { Type = 0, UserType = 1000 };
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

inline void QStandardItem::setText(const QString &atext)
{
   setData(atext, Qt::DisplayRole);
}

inline void QStandardItem::setIcon(const QIcon &aicon)
{
   setData(aicon, Qt::DecorationRole);
}

#ifndef QT_NO_TOOLTIP
inline void QStandardItem::setToolTip(const QString &atoolTip)
{
   setData(atoolTip, Qt::ToolTipRole);
}
#endif

#ifndef QT_NO_STATUSTIP
inline void QStandardItem::setStatusTip(const QString &astatusTip)
{
   setData(astatusTip, Qt::StatusTipRole);
}
#endif

#ifndef QT_NO_WHATSTHIS
inline void QStandardItem::setWhatsThis(const QString &awhatsThis)
{
   setData(awhatsThis, Qt::WhatsThisRole);
}
#endif

inline void QStandardItem::setSizeHint(const QSize &asizeHint)
{
   setData(asizeHint, Qt::SizeHintRole);
}

inline void QStandardItem::setFont(const QFont &afont)
{
   setData(afont, Qt::FontRole);
}

inline void QStandardItem::setTextAlignment(Qt::Alignment atextAlignment)
{
   setData(int(atextAlignment), Qt::TextAlignmentRole);
}

inline void QStandardItem::setBackground(const QBrush &abrush)
{
   setData(abrush, Qt::BackgroundRole);
}

inline void QStandardItem::setForeground(const QBrush &abrush)
{
   setData(abrush, Qt::ForegroundRole);
}

inline void QStandardItem::setCheckState(Qt::CheckState acheckState)
{
   setData(acheckState, Qt::CheckStateRole);
}

inline void QStandardItem::setAccessibleText(const QString &aaccessibleText)
{
   setData(aaccessibleText, Qt::AccessibleTextRole);
}

inline void QStandardItem::setAccessibleDescription(const QString &aaccessibleDescription)
{
   setData(aaccessibleDescription, Qt::AccessibleDescriptionRole);
}

inline void QStandardItem::setChild(int arow, QStandardItem *aitem)
{
   setChild(arow, 0, aitem);
}

inline void QStandardItem::appendRow(const QList<QStandardItem *> &aitems)
{
   insertRow(rowCount(), aitems);
}

inline void QStandardItem::appendRows(const QList<QStandardItem *> &aitems)
{
   insertRows(rowCount(), aitems);
}

inline void QStandardItem::appendColumn(const QList<QStandardItem *> &aitems)
{
   insertColumn(columnCount(), aitems);
}

inline void QStandardItem::insertRow(int arow, QStandardItem *aitem)
{
   insertRow(arow, QList<QStandardItem *>() << aitem);
}

inline void QStandardItem::appendRow(QStandardItem *aitem)
{
   insertRow(rowCount(), aitem);
}

class Q_GUI_EXPORT QStandardItemModel : public QAbstractItemModel
{
   GUI_CS_OBJECT(QStandardItemModel)

   GUI_CS_PROPERTY_READ(sortRole, sortRole)
   GUI_CS_PROPERTY_WRITE(sortRole, setSortRole)

 public:
   explicit QStandardItemModel(QObject *parent = nullptr);
   QStandardItemModel(int rows, int columns, QObject *parent = nullptr);
   ~QStandardItemModel();

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

   Q_DISABLE_COPY(QStandardItemModel)
   Q_DECLARE_PRIVATE(QStandardItemModel)

   GUI_CS_SLOT_1(Private, void _q_emitItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight))
   GUI_CS_SLOT_2(_q_emitItemChanged)
};

inline void QStandardItemModel::setItem(int arow, QStandardItem *aitem)
{
   setItem(arow, 0, aitem);
}

inline void QStandardItemModel::appendRow(QStandardItem *aitem)
{
   appendRow(QList<QStandardItem *>() << aitem);
}

inline void QStandardItemModel::insertRow(int arow, QStandardItem *aitem)
{
   insertRow(arow, QList<QStandardItem *>() << aitem);
}

inline bool QStandardItemModel::insertRow(int arow, const QModelIndex &aparent)
{
   return QAbstractItemModel::insertRow(arow, aparent);
}

inline bool QStandardItemModel::insertColumn(int acolumn, const QModelIndex &aparent)
{
   return QAbstractItemModel::insertColumn(acolumn, aparent);
}

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &in, QStandardItem &item);
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &out, const QStandardItem &item);
#endif

#endif // QT_NO_STANDARDITEMMODEL

QT_END_NAMESPACE

#endif //QSTANDARDITEMMODEL_H
