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

#ifndef QFILESYSTEMMODEL_H
#define QFILESYSTEMMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qpair.h>
#include <QtCore/qdir.h>
#include <QtGui/qicon.h>
#include <QtCore/qdiriterator.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_FILESYSTEMMODEL

class ExtendedInformation;
class QFileSystemModelPrivate;
class QFileIconProvider;

class Q_GUI_EXPORT QFileSystemModel : public QAbstractItemModel
{
    CS_OBJECT(QFileSystemModel)

    GUI_CS_PROPERTY_READ(resolveSymlinks, resolveSymlinks)
    GUI_CS_PROPERTY_WRITE(resolveSymlinks, setResolveSymlinks)
    GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
    GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)
    GUI_CS_PROPERTY_READ(nameFilterDisables, nameFilterDisables)
    GUI_CS_PROPERTY_WRITE(nameFilterDisables, setNameFilterDisables)

public:
    GUI_CS_SIGNAL_1(Public, void rootPathChanged(const QString & newPath))
    GUI_CS_SIGNAL_2(rootPathChanged,newPath) 
    GUI_CS_SIGNAL_1(Public, void fileRenamed(const QString & path,const QString & oldName,const QString & newName))
    GUI_CS_SIGNAL_2(fileRenamed,path,oldName,newName) 
    GUI_CS_SIGNAL_1(Public, void directoryLoaded(const QString & path))
    GUI_CS_SIGNAL_2(directoryLoaded,path) 

    enum Roles {
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
        FileNameRole = Qt::UserRole + 2,
        FilePermissions = Qt::UserRole + 3
    };

    explicit QFileSystemModel(QObject *parent = 0);
    ~QFileSystemModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(const QString &path, int column = 0) const;
    QModelIndex parent(const QModelIndex &child) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant myComputer(int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    // QFileSystemModel specific API
    QModelIndex setRootPath(const QString &path);
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
    bool rmdir(const QModelIndex &index) const; // ### Qt5/should not be const

    inline QString fileName(const QModelIndex &index) const;
    inline QIcon fileIcon(const QModelIndex &index) const;
    QFile::Permissions permissions(const QModelIndex &index) const;
    inline QFileInfo fileInfo(const QModelIndex &index) const;
    bool remove(const QModelIndex &index) const;

protected:
    QFileSystemModel(QFileSystemModelPrivate &, QObject *parent = 0);
    void timerEvent(QTimerEvent *event);
    bool event(QEvent *event);

private:
    Q_DECLARE_PRIVATE(QFileSystemModel)
    Q_DISABLE_COPY(QFileSystemModel)

    GUI_CS_SLOT_1(Private, void _q_directoryChanged(const QString & directory,const QStringList & list))
    GUI_CS_SLOT_2(_q_directoryChanged)

    GUI_CS_SLOT_1(Private, void _q_performDelayedSort())
    GUI_CS_SLOT_2(_q_performDelayedSort)

    GUI_CS_SLOT_1(Private, void _q_fileSystemChanged(const QString & path,const QList <QPair <QString,QFileInfo>> & un_named_arg2))
    GUI_CS_SLOT_2(_q_fileSystemChanged)

    GUI_CS_SLOT_1(Private, void _q_resolvedName(const QString & fileName,const QString & resolvedName))
    GUI_CS_SLOT_2(_q_resolvedName)

    friend class QFileDialogPrivate;
};

inline QString QFileSystemModel::fileName(const QModelIndex &aindex) const
   { return aindex.data(Qt::DisplayRole).toString(); }

inline QIcon QFileSystemModel::fileIcon(const QModelIndex &aindex) const
   { return qvariant_cast<QIcon>(aindex.data(Qt::DecorationRole)); }

inline QFileInfo QFileSystemModel::fileInfo(const QModelIndex &aindex) const
   { return QFileInfo(filePath(aindex)); }

#endif // QT_NO_FILESYSTEMMODEL

QT_END_NAMESPACE

#endif // QFILESYSTEMMODEL_H

