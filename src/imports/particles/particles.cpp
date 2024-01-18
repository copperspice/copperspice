/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <QtDeclarative/qdeclarativeextensionplugin.h>
#include <QtDeclarative/qdeclarative.h>

#include "qdeclarativeparticles_p.h"

QT_BEGIN_NAMESPACE

class QParticlesQmlModule : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
public:
    virtual void registerTypes(const char *uri)
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Qt.labs.particles"));
        qmlRegisterType<QDeclarativeParticleMotion>(uri,1,0,"ParticleMotion");
        qmlRegisterType<QDeclarativeParticleMotionGravity>(uri,1,0,"ParticleMotionGravity");
        qmlRegisterType<QDeclarativeParticleMotionLinear>(uri,1,0,"ParticleMotionLinear");
        qmlRegisterType<QDeclarativeParticleMotionWander>(uri,1,0,"ParticleMotionWander");
        qmlRegisterType<QDeclarativeParticles>(uri,1,0,"Particles");
    }
};

QT_END_NAMESPACE

Q_EXPORT_PLUGIN2(qmlparticlesplugin, QT_PREPEND_NAMESPACE(QParticlesQmlModule));

