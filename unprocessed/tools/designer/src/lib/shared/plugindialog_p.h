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

#ifndef PLUGINDIALOG_H
#define PLUGINDIALOG_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "ui_plugindialog.h"

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PluginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PluginDialog(QDesignerFormEditorInterface *core, QWidget *parent = 0);

private slots:
    void updateCustomWidgetPlugins();

private:
    void populateTreeWidget();
    QIcon pluginIcon(const QIcon &icon);
    QTreeWidgetItem* setTopLevelItem(const QString &itemName);
    QTreeWidgetItem* setPluginItem(QTreeWidgetItem *topLevelItem,
                                   const QString &itemName, const QFont &font);
    void setItem(QTreeWidgetItem *pluginItem, const QString &name,
                 const QString &toolTip, const QString &whatsThis, const QIcon &icon);

    QDesignerFormEditorInterface *m_core;
    Ui::PluginDialog ui;
    QIcon interfaceIcon;
    QIcon featureIcon;
};

}

QT_END_NAMESPACE

#endif
