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

#include "seeksliderplugin.h"

#include <phonon/seekslider.h>

static const char *toolTipC = "Phonon Seek Slider";

QT_BEGIN_NAMESPACE

SeekSliderPlugin::SeekSliderPlugin(const QString &group, QObject *parent) :
    QObject(parent),
    m_group(group),
    m_initialized(false)
{
}

QString SeekSliderPlugin::name() const
{
    return QLatin1String("Phonon::SeekSlider");
}

QString SeekSliderPlugin::group() const
{
    return m_group;
}

QString SeekSliderPlugin::toolTip() const
{
    return tr(toolTipC);
}

QString SeekSliderPlugin::whatsThis() const
{
    return tr(toolTipC);
}

QString SeekSliderPlugin::includeFile() const
{
    return QLatin1String("<phonon/seekslider.h>");
}

QIcon SeekSliderPlugin::icon() const
{
    return QIcon(QLatin1String(":/trolltech/phononwidgets/images/seekslider.png"));
}

bool SeekSliderPlugin::isContainer() const
{
    return false;
}

QWidget *SeekSliderPlugin::createWidget(QWidget *parent)
{
    return new Phonon::SeekSlider(parent);
}

bool SeekSliderPlugin::isInitialized() const
{
    return m_initialized;
}

void SeekSliderPlugin::initialize(QDesignerFormEditorInterface *)
{
    if (m_initialized)
        return;
    m_initialized = true;
}

QString SeekSliderPlugin::domXml() const
{
    return QLatin1String("\
    <ui language=\"c++\">\
        <widget class=\"Phonon::SeekSlider\" name=\"seekSlider\"/>\
    </ui>");
}

QT_END_NAMESPACE
