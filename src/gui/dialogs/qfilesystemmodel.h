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

#ifndef QFILESYSTEMMODEL_H
#define QFILESYSTEMMODEL_H

#include <qabstractitemmodel.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <qicon.h>
#include <qpair.h>

#ifndef QT_NO_FILESYSTEMMODEL

class ExtendedInformation;
class QFileIconProvider;
class QFileSystemModelPrivate;

class Q_GUI_EXPORT QFileSystemModel : public QAbstractItemModel
{
   GUI_CS_OBJECT(QFileSystemModel)

   GUI_CS_PROPERTY_READ(resolveSymlinks, resolveSymlinks)
   GUI_CS_PROPERTY_WRITE(resolveSymlinks, setResolveSymlinks)

   GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
   GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)

   GUI_CS_PROPERTY_READ(nameFilterDisables, nameFilterDisables)
   GUI_CS_PROPERTY_WRITE(nameFilterDisables, setNameFilterDisables)

 public:
   GUI_CS_SIGNAL_1(Public, void rootPathChanged(const QString &newPath))
   GUI_CS_SIGNAL_2(rootPathChanged, newPath)
   GUI_CS_SIGNAL_1(Public, void fileRenamed(const QString &path, const QString &oldName, const QString &newName))
   GUI_CS_SIGNAL_2(fileRenamed, path, oldName, newName)
   GUI_CS_SIGNAL_1(Public, void directoryLoaded(const QString &path))
   GUI_CS_SIGNAL_2(directoryLoaded, path)

   enum Roles {
      FileIconRole = Qt::DecorationRole,
      FilePathRole = Qt::UserRole + 1,
      FileNameRole = Qt::UserRole + 2,
      FilePermissions = Qt::UserRole + 3
   };

   explicit QFileSystemModel(QObject *parent = nullptr);

   QFileSystemModel(const QFileSystemModel &) = delete;
   QFileSystemModel &operator=(const QFileSystemModel &) = delete;

   ~QFileSystemModel();

   QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
   QModelIndex index(const QString &path, int column = 0) const;
   QModelIndex parent(const QModelIndex &index) const override;

   using QObject::parent;

   bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
   bool canFetchMore(const QModelIndex &parent) const override;
   void fetchMore(const QModelIndex &parent) override;

   int rowCount(const QModelIndex &parent = QModelIndex()) const override;
   int columnCount(const QModelIndex &parent = QModelIndex()) const override;

   QVariant myComputer(int role = Qt::DisplayRole) const;
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
   bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

   QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

   Qt::ItemFlags flags(const QModelIndex &index) const override;

   void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

   QStringList mimeTypes() const override;
   QMimeData *mimeData(const QModelIndexList &indexes) const override;

   bool dropMimeData(const QMimeData *data, Qt::DropAction action,
      int row, int column, const QModelIndex &parent) override;

   Qt::DropActions supportedDropActions() const override;

   QModelIndex setRootPath(const QString &newPath);
   QString rootPath() const;
   QDir rootDirectory() const;

   void setIconProvider(QFileIconProvider *provider);
   QFileIconProvider *iconProvider() const;

   void setFilter(QDir::Filters filters);
   QDir::Filters filter() const;

   void setResolveSymlinks(bool enable);
   bool resolveSymlinks() const;

   void setReadOnly(bool enable);
   bool isReadOnly() const;

   void setNameFilterDisables(bool enable);
   bool nameFilterDisables() const;

   void setNameFilters(const QStringList &filters);
   QStringList nameFilters() const;

   QString filePath(const QModelIndex &index) const;
   bool isDir(const QModelIndex &index) const;
   qint64 size(const QModelIndex &index) const;
   QString type(const QModelIndex &index) const;
   QDateTime lastModified(const QModelIndex &index) const;

   QModelIndex mkdir(const QModelIndex &parent, const QString &name);
   bool rmdir(const QModelIndex &index);

   inline QString fileName(const QModelIndex &index) const;
   inline QIcon fileIcon(const QModelIndex &index) const;
   QFile::Permissions permissions(const QModelIndex &index) const;
   QFileInfo fileInfo(const QModelIndex &index) const;
   bool remove(const QModelIndex &index) const;

 protected:
   QFileSystemModel(QFileSystemModelPrivate &, QObject *parent = nullptr);
   void timerEvent(QTimerEvent *event) override;
   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QFileSystemModel)

   GUI_CS_SLOT_1(Private, void _q_directoryChanged(const QString &directory, const QStringList &list))
   GUI_CS_SLOT_2(_q_directoryChanged)

   GUI_CS_SLOT_1(Private, void _q_performDelayedSort())
   GUI_CS_SLOT_2(_q_performDelayedSort)

   GUI_CS_SLOT_1(Private, void _q_fileSystemChanged(const QString &path,
         const QVector<QPair <QString, QFileInfo>> &data))
   GUI_CS_SLOT_2(_q_fileSystemChanged)

   GUI_CS_SLOT_1(Private, void _q_resolvedName(const QString &fileName, const QString &resolvedName))
   GUI_CS_SLOT_2(_q_resolvedName)

   friend class QFileDialogPrivate;
};

inline QString QFileSystemModel::fileName(const QModelIndex &index) const
{
   return index.data(Qt::DisplayRole).toString();
}

inline QIcon QFileSystemModel::fileIcon(const QModelIndex &index) const
{
   return (index.data(Qt::DecorationRole)).value<QIcon>();
}

#endif // QT_NO_FILESYSTEMMODEL

#endif

