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

#ifndef QACTIVEXTASKMENU_H
#define QACTIVEXTASKMENU_H

#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QtDesigner/private/extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QDesignerAxWidget;

class QAxWidgetTaskMenu: public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit QAxWidgetTaskMenu(QDesignerAxWidget *object, QObject *parent = 0);
    virtual ~QAxWidgetTaskMenu();
    virtual QList<QAction*> taskActions() const;

private slots:
    void setActiveXControl();
    void resetActiveXControl();

private:
    QDesignerAxWidget *m_axwidget;
    QAction *m_setAction;
    QAction *m_resetAction;
    QList<QAction*> m_taskActions;
};

typedef qdesigner_internal::ExtensionFactory<QDesignerTaskMenuExtension, QDesignerAxWidget, QAxWidgetTaskMenu>  ActiveXTaskMenuFactory;

QT_END_NAMESPACE

#endif // QACTIVEXTASKMENU
