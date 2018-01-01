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

#ifndef QDESIGNER_APPFONTWIDGET_H
#define QDESIGNER_APPFONTWIDGET_H

#include <QtGui/QGroupBox>
#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class AppFontModel;

class QTreeView;
class QToolButton;
class QItemSelection;
class QDesignerSettingsInterface;

// AppFontWidget: Manages application fonts which the user can load and 
// provides API for saving/restoring them.

class AppFontWidget : public QGroupBox
{
    Q_DISABLE_COPY(AppFontWidget)
    Q_OBJECT
public:
    explicit AppFontWidget(QWidget *parent = 0);

    QStringList fontFiles() const;

    static void save(QDesignerSettingsInterface *s, const QString &prefix);
    static void restore(const QDesignerSettingsInterface *s, const QString &prefix);

private slots:
    void addFiles();
    void slotRemoveFiles();
    void slotRemoveAll();
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

private:
    QTreeView *m_view;
    QToolButton *m_addButton;
    QToolButton *m_removeButton;
    QToolButton *m_removeAllButton;
    AppFontModel *m_model;
};

// AppFontDialog: Non modal dialog for AppFontWidget which has Qt::WA_DeleteOnClose set.

class AppFontDialog : public QDialog
{
    Q_DISABLE_COPY(AppFontDialog)
    Q_OBJECT
public:
    explicit AppFontDialog(QWidget *parent = 0);

private:
    AppFontWidget *m_appFontWidget;
};

QT_END_NAMESPACE

#endif // QDESIGNER_APPFONTWIDGET_H
