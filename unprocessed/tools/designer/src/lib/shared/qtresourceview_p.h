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

#ifndef QTRESOURCEVIEW_H
#define QTRESOURCEVIEW_H

#include "shared_global_p.h"
#include <QtGui/QWidget>
#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QtResourceModel;
class QtResourceSet;
class QDesignerFormEditorInterface;
class QMimeData;

class QDESIGNER_SHARED_EXPORT QtResourceView : public QWidget
{
    Q_OBJECT
public:
    explicit QtResourceView(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    ~QtResourceView();

    void setDragEnabled(bool dragEnabled);
    bool dragEnabled() const;

    QtResourceModel *model() const;
    void setResourceModel(QtResourceModel *model);

    QString selectedResource() const;
    void selectResource(const QString &resource);

    QString settingsKey() const;
    void setSettingsKey(const QString &key);

    bool isResourceEditingEnabled() const;
    void setResourceEditingEnabled(bool enable);

    // Helpers for handling the drag originating in QtResourceView (Xml/text)
    enum ResourceType { ResourceImage, ResourceStyleSheet, ResourceOther };
    static QString encodeMimeData(ResourceType resourceType, const QString &path);

    static bool decodeMimeData(const QMimeData *md, ResourceType *t = 0, QString *file = 0);
    static bool decodeMimeData(const QString &text, ResourceType *t = 0, QString *file = 0);

signals:
    void resourceSelected(const QString &resource);
    void resourceActivated(const QString &resource);

protected:
    bool event(QEvent *event);

private:

    QScopedPointer<class QtResourceViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtResourceView)
    Q_DISABLE_COPY(QtResourceView)
    Q_PRIVATE_SLOT(d_func(), void slotResourceSetActivated(QtResourceSet *))
    Q_PRIVATE_SLOT(d_func(), void slotCurrentPathChanged(QTreeWidgetItem *))
    Q_PRIVATE_SLOT(d_func(), void slotCurrentResourceChanged(QListWidgetItem *))
    Q_PRIVATE_SLOT(d_func(), void slotResourceActivated(QListWidgetItem *))
    Q_PRIVATE_SLOT(d_func(), void slotEditResources())
    Q_PRIVATE_SLOT(d_func(), void slotReloadResources())
    Q_PRIVATE_SLOT(d_func(), void slotCopyResourcePath())
    Q_PRIVATE_SLOT(d_func(), void slotListWidgetContextMenuRequested(const QPoint &pos))
    Q_PRIVATE_SLOT(d_func(), void slotFilterChanged(const QString &pattern))
};

class QDESIGNER_SHARED_EXPORT  QtResourceViewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QtResourceViewDialog(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    virtual ~QtResourceViewDialog();

    QString selectedResource() const;
    void selectResource(const QString &path);

    bool isResourceEditingEnabled() const;
    void setResourceEditingEnabled(bool enable);

private:
    QScopedPointer<class QtResourceViewDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtResourceViewDialog)
    Q_DISABLE_COPY(QtResourceViewDialog)
    Q_PRIVATE_SLOT(d_func(), void slotResourceSelected(const QString &))
};

QT_END_NAMESPACE

#endif
