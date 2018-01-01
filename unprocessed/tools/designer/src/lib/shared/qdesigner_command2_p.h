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

#ifndef QDESIGNER_COMMAND2_H
#define QDESIGNER_COMMAND2_H

#include "shared_global_p.h"
#include "qdesigner_formwindowcommand_p.h"

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class LayoutCommand;
class BreakLayoutCommand;

/* This command changes the type of a managed layout on a widget (including
 * red layouts of type 'QLayoutWidget') into another type, maintaining the
 * applicable properties. It does this by chaining BreakLayoutCommand and
 * LayoutCommand, parametrizing them not to actually delete/reparent
 * QLayoutWidget's. */

class QDESIGNER_SHARED_EXPORT MorphLayoutCommand : public QDesignerFormWindowCommand {
    Q_DISABLE_COPY(MorphLayoutCommand)
public:
    explicit MorphLayoutCommand(QDesignerFormWindowInterface *formWindow);
    virtual ~MorphLayoutCommand();

    bool init(QWidget *w, int newType);

    static bool canMorph(const QDesignerFormWindowInterface *formWindow, QWidget *w, int *ptrToCurrentType = 0);

    virtual void redo();
    virtual void undo();

private:
    static QString formatDescription(QDesignerFormEditorInterface *core, const QWidget *w, int oldType, int newType);

    BreakLayoutCommand *m_breakLayoutCommand;
    LayoutCommand *m_layoutCommand;
    int m_newType;
    QWidgetList m_widgets;
    QWidget *m_layoutBase;
};

// Change the alignment of a widget in a managed grid/box layout cell.
class LayoutAlignmentCommand : public QDesignerFormWindowCommand {
    Q_DISABLE_COPY(LayoutAlignmentCommand)
public:
    explicit LayoutAlignmentCommand(QDesignerFormWindowInterface *formWindow);

    bool init(QWidget *w, Qt::Alignment alignment);

    virtual void redo();
    virtual void undo();

    // Find out alignment and return whether command is enabled.
    static Qt::Alignment alignmentOf(const QDesignerFormEditorInterface *core, QWidget *w, bool *enabled = 0);

private:
    static void applyAlignment(const QDesignerFormEditorInterface *core, QWidget *w, Qt::Alignment a);

    Qt::Alignment m_newAlignment;
    Qt::Alignment m_oldAlignment;
    QWidget *m_widget;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_COMMAND2_H
