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

#include "volumesliderplugin.h"

#include <phonon/volumeslider.h>

static const char *toolTipC = "Phonon Volume Slider";

QT_BEGIN_NAMESPACE

VolumeSliderPlugin::VolumeSliderPlugin(const QString &group, QObject *parent) :
    QObject(parent),
    m_group(group),
    m_initialized(false)
{
}

QString VolumeSliderPlugin::name() const
{
    return QLatin1String("Phonon::VolumeSlider");
}

QString VolumeSliderPlugin::group() const
{
    return m_group;
}

QString VolumeSliderPlugin::toolTip() const
{
    return tr(toolTipC);
}

QString VolumeSliderPlugin::whatsThis() const
{
    return tr(toolTipC);
}

QString VolumeSliderPlugin::includeFile() const
{
    return QLatin1String("<phonon/volumeslider.h>");
}

QIcon VolumeSliderPlugin::icon() const
{
    return QIcon(QLatin1String(":/trolltech/phononwidgets/images/volumeslider.png"));
}

bool VolumeSliderPlugin::isContainer() const
{
    return false;
}

QWidget *VolumeSliderPlugin::createWidget(QWidget *parent)
{
    return new Phonon::VolumeSlider(parent);
}

bool VolumeSliderPlugin::isInitialized() const
{
    return m_initialized;
}

void VolumeSliderPlugin::initialize(QDesignerFormEditorInterface *)
{
    if (m_initialized)
        return;
    m_initialized = true;
}

QString VolumeSliderPlugin::domXml() const
{
    return QLatin1String("\
    <ui language=\"c++\">\
        <widget class=\"Phonon::VolumeSlider\" name=\"volumeSlider\"/>\
    </ui>");
}

QT_END_NAMESPACE
