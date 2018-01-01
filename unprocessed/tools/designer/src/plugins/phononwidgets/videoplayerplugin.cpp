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

#include "videoplayerplugin.h"
#include "videoplayertaskmenu.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>

#include <QtCore/qplugin.h>
#include <phonon/videoplayer.h>

static const char *toolTipC = "Phonon Video Player";

QT_BEGIN_NAMESPACE

VideoPlayerPlugin::VideoPlayerPlugin(const QString &group, QObject *parent) :
    QObject(parent),
    m_group(group),
    m_initialized(false)
{
}

QString VideoPlayerPlugin::name() const
{
    return QLatin1String("Phonon::VideoPlayer");
}

QString VideoPlayerPlugin::group() const
{
    return m_group;
}

QString VideoPlayerPlugin::toolTip() const
{
    return tr(toolTipC);
}

QString VideoPlayerPlugin::whatsThis() const
{
    return tr(toolTipC);
}

QString VideoPlayerPlugin::includeFile() const
{
    return QLatin1String("<phonon/videoplayer.h>");
}

QIcon VideoPlayerPlugin::icon() const
{
    return QIcon(QLatin1String(":/trolltech/phononwidgets/images/videoplayer.png"));
}

bool VideoPlayerPlugin::isContainer() const
{
    return false;
}

QWidget *VideoPlayerPlugin::createWidget(QWidget *parent)
{
    return new Phonon::VideoPlayer(Phonon::NoCategory, parent);
}

bool VideoPlayerPlugin::isInitialized() const
{
    return m_initialized;
}

void VideoPlayerPlugin::initialize(QDesignerFormEditorInterface * core)
{
    if (m_initialized)
        return;

    QExtensionManager *mgr = core->extensionManager();
    VideoPlayerTaskMenuFactory::registerExtension(mgr, Q_TYPEID(QDesignerTaskMenuExtension));
    m_initialized = true;
}

QString VideoPlayerPlugin::domXml() const
{
    return QLatin1String("\
    <ui language=\"c++\">\
        <widget class=\"Phonon::VideoPlayer\" name=\"videoPlayer\">\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>300</width>\
                    <height>200</height>\
                </rect>\
            </property>\
        </widget>\
    </ui>");
}

QT_END_NAMESPACE
