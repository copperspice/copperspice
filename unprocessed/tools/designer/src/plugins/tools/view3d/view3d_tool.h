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

#ifndef VIEW3D_TOOL_H
#define VIEW3D_TOOL_H

#include "view3d_global.h"
#include "view3d.h"
#include <QtDesigner/QDesignerFormWindowToolInterface>

QT_BEGIN_NAMESPACE

class VIEW3D_EXPORT QView3DTool : public QDesignerFormWindowToolInterface
{
    Q_OBJECT

public:
    explicit QView3DTool(QDesignerFormWindowInterface *formWindow, QObject *parent = 0);
    virtual QDesignerFormEditorInterface *core() const;
    virtual QDesignerFormWindowInterface *formWindow() const;
    virtual QWidget *editor() const;

    virtual QAction *action() const;

    virtual void activated();
    virtual void deactivated();

    virtual bool handleEvent(QWidget *widget, QWidget *managedWidget, QEvent *event);

private:
    QDesignerFormWindowInterface *m_formWindow;
    mutable QPointer<QView3D> m_editor;
    QAction *m_action;
};

#endif // VIEW3D_TOOL_H

QT_END_NAMESPACE
