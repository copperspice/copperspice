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

#include <qgraphicssystemplugin_p.h>
#include <qgraphicssystem_gl_p.h>
#include <qgl.h>

QT_BEGIN_NAMESPACE

class QGLGraphicsSystemPlugin : public QGraphicsSystemPlugin
{
public:
    QStringList keys() const;
    QGraphicsSystem *create(const QString&);
};

QStringList QGLGraphicsSystemPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("OpenGL") << QLatin1String("OpenGL1");
#if !defined(QT_OPENGL_ES_1)
    list << QLatin1String("OpenGL2");
#endif
#if defined(Q_WS_X11) && !defined(QT_NO_EGL)
    list << QLatin1String("X11GL");
#endif
    return list;
}

QGraphicsSystem* QGLGraphicsSystemPlugin::create(const QString& system)
{
    if (system.toLower() == QLatin1String("opengl1")) {
        QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);
        return new QGLGraphicsSystem(false);
    }

#if !defined(QT_OPENGL_ES_1)
    if (system.toLower() == QLatin1String("opengl2")) {
        QGL::setPreferredPaintEngine(QPaintEngine::OpenGL2);
        return new QGLGraphicsSystem(false);
    }
#endif

#if defined(Q_WS_X11) && !defined(QT_NO_EGL)
    if (system.toLower() == QLatin1String("x11gl"))
        return new QGLGraphicsSystem(true);
#endif

    if (system.toLower() == QLatin1String("opengl"))
        return new QGLGraphicsSystem(false);

    return 0;
}

Q_EXPORT_PLUGIN2(opengl, QGLGraphicsSystemPlugin)

QT_END_NAMESPACE
