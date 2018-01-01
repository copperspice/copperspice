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

#ifndef SIGNALSLOTEDITOR_TOOL_H
#define SIGNALSLOTEDITOR_TOOL_H

#include "signalsloteditor_global.h"
#include "signalsloteditor.h"

#include <QtCore/QPointer>
#include <QtDesigner/QDesignerFormWindowToolInterface>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QAction;

namespace qdesigner_internal {

class SignalSlotEditor;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditorTool: public QDesignerFormWindowToolInterface
{
    Q_OBJECT
public:
    explicit SignalSlotEditorTool(QDesignerFormWindowInterface *formWindow, QObject *parent = 0);
    virtual ~SignalSlotEditorTool();

    virtual QDesignerFormEditorInterface *core() const;
    virtual QDesignerFormWindowInterface *formWindow() const;

    virtual QWidget *editor() const;

    QAction *action() const;

    virtual void activated();
    virtual void deactivated();

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

    virtual void saveToDom(DomUI *ui, QWidget *mainContainer);
    virtual void loadFromDom(DomUI *ui, QWidget *mainContainer);

private:
    QDesignerFormWindowInterface *m_formWindow;
    mutable QPointer<qdesigner_internal::SignalSlotEditor> m_editor;
    QAction *m_action;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SIGNALSLOTEDITOR_TOOL_H
