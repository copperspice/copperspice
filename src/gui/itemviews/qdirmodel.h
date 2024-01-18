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

#ifndef QDIRMODEL_H
#define QDIRMODEL_H

#include <qabstractitemmodel.h>
#include <qdir.h>
#include <qfileiconprovider.h>

#ifndef QT_NO_DIRMODEL

class QDirModelPrivate;

class Q_GUI_EXPORT QDirModel : public QAbstractItemModel
{
   GUI_CS_OBJECT(QDirModel)

   GUI_CS_PROPERTY_READ(resolveSymlinks, resolveSymlinks)
   GUI_CS_PROPERTY_WRITE(resolveSymlinks, setResolveSymlinks)

   GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
   GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)

   GUI_CS_PROPERTY_READ(lazyChildCount, lazyChildCount)
   GUI_CS_PROPERTY_WRITE(lazyChildCount, setLazyChildCount)

 public:
   enum Roles {
      FileIconRole = Qt::DecorationRole,
      FilePathRole = Qt::UserRole + 1,
      FileNameRole
   };

   QDirModel(const QStringList &nameFilters, QDir::Filters filters,
      QDir::SortFlags sort, QObject *parent = nullptr);
   explicit QDirModel(QObject *parent = nullptr);

   QDirModel(const QDirModel &) = delete;
   QDirModel &operator=(const QDirModel &) = delete;

   ~QDirModel();

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex parent(const QModelIndex &child) const override;

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;

   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

   bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
   Qt::ItemFlags flags(const QModelIndex &index) const override;

   void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

   QStringList mimeTypes() const override;
   QMimeData *mimeData(const QModelIndexList &indexes) const override;

   bool dropMimeData(const QMimeData *data, Qt::DropAction action,
      int row, int column, const QModelIndex &parent) override;

   Qt::DropActions supportedDropActions() const override;

   // QDirModel specific API

   void setIconProvider(QFileIconProvider *provider);
   QFileIconProvider *iconProvider() const;

   void setNameFilters(const QStringList &filters);
   QStringList nameFilters() const;

   void setFilter(QDir::Filters filters);
   QDir::Filters filter() const;

   void setSorting(QDir::SortFlags sort);
   QDir::SortFlags sorting() const;

   void setResolveSymlinks(bool enable);
   bool resolveSymlinks() const;

   void setReadOnly(bool enable);
   bool isReadOnly() const;

   void setLazyChildCount(bool enable);
   bool lazyChildCount() const;

   QModelIndex index(const QString &path, int column = 0) const;

   bool isDir(const QModelIndex &index) const;
   QModelIndex mkdir(const QModelIndex &parent, const QString &name);
   bool rmdir(const QModelIndex &index);
   bool remove(const QModelIndex &index);

   QString filePath(const QModelIndex &index) const;
   QString fileName(const QModelIndex &index) const;
   QIcon fileIcon(const QModelIndex &index) const;
   QFileInfo fileInfo(const QModelIndex &index) const;

   using QObject::parent;

   GUI_CS_SLOT_1(Public, void refresh(const QModelIndex &parent = QModelIndex()))
   GUI_CS_SLOT_2(refresh)

 protected:
   QDirModel(QDirModelPrivate &, QObject *parent = nullptr);
   friend class QFileDialogPrivate;

 private:
   Q_DECLARE_PRIVATE(QDirModel)

   GUI_CS_SLOT_1(Private, void _q_refresh())
   GUI_CS_SLOT_2(_q_refresh)
};

#endif // QT_NO_DIRMODEL

#endif // QDIRMODEL_H
