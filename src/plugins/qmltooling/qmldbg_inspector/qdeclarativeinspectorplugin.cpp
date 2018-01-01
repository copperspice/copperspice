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

#include <qdeclarativeinspectorplugin.h>
#include <qdeclarativeviewinspector_p.h>
#include <QtCore/qplugin.h>
#include <qdeclarativeinspectorservice_p.h>

namespace QmlJSDebugger {

QDeclarativeInspectorPlugin::QDeclarativeInspectorPlugin() :
    m_inspector(0)
{
}

QDeclarativeInspectorPlugin::~QDeclarativeInspectorPlugin()
{
    delete m_inspector;
}

void QDeclarativeInspectorPlugin::activate()
{
    QDeclarativeInspectorService *service = QDeclarativeInspectorService::instance();
    QList<QDeclarativeView*> views = service->views();
    if (views.isEmpty())
        return;

    // TODO: Support multiple views
    QDeclarativeView *view = service->views().at(0);
    m_inspector = new QDeclarativeViewInspector(view, view);
}

void QDeclarativeInspectorPlugin::deactivate()
{
    delete m_inspector;
}

} // namespace QmlJSDebugger

Q_EXPORT_PLUGIN2(declarativeinspector, QmlJSDebugger::QDeclarativeInspectorPlugin)
