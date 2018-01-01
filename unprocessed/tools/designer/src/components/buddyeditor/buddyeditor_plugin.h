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

#ifndef BUDDYEDITOR_PLUGIN_H
#define BUDDYEDITOR_PLUGIN_H

#include "buddyeditor_global.h"

#include <QtDesigner/QDesignerFormEditorPluginInterface>

#include <QtCore/QPointer>
#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class BuddyEditorTool;

class QT_BUDDYEDITOR_EXPORT BuddyEditorPlugin: public QObject, public QDesignerFormEditorPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerFormEditorPluginInterface)
public:
    BuddyEditorPlugin();
    virtual ~BuddyEditorPlugin();

    virtual bool isInitialized() const;
    virtual void initialize(QDesignerFormEditorInterface *core);
    QAction *action() const;

    virtual QDesignerFormEditorInterface *core() const;

public slots:
    void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow);

private slots:
    void addFormWindow(QDesignerFormWindowInterface *formWindow);
    void removeFormWindow(QDesignerFormWindowInterface *formWindow);

private:
    QPointer<QDesignerFormEditorInterface> m_core;
    QHash<QDesignerFormWindowInterface*, BuddyEditorTool*> m_tools;
    bool m_initialized;
    QAction *m_action;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // BUDDYEDITOR_PLUGIN_H
