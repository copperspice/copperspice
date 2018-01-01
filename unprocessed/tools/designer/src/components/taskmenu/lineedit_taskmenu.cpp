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

#include "lineedit_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// -------- LineEditTaskMenuInlineEditor
class LineEditTaskMenuInlineEditor : public  TaskMenuInlineEditor
{
public:
    LineEditTaskMenuInlineEditor(QLineEdit *button, QObject *parent);

protected:
    virtual QRect editRectangle() const;
};

LineEditTaskMenuInlineEditor::LineEditTaskMenuInlineEditor(QLineEdit *w, QObject *parent) :
      TaskMenuInlineEditor(w, ValidationSingleLine, QLatin1String("text"), parent)
{
}

QRect LineEditTaskMenuInlineEditor::editRectangle() const
{
    QStyleOption opt;
    opt.init(widget());
    return opt.rect;
}

// --------------- LineEditTaskMenu
LineEditTaskMenu::LineEditTaskMenu(QLineEdit *lineEdit, QObject *parent) :
    QDesignerTaskMenu(lineEdit, parent),
    m_editTextAction(new QAction(tr("Change text..."), this))
{
    TaskMenuInlineEditor *editor = new LineEditTaskMenuInlineEditor(lineEdit, this);
    connect(m_editTextAction, SIGNAL(triggered()), editor, SLOT(editText()));
    m_taskActions.append(m_editTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

QAction *LineEditTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> LineEditTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

}

QT_END_NAMESPACE
