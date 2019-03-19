/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

/********************************************************
**  This file is part of the KDE project.
********************************************************/

#include <gst/interfaces/propertyprobe.h>
#include "effectmanager.h"
#include "backend.h"
#include "gsthelper.h"

/*
 * This class manages the list of currently
 * available audio effects.
 */

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{

EffectInfo::EffectInfo(const QString &name, const QString&description, const QString&author)
        : m_name(name)
        , m_description(description)
        , m_author(author) {}

EffectManager::EffectManager(Backend *backend)
        : QObject(backend)
        , m_backend(backend)
{
    GList* factoryList = gst_registry_get_feature_list(gst_registry_get_default (), GST_TYPE_ELEMENT_FACTORY);
    QString name, klass, description, author;
    for (GList* iter = g_list_first(factoryList) ; iter != NULL ; iter = g_list_next(iter)) {
        GstPluginFeature *feature = GST_PLUGIN_FEATURE(iter->data);
        klass = gst_element_factory_get_klass(GST_ELEMENT_FACTORY(feature));
        if ( klass == "Filter/Effect/Audio" ) {
            name =  GST_PLUGIN_FEATURE_NAME(feature);

            // These plugins simply make no sense to the frontend:
            // "audiorate" Should be internal
            // "volume" not needed
            // "equalizer-nbands" not really useful at the moment

            // These plugins simply don't work or have major stability issues:
            // "iir" Does not seem to do much at the moment
            // "audioinvert" Only works for some streams, should be invesigated
            // "lpwsinc" Crashes for large values of filter kernel
            // "name" Crashes for large values of filter kernel

            // Seems to be working, but not well tested:
            // name == "rglimiter" Seems functional
            // name == "rgvolume" Seems to be working

            QString pluginString = qgetenv("PHONON_GST_ALL_EFFECTS");
            bool acceptAll = pluginString.toInteger<int>();

            if (acceptAll
                // Plugins that have been accepted so far
                 || name == "audiopanorama"
                 || name == "audioamplify"
                 || name == "audiodynamic"
                 || name == "equalizer-10bands"
                 || name == "speed")
                {
                    description = gst_element_factory_get_description (GST_ELEMENT_FACTORY(feature));
                    author = gst_element_factory_get_author (GST_ELEMENT_FACTORY(feature));
                    EffectInfo *effect = new EffectInfo(name, description, author);
                    m_audioEffectList.append(effect);
            }
        }
    }
    g_list_free(factoryList);
}

EffectManager::~EffectManager()
{
    qDeleteAll(m_audioEffectList);
    m_audioEffectList.clear();
}

/**
  * Returns a list of available audio effects
  */
const QList<EffectInfo*> EffectManager::audioEffects() const
{
    return m_audioEffectList;
}

}
}

QT_END_NAMESPACE
