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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#include "globalconfig.h"
#include "globalconfig_p.h"

#include "factory_p.h"
#include "objectdescription.h"
#include "phonondefs_p.h"
#include "platformplugin.h"
#include "backendinterface.h"
#include "qsettingsgroup_p.h"
#include "phononnamespace_p.h"
#include "pulsesupport.h"

#include <QtCore/QList>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

namespace Phonon
{

GlobalConfigPrivate::GlobalConfigPrivate() : config(QLatin1String("kde.org"), QLatin1String("libphonon"))
{
}

GlobalConfig::GlobalConfig()
    : k_ptr(new GlobalConfigPrivate)
{
}

GlobalConfig::~GlobalConfig()
{
    delete k_ptr;
}

enum WhatToFilter {
    FilterAdvancedDevices = 1,
    FilterHardwareDevices = 2,
    FilterUnavailableDevices = 4
};

static void filter(ObjectDescriptionType type, BackendInterface *backendIface, QList<int> *list, int whatToFilter)
{
    QMutableListIterator<int> it(*list);
    while (it.hasNext()) {
        QHash<QByteArray, QVariant> properties;
        if (backendIface)
            properties = backendIface->objectDescriptionProperties(type, it.next());
        else
            properties = PulseSupport::getInstance()->objectDescriptionProperties(type, it.next());
        QVariant var;
        if (whatToFilter & FilterAdvancedDevices) {
            var = properties.value("isAdvanced");
            if (var.isValid() && var.toBool()) {
                it.remove();
                continue;
            }
        }
        if (whatToFilter & FilterHardwareDevices) {
            var = properties.value("isHardwareDevice");
            if (var.isValid() && var.toBool()) {
                it.remove();
                continue;
#ifndef QT_NO_PHONON_SETTINGSGROUP
            }
        }
        if (whatToFilter & FilterUnavailableDevices) {
            var = properties.value("available");
            if (var.isValid() && !var.toBool()) {
                it.remove();
                continue;
            }
        }
    }
}

static QList<int> sortDevicesByCategoryPriority(const GlobalConfig *config, const QSettingsGroup *backendConfig, ObjectDescriptionType type, Phonon::Category category, QList<int> &defaultList)
{
    Q_ASSERT(config); Q_UNUSED(config);
    Q_ASSERT(backendConfig);
    Q_ASSERT(type == AudioOutputDeviceType || type == AudioCaptureDeviceType);

    if (defaultList.size() <= 1) {
        // nothing to sort
        return defaultList;
    } else {
        // make entries unique
        QSet<int> seen;
        QMutableListIterator<int> it(defaultList);
        while (it.hasNext()) {
            if (seen.contains(it.next())) {
                it.remove();
            } else {
                seen.insert(it.value());
            }
        }
    }

    QList<int> deviceList;
    PulseSupport *pulse = PulseSupport::getInstance();
    if (pulse->isActive()) {
        deviceList = pulse->objectIndexesByCategory(type, category);
    } else {
        QString categoryKey = QLatin1String("Category_") + QString::number(static_cast<int>(category));
        if (!backendConfig->hasKey(categoryKey)) {
            // no list in config for the given category
            categoryKey = QLatin1String("Category_") + QString::number(static_cast<int>(Phonon::NoCategory));
            if (!backendConfig->hasKey(categoryKey)) {
                // no list in config for NoCategory
                return defaultList;
            }
        }

        //Now the list from d->config
        deviceList = backendConfig->value(categoryKey, QList<int>());
    }

    //if there are devices in d->config that the backend doesn't report, remove them from the list
    QMutableListIterator<int> i(deviceList);
    while (i.hasNext()) {
        if (0 == defaultList.removeAll(i.next())) {
            i.remove();
        }
    }

    //if the backend reports more devices that are not in d->config append them to the list
    deviceList += defaultList;

    return deviceList;
}

bool GlobalConfig::hideAdvancedDevices() const
{
    K_D(const GlobalConfig);
    //The devices need to be stored independently for every backend
    const QSettingsGroup generalGroup(&d->config, QLatin1String("General"));
    return generalGroup.value(QLatin1String("HideAdvancedDevices"), true);
}

void GlobalConfig::setHideAdvancedDevices(bool hide)
{
    K_D(GlobalConfig);
    QSettingsGroup generalGroup(&d->config, QLatin1String("General"));
    generalGroup.setValue(QLatin1String("HideAdvancedDevices"), hide);
}

static bool isHiddenAudioOutputDevice(const GlobalConfig *config, int i)
{
    Q_ASSERT(config);

    if (!config->hideAdvancedDevices())
        return false;

    AudioOutputDevice ad = AudioOutputDevice::fromIndex(i);
    const QVariant var = ad.property("isAdvanced");
    return (var.isValid() && var.toBool());
}

#ifndef QT_NO_PHONON_AUDIOCAPTURE
static bool isHiddenAudioCaptureDevice(const GlobalConfig *config, int i)
{
    Q_ASSERT(config);

    if (!config->hideAdvancedDevices())
        return false;

    AudioCaptureDevice ad = AudioCaptureDevice::fromIndex(i);
    const QVariant var = ad.property("isAdvanced");
    return (var.isValid() && var.toBool());
}
#endif

static QList<int> reindexList(const GlobalConfig *config, Phonon::Category category, QList<int>newOrder, bool output)
{
    Q_ASSERT(config);
#ifdef QT_NO_PHONON_AUDIOCAPTURE
    Q_ASSERT(output);
#endif

    /*QString sb;
    sb = QString("(Size %1)").arg(currentList.size());
    for (int i : currentList) {
       sb += QString("%1, ").arg(i);
    }
    fprintf(stderr, "=== Reindex Current: %s\n", sb.toUtf8().constData());

    sb = QString("(Size %1)").arg(newOrder.size());
    for (int i : newOrder) {
       sb += QString("%1, ").arg(i);
    }  
    fprintf(stderr, "=== Reindex Before : %s\n", sb.toUtf8().constData());*/

    QList<int> currentList;
    if (output)
        currentList = config->audioOutputDeviceListFor(category, GlobalConfig::ShowUnavailableDevices|GlobalConfig::ShowAdvancedDevices);
#ifndef QT_NO_PHONON_AUDIOCAPTURE
    else
        currentList = config->audioCaptureDeviceListFor(category, GlobalConfig::ShowUnavailableDevices|GlobalConfig::ShowAdvancedDevices);
#endif

    QList<int> newList;

    for (int i : newOrder) {
        int found = currentList.indexOf(i);
        if (found < 0) {
            // It's not in the list, so something is odd (e.g. client error). Ignore it.
            continue;
        }

        // Iterate through the list from this point onward. If there are hidden devices
        // immediately following, take them too.
        newList.append(currentList.takeAt(found));
        while (found < currentList.size()) {
            bool hidden = true;
            if (output)
                hidden = isHiddenAudioOutputDevice(config, currentList.at(found));
#ifndef QT_NO_PHONON_AUDIOCAPTURE
            else
                hidden = isHiddenAudioCaptureDevice(config, currentList.at(found));
#endif

            if (!hidden)
                break;
            newList.append(currentList.takeAt(found));
        }
    }

    // If there are any devices left in.. just tack them on the end.
    if (currentList.size() > 0)
        newList += currentList;

    /*sb = QString("(Size %1)").arg(newList.size());
    for (int i : newList) {
       sb += QString("%1, ").arg(i);
    }  
    fprintf(stderr, "=== Reindex After  : %s\n", sb.toUtf8().constData());*/
    return newList;
}


void GlobalConfig::setAudioOutputDeviceListFor(Phonon::Category category, QList<int> order)
{
    PulseSupport *pulse = PulseSupport::getInstance();
    if (pulse->isActive()) {
        pulse->setOutputDevicePriorityForCategory(category, order);
        return;
    }

    K_D(GlobalConfig);
    QSettingsGroup backendConfig(&d->config, QLatin1String("AudioOutputDevice")); // + Factory::identifier());

    order = reindexList(this, category, order, true);

    const QList<int> noCategoryOrder = audioOutputDeviceListFor(Phonon::NoCategory, ShowUnavailableDevices|ShowAdvancedDevices);
    if (category != Phonon::NoCategory && order == noCategoryOrder) {
        backendConfig.removeEntry(QLatin1String("Category_") + QString::number(category));
    } else {
        backendConfig.setValue(QLatin1String("Category_") + QString::number(category), order);
    }
}
#endif //QT_NO_PHONON_SETTINGSGROUP

#ifndef QT_NO_PHONON_SETTINGSGROUP
QList<int> GlobalConfig::audioOutputDeviceListFor(Phonon::Category category, int override) const
{
    K_D(const GlobalConfig);

    const bool hide = ((override & AdvancedDevicesFromSettings)
            ? hideAdvancedDevices() : static_cast<bool>(override & HideAdvancedDevices));

    QList<int> defaultList;

    PulseSupport *pulse = PulseSupport::getInstance();
    if (pulse->isActive()) {
        defaultList = pulse->objectDescriptionIndexes(Phonon::AudioOutputDeviceType);

        if (hide || (override & HideUnavailableDevices)) {
            filter(AudioOutputDeviceType, NULL, &defaultList, (hide ? FilterAdvancedDevices : 0)
                    | ((override & HideUnavailableDevices) ? FilterUnavailableDevices : 0) );
        }

    } else {
        BackendInterface *backendIface = qobject_cast<BackendInterface *>(Factory::backend());

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
        if (PlatformPlugin *platformPlugin = Factory::platformPlugin()) {
            // the platform plugin lists the audio devices for the platform
            // this list already is in default order (as defined by the platform plugin)

            defaultList = platformPlugin->objectDescriptionIndexes(Phonon::AudioOutputDeviceType);

            if (hide) {
                QMutableListIterator<int> it(defaultList);

                while (it.hasNext()) {
                    AudioOutputDevice objDesc = AudioOutputDevice::fromIndex(it.next());

                    const QVariant var = objDesc.property("isAdvanced");

                    if (var.isValid() && var.toBool()) {
                        it.remove();
                    }
                }
            }
        }
#endif

        // lookup the available devices directly from the backend
        if (backendIface) {
            // this list already is in default order (as defined by the backend)
            QList<int> list = backendIface->objectDescriptionIndexes(Phonon::AudioOutputDeviceType);

            if (hide || !defaultList.isEmpty() || (override & HideUnavailableDevices)) {
                filter(AudioOutputDeviceType, backendIface, &list,
                        (hide ? FilterAdvancedDevices : 0)
                        // the platform plugin maybe already provided the hardware devices?
                        | (defaultList.isEmpty() ? 0 : FilterHardwareDevices)
                        | ((override & HideUnavailableDevices) ? FilterUnavailableDevices : 0) );
            }

            defaultList += list;
        }
    }

    const QSettingsGroup backendConfig(&d->config, QLatin1String("AudioOutputDevice")); // + Factory::identifier());
    return sortDevicesByCategoryPriority(this, &backendConfig, AudioOutputDeviceType, category, defaultList);
}
#endif //QT_NO_PHONON_SETTINGSGROUP

int GlobalConfig::audioOutputDeviceFor(Phonon::Category category, int override) const
{

#ifndef QT_NO_PHONON_SETTINGSGROUP
    QList<int> ret = audioOutputDeviceListFor(category, override);

    if (! ret.isEmpty()) {
       return ret.first();
    }

#endif

   return -1;
}

#ifndef QT_NO_PHONON_AUDIOCAPTURE
void GlobalConfig::setAudioCaptureDeviceListFor(Phonon::Category category, QList<int> order)
{
#ifndef QT_NO_PHONON_SETTINGSGROUP
    PulseSupport *pulse = PulseSupport::getInstance();
    if (pulse->isActive()) {
        pulse->setCaptureDevicePriorityForCategory(category, order);
        return;
    }

    K_D(GlobalConfig);
    QSettingsGroup backendConfig(&d->config, QLatin1String("AudioCaptureDevice")); // + Factory::identifier());

    order = reindexList(this, category, order, false);

    const QList<int> noCategoryOrder = audioCaptureDeviceListFor(Phonon::NoCategory, ShowUnavailableDevices|ShowAdvancedDevices);
    if (category != Phonon::NoCategory && order == noCategoryOrder) {
        backendConfig.removeEntry(QLatin1String("Category_") + QString::number(category));
    } else {
        backendConfig.setValue(QLatin1String("Category_") + QString::number(category), order);
    }
}

QList<int> GlobalConfig::audioCaptureDeviceListFor(Phonon::Category category, int override) const
{
    K_D(const GlobalConfig);

    const bool hide = ((override & AdvancedDevicesFromSettings)
            ? hideAdvancedDevices()
            : static_cast<bool>(override & HideAdvancedDevices));

    QList<int> defaultList;

    PulseSupport *pulse = PulseSupport::getInstance();
    if (pulse->isActive()) {
        defaultList = pulse->objectDescriptionIndexes(Phonon::AudioCaptureDeviceType);
        if (hide || (override & HideUnavailableDevices)) {
            filter(AudioCaptureDeviceType, NULL, &defaultList,
                    (hide ? FilterAdvancedDevices : 0)
                    | ((override & HideUnavailableDevices) ? FilterUnavailableDevices : 0)
                    );
        }
    } else {
        BackendInterface *backendIface = qobject_cast<BackendInterface *>(Factory::backend());

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
#else //QT_NO_SETTINGSGROUP
    return QList<int>();
#endif //QT_NO_SETTINGSGROUP
        if (PlatformPlugin *platformPlugin = Factory::platformPlugin()) {
            // the platform plugin lists the audio devices for the platform
            // this list already is in default order (as defined by the platform plugin)
            defaultList = platformPlugin->objectDescriptionIndexes(Phonon::AudioCaptureDeviceType);
            if (hide) {
                QMutableListIterator<int> it(defaultList);
                while (it.hasNext()) {
                    AudioCaptureDevice objDesc = AudioCaptureDevice::fromIndex(it.next());
                    const QVariant var = objDesc.property("isAdvanced");
                    if (var.isValid() && var.toBool()) {
                        it.remove();
                    }
                }
            }
        }
#endif //QT_NO_PHONON_PLATFORMPLUGIN

        // lookup the available devices directly from the backend
        if (backendIface) {
            // this list already is in default order (as defined by the backend)
            QList<int> list = backendIface->objectDescriptionIndexes(Phonon::AudioCaptureDeviceType);
            if (hide || !defaultList.isEmpty() || (override & HideUnavailableDevices)) {
                filter(AudioCaptureDeviceType, backendIface, &list,
                        (hide ? FilterAdvancedDevices : 0)
                        // the platform plugin maybe already provided the hardware devices?
                        | (defaultList.isEmpty() ? 0 : FilterHardwareDevices)
                        | ((override & HideUnavailableDevices) ? FilterUnavailableDevices : 0)
                      );
            }
            defaultList += list;
        }
    }

    const QSettingsGroup backendConfig(&d->config, QLatin1String("AudioCaptureDevice")); // + Factory::identifier());
    return sortDevicesByCategoryPriority(this, &backendConfig, AudioCaptureDeviceType, category, defaultList);
}

int GlobalConfig::audioCaptureDeviceFor(Phonon::Category category, int override) const
{
    QList<int> ret = audioCaptureDeviceListFor(category, override);
    if (ret.isEmpty())
        return -1;
    return ret.first();
}
#endif //QT_NO_PHONON_AUDIOCAPTURE


} // namespace Phonon

QT_END_NAMESPACE
