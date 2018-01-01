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

#include "q3toolbar_extrainfo.h"

#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/private/ui4_p.h>

#include <Qt3Support/Q3ToolBar>

QT_BEGIN_NAMESPACE

inline QHash<QString, DomProperty *> propertyMap(const QList<DomProperty *> &properties) // ### remove me
{
    QHash<QString, DomProperty *> map;

    for (int i=0; i<properties.size(); ++i) {
        DomProperty *p = properties.at(i);
        map.insert(p->attributeName(), p);
    }

    return map;
}

Q3ToolBarExtraInfo::Q3ToolBarExtraInfo(Q3ToolBar *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{}

QWidget *Q3ToolBarExtraInfo::widget() const
{ return m_widget; }

QDesignerFormEditorInterface *Q3ToolBarExtraInfo::core() const
{ return m_core; }

bool Q3ToolBarExtraInfo::saveUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }

bool Q3ToolBarExtraInfo::loadUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }


bool Q3ToolBarExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    Q_UNUSED(ui_widget);
    return true;
}

bool Q3ToolBarExtraInfo::loadWidgetExtraInfo(DomWidget *ui_widget)
{
    Q_UNUSED(ui_widget);
    return true;
}

Q3ToolBarExtraInfoFactory::Q3ToolBarExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent)
    : QExtensionFactory(parent), m_core(core)
{}

QObject *Q3ToolBarExtraInfoFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
        return 0;

    if (Q3ToolBar *w = qobject_cast<Q3ToolBar*>(object))
        return new Q3ToolBarExtraInfo(w, m_core, parent);

    return 0;
}

QT_END_NAMESPACE
