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

#include <QtGui/QPlatformIntegrationPlugin>
#include "qdirectfbintegration.h"

QT_BEGIN_NAMESPACE

#ifdef DIRECTFB_GL_EGL
#define QT_EGL_BACKEND_STRING(list) list << "directfbegl";
#define QT_EGL_BACKEND_CREATE(list, out) \
    if (list.toLower() == "directfbegl") \
        out = new QDirectFbIntegrationEGL;
#else
#define QT_EGL_BACKEND_STRING(list)
#define QT_EGL_BACKEND_CREATE(system, out)
#endif

class QDirectFbIntegrationPlugin : public QPlatformIntegrationPlugin
{
public:
    QStringList keys() const;
    QPlatformIntegration *create(const QString&, const QStringList&);
};

QStringList QDirectFbIntegrationPlugin::keys() const
{
    QStringList list;
    list << "directfb";
    QT_EGL_BACKEND_STRING(list);
    return list;
}

QPlatformIntegration * QDirectFbIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    QDirectFbIntegration *integration = 0;

    if (system.toLower() == "directfb")
        integration = new QDirectFbIntegration;
    QT_EGL_BACKEND_CREATE(system, integration)

    if (!integration)
        return 0;

    integration->initialize();
    return integration;
}

Q_EXPORT_PLUGIN2(directfb, QDirectFbIntegrationPlugin)

QT_END_NAMESPACE
