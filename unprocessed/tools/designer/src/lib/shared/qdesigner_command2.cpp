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

#include "qdesigner_command2_p.h"
#include "formwindowbase_p.h"
#include "layoutinfo_p.h"
#include "qdesigner_command_p.h"
#include "widgetfactory_p.h"
#include "qlayout_widget_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerMetaDataBaseInterface>

#include <QtGui/QApplication>
#include <QtGui/QLayout>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

MorphLayoutCommand::MorphLayoutCommand(QDesignerFormWindowInterface *formWindow) :
    QDesignerFormWindowCommand(QString(), formWindow),
    m_breakLayoutCommand(new BreakLayoutCommand(formWindow)),
    m_layoutCommand(new LayoutCommand(formWindow)),
    m_newType(LayoutInfo::VBox),
    m_layoutBase(0)
{
}

MorphLayoutCommand::~MorphLayoutCommand()
{
    delete m_layoutCommand;
    delete m_breakLayoutCommand;
}

bool MorphLayoutCommand::init(QWidget *w, int newType)
{
    int oldType;
    QDesignerFormWindowInterface *fw = formWindow();
    if (!canMorph(fw, w, &oldType) || oldType == newType)
        return false;
    m_layoutBase = w;
    m_newType = newType;
    // Find all managed widgets
    m_widgets.clear();
    const QLayout *layout = LayoutInfo::managedLayout(fw->core(), w);
    const int count = layout->count();
    for (int i = 0; i < count ; i++) {
        if (QWidget *w = layout->itemAt(i)->widget())
            if (fw->isManaged(w))
                m_widgets.push_back(w);
    }
    const bool reparentLayoutWidget = false; // leave QLayoutWidget intact
    m_breakLayoutCommand->init(m_widgets, m_layoutBase, reparentLayoutWidget);
    m_layoutCommand->init(m_layoutBase, m_widgets, static_cast<LayoutInfo::Type>(m_newType), m_layoutBase, reparentLayoutWidget);
    setText(formatDescription(core(), m_layoutBase, oldType, newType));
    return true;
}

bool MorphLayoutCommand::canMorph(const QDesignerFormWindowInterface *formWindow, QWidget *w, int *ptrToCurrentType)
{
    if (ptrToCurrentType)
        *ptrToCurrentType = LayoutInfo::NoLayout;
    // We want a managed widget or a container page
    // with a level-0 managed layout
    QDesignerFormEditorInterface *core = formWindow->core();
    QLayout *layout = LayoutInfo::managedLayout(core, w);
    if (!layout)
        return false;
    const LayoutInfo::Type type = LayoutInfo::layoutType(core, layout);
    if (ptrToCurrentType)
        *ptrToCurrentType = type;
    switch (type) {
    case LayoutInfo::HBox:
    case LayoutInfo::VBox:
    case LayoutInfo::Grid:
    case LayoutInfo::Form:
        return true;
        break;
    case LayoutInfo::NoLayout:
    case LayoutInfo::HSplitter: // Nothing doing
    case LayoutInfo::VSplitter:
    case LayoutInfo::UnknownLayout:
        break;
    }
    return false;
}

void MorphLayoutCommand::redo()
{
    m_breakLayoutCommand->redo();
    m_layoutCommand->redo();
    /* Transfer applicable properties which is a cross-section of the modified
     * properties except object name. */
    if (const LayoutProperties *properties = m_breakLayoutCommand->layoutProperties()) {
        const int oldMask = m_breakLayoutCommand->propertyMask();
        QLayout *newLayout = LayoutInfo::managedLayout(core(), m_layoutBase);
        const int newMask = LayoutProperties::visibleProperties(newLayout);
        const int applicableMask = (oldMask & newMask) & ~LayoutProperties::ObjectNameProperty;
        if (applicableMask)
            properties->toPropertySheet(core(), newLayout, applicableMask);
    }
}

void MorphLayoutCommand::undo()
{
    m_layoutCommand->undo();
    m_breakLayoutCommand->undo();
}

QString MorphLayoutCommand::formatDescription(QDesignerFormEditorInterface * /* core*/, const QWidget *w, int oldType, int newType)
{
    const QString oldName = LayoutInfo::layoutName(static_cast<LayoutInfo::Type>(oldType));
    const QString newName = LayoutInfo::layoutName(static_cast<LayoutInfo::Type>(newType));
    const QString widgetName = qobject_cast<const QLayoutWidget*>(w) ? w->layout()->objectName() : w->objectName();
    return QApplication::translate("Command", "Change layout of '%1' from %2 to %3").arg(widgetName, oldName, newName);
}

LayoutAlignmentCommand::LayoutAlignmentCommand(QDesignerFormWindowInterface *formWindow) :
    QDesignerFormWindowCommand(QApplication::translate("Command", "Change layout alignment"), formWindow),
    m_newAlignment(0), m_oldAlignment(0), m_widget(0)
{
}

bool LayoutAlignmentCommand::init(QWidget *w, Qt::Alignment alignment)
{
    bool enabled;
    m_newAlignment = alignment;
    m_oldAlignment = LayoutAlignmentCommand::alignmentOf(core(), w, &enabled);
    m_widget = w;
    return enabled;
}

void LayoutAlignmentCommand::redo()
{
    LayoutAlignmentCommand::applyAlignment(core(), m_widget, m_newAlignment);
}

void LayoutAlignmentCommand::undo()
{
    LayoutAlignmentCommand::applyAlignment(core(), m_widget, m_oldAlignment);
}

// Find out alignment and return whether command is enabled.
Qt::Alignment LayoutAlignmentCommand::alignmentOf(const QDesignerFormEditorInterface *core, QWidget *w, bool *enabledIn)
{
    bool managed;
    QLayout *layout;

    if (enabledIn)
        *enabledIn = false;
    // Can only work on a managed layout
    const LayoutInfo::Type type = LayoutInfo::laidoutWidgetType(core, w, &managed, &layout);
    const bool enabled = layout && managed &&
                         (type == LayoutInfo::HBox || type == LayoutInfo::VBox
                          || type == LayoutInfo::Grid);
    if (!enabled)
        return Qt::Alignment(0);
    // Get alignment
    const int index = layout->indexOf(w);
    Q_ASSERT(index >= 0);
    if (enabledIn)
        *enabledIn = true;
    return layout->itemAt(index)->alignment();
}

void LayoutAlignmentCommand::applyAlignment(const QDesignerFormEditorInterface *core, QWidget *w, Qt::Alignment a)
{
    // Find layout and apply to item
    QLayout *layout;
    LayoutInfo::laidoutWidgetType(core, w, 0, &layout);
    if (layout) {
        const int index = layout->indexOf(w);
        if (index >= 0) {
            layout->itemAt(index)->setAlignment(a);
            layout->update();
        }
    }
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
