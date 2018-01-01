/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QTRESOURCEMODEL_H
#define QTRESOURCEMODEL_H

#include "shared_global_p.h"
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class QtResourceModel;

class QDESIGNER_SHARED_EXPORT QtResourceSet // one instance per one form
{
public:
    QStringList activeQrcPaths() const;

    // activateQrcPaths(): if this QtResourceSet is active it emits resourceSetActivated();
    // otherwise only in case if active QtResource set contains one of
    // paths which was marked as modified by this resource set, the signal
    // is emitted (with reload = true);
    // If new path appears on the list it is automatically added to
    // loaded list of QtResourceModel. In addition it is marked as modified in case
    // QtResourceModel didn't contain the path.
    // If some path is removed from that list (and is not used in any other
    // resource set) it is automatically unloaded. The removed file can also be
    // marked as modified (later when another resource set which contains
    // removed path is activated will be reloaded)
    void activateQrcPaths(const QStringList &paths, int *errorCount = 0, QString *errorMessages = 0);

    bool isModified(const QString &path) const; // for all paths in resource model (redundant here, maybe it should be removed from here)
    void setModified(const QString &path);      // for all paths in resource model (redundant here, maybe it should be removed from here)
private:
    QtResourceSet();
    QtResourceSet(QtResourceModel *model);
    ~QtResourceSet();
    friend class QtResourceModel;

    QScopedPointer<class QtResourceSetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtResourceSet)
    Q_DISABLE_COPY(QtResourceSet)
};

class QDESIGNER_SHARED_EXPORT QtResourceModel : public QObject // one instance per whole designer
{
    Q_OBJECT
public:
    QtResourceModel(QObject *parent = 0);
    ~QtResourceModel();

    QStringList loadedQrcFiles() const;
    bool isModified(const QString &path) const; // only for paths which are on loadedQrcFiles() list
    void setModified(const QString &path);      // only for paths which are on loadedQrcPaths() list

    QList<QtResourceSet *> resourceSets() const;

    QtResourceSet *currentResourceSet() const;
    void setCurrentResourceSet(QtResourceSet *resourceSet, int *errorCount = 0, QString *errorMessages = 0);

    QtResourceSet *addResourceSet(const QStringList &paths);
    void removeResourceSet(QtResourceSet *resourceSet);

    void reload(const QString &path, int *errorCount = 0, QString *errorMessages = 0);
    void reload(int *errorCount = 0, QString *errorMessages = 0);

    // Contents of the current resource set (content file to qrc path)
    QMap<QString, QString> contents() const;
    // Find the qrc file belonging to the contained file (from current resource set)
    QString qrcPath(const QString &file) const;

    void setWatcherEnabled(bool enable);
    bool isWatcherEnabled() const;

    void setWatcherEnabled(const QString &path, bool enable);
    bool isWatcherEnabled(const QString &path);

signals:
    void resourceSetActivated(QtResourceSet *resourceSet, bool resourceSetChanged); // resourceSetChanged since last time it was activated!
    void qrcFileModifiedExternally(const QString &path);

private:
    friend class QtResourceSet;

    QScopedPointer<class QtResourceModelPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtResourceModel)
    Q_DISABLE_COPY(QtResourceModel)

    Q_PRIVATE_SLOT(d_func(), void slotFileChanged(const QString &))
};

QT_END_NAMESPACE

#endif
