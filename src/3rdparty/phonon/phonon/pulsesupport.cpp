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

#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QEventLoop>
#include <QtCore/QDebug>
#include <QtCore/QStringList>

#ifdef HAVE_PULSEAUDIO
#include <glib.h>
#include <pulse/pulseaudio.h>
#include <pulse/xmalloc.h>
#include <pulse/glib-mainloop.h>
#ifdef HAVE_PULSEAUDIO_DEVICE_MANAGER
#  include <pulse/ext-device-manager.h>
#endif
#endif // HAVE_PULSEAUDIO

#include "pulsesupport.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{

static PulseSupport* s_instance = NULL;

#ifdef HAVE_PULSEAUDIO
/***
* Prints a conditional debug message based on the current debug level
* If obj is provided, classname and objectname will be printed as well
*
* see debugLevel()
*/

static int debugLevel() {
    static int level = -1;
    if (level < 1) {
        level = 0;
        QByteArray pulseenv = qgetenv("PHONON_PULSEAUDIO_DEBUG");
        int l = pulseenv.toInt();
        if (l > 0)
            level = (l > 2 ? 2 : l);
    }
    return level;
}

static void logMessage(const QString &message, int priority = 2, QObject *obj=0);
static void logMessage(const QString &message, int priority, QObject *obj)
{
    if (debugLevel() > 0) {
        QString output;
        if (obj) {
            // Strip away namespace from className
            QByteArray className(obj->metaObject()->className());
            int nameLength = className.length() - className.lastIndexOf(':') - 1;
            className = className.right(nameLength);
            output.sprintf("%s %s (%s %p)", message.toLatin1().constData(), 
                           obj->objectName().toLatin1().constData(), 
                           className.constData(), obj);
        }
        else {
            output = message;
        }
        if (priority <= debugLevel()) {
            qDebug() << QString::fromLatin1("PulseSupport(%1): %2").arg(priority).arg(output);
        }
    }
}


class AudioDevice
{
    public:
        inline
        AudioDevice(QString name, QString desc, QString icon, uint32_t index)
        : pulseName(name), pulseIndex(index)
        {
            properties["name"] = desc;
            properties["description"] = QLatin1String(""); // We don't have descriptions (well we do, but we use them as the name!)
            properties["icon"] = icon;
            properties["available"] = (index != PA_INVALID_INDEX);
            properties["isAdvanced"] = false; // Nothing is advanced!
        }

        // Needed for QMap
        inline AudioDevice() {}

        QString pulseName;
        uint32_t pulseIndex;
        QHash<QByteArray, QVariant> properties;
};
bool operator!=(const AudioDevice &a, const AudioDevice &b)
{
    return !(a.pulseName == b.pulseName && a.properties == b.properties);
}

class PulseUserData
{
    public:
        inline 
        PulseUserData()
        {
        }

        QMap<QString, AudioDevice> newOutputDevices;
        QMap<Phonon::Category, QMap<int, int> > newOutputDevicePriorities; // prio, device

        QMap<QString, AudioDevice> newCaptureDevices;
        QMap<Phonon::Category, QMap<int, int> > newCaptureDevicePriorities; // prio, device
};

static QMap<QString, Phonon::Category> s_roleCategoryMap;

static bool s_pulseActive = false;

static pa_glib_mainloop *s_mainloop = NULL;
static pa_context *s_context = NULL;



static int s_deviceIndexCounter = 0;

static QMap<QString, int> s_outputDeviceIndexes;
static QMap<int, AudioDevice> s_outputDevices;
static QMap<Phonon::Category, QMap<int, int> > s_outputDevicePriorities; // prio, device
static QMap<QString, uint32_t> s_outputStreamIndexMap;

static QMap<QString, int> s_captureDeviceIndexes;
static QMap<int, AudioDevice> s_captureDevices;
static QMap<Phonon::Category, QMap<int, int> > s_captureDevicePriorities; // prio, device
static QMap<QString, uint32_t> s_captureStreamIndexMap;

static void createGenericDevices()
{
    // OK so we don't have the device manager extension, but we can show a single device and fake it.
    int index;
    s_outputDeviceIndexes.clear();
    s_outputDevices.clear();
    s_outputDevicePriorities.clear();
    index = s_deviceIndexCounter++;
    s_outputDeviceIndexes.insert(QLatin1String("sink:default"), index);
    s_outputDevices.insert(index, AudioDevice(QLatin1String("sink:default"), QObject::tr("PulseAudio Sound Server"), QLatin1String("audio-backend-pulseaudio"), 0));
    for (int i = Phonon::NoCategory; i <= Phonon::LastCategory; ++i) {
        Phonon::Category cat = static_cast<Phonon::Category>(i);
        s_outputDevicePriorities[cat].insert(0, index);
    }

    s_captureDeviceIndexes.clear();
    s_captureDevices.clear();
    s_captureDevicePriorities.clear();
    index = s_deviceIndexCounter++;
    s_captureDeviceIndexes.insert(QLatin1String("source:default"), index);
    s_captureDevices.insert(index, AudioDevice(QLatin1String("source:default"), QObject::tr("PulseAudio Sound Server"), QLatin1String("audio-backend-pulseaudio"), 0));
    for (int i = Phonon::NoCategory; i <= Phonon::LastCategory; ++i) {
        Phonon::Category cat = static_cast<Phonon::Category>(i);
        s_captureDevicePriorities[cat].insert(0, index);
    }
}

#ifdef HAVE_PULSEAUDIO_DEVICE_MANAGER
static void ext_device_manager_read_cb(pa_context *c, const pa_ext_device_manager_info *info, int eol, void *userdata) {
    Q_ASSERT(c);
    Q_ASSERT(userdata);

    PulseUserData *u = reinterpret_cast<PulseUserData*>(userdata);

    if (eol < 0) {
        logMessage(QString("Failed to initialize device manager extension: %1").arg(pa_strerror(pa_context_errno(c))));
        logMessage("Falling back to single device mode");
        createGenericDevices();
        delete u;

        // If this is our probe phase, exit now
        if (s_context != c)
            pa_context_disconnect(c);

        return;
    }

    if (eol) {
        // We're done reading the data, so order it by priority and copy it into the
        // static variables where it can then be accessed by those classes that need it.

        QMap<QString, AudioDevice>::iterator newdev_it;

        // Check for new output devices or things changing about known output devices.
        bool output_changed = false;
        for (newdev_it = u->newOutputDevices.begin(); newdev_it != u->newOutputDevices.end(); ++newdev_it) {
            QString name = newdev_it.key();

            // The name + index map is always written when a new device is added.
            Q_ASSERT(s_outputDeviceIndexes.contains(name));

            int index = s_outputDeviceIndexes[name];
            if (!s_outputDevices.contains(index)) {
                // This is a totally new device
                output_changed = true;
                logMessage(QString("Brand New Output Device Found."));
                s_outputDevices.insert(index, *newdev_it);
            } else  if (s_outputDevices[index] != *newdev_it) {
                // We have this device already, but is it different?
                output_changed = true;
                logMessage(QString("Change to Existing Output Device (may be Added/Removed or something else)"));
                s_outputDevices.remove(index);
                s_outputDevices.insert(index, *newdev_it);
            }
        }
        // Go through the output devices we know about and see if any are no longer mentioned in the list.
        QMutableMapIterator<QString, int> output_existing_it(s_outputDeviceIndexes);
        while (output_existing_it.hasNext()) {
            output_existing_it.next();
            if (!u->newOutputDevices.contains(output_existing_it.key())) {
                output_changed = true;
                logMessage(QString("Output Device Completely Removed"));
                s_outputDevices.remove(output_existing_it.value());
                output_existing_it.remove();
            }
        }

        // Check for new capture devices or things changing about known capture devices.
        bool capture_changed = false;
        for (newdev_it = u->newCaptureDevices.begin(); newdev_it != u->newCaptureDevices.end(); ++newdev_it) {
            QString name = newdev_it.key();

            // The name + index map is always written when a new device is added.
            Q_ASSERT(s_captureDeviceIndexes.contains(name));

            int index = s_captureDeviceIndexes[name];
            if (!s_captureDevices.contains(index)) {
                // This is a totally new device
                capture_changed = true;
                logMessage(QString("Brand New Capture Device Found."));
                s_captureDevices.insert(index, *newdev_it);
            } else  if (s_captureDevices[index] != *newdev_it) {
                // We have this device already, but is it different?
                capture_changed = true;
                logMessage(QString("Change to Existing Capture Device (may be Added/Removed or something else)"));
                s_captureDevices.remove(index);
                s_captureDevices.insert(index, *newdev_it);
            }
        }
        // Go through the capture devices we know about and see if any are no longer mentioned in the list.
        QMutableMapIterator<QString, int> capture_existing_it(s_captureDeviceIndexes);
        while (capture_existing_it.hasNext()) {
            capture_existing_it.next();
            if (!u->newCaptureDevices.contains(capture_existing_it.key())) {
                capture_changed = true;
                logMessage(QString("Capture Device Completely Removed"));
                s_captureDevices.remove(capture_existing_it.value());
                capture_existing_it.remove();
            }
        }

        // Just copy accross the new priority lists as we know they are valid
        if (s_outputDevicePriorities != u->newOutputDevicePriorities) {
            output_changed = true;
            s_outputDevicePriorities = u->newOutputDevicePriorities;
        }
        if (s_captureDevicePriorities != u->newCaptureDevicePriorities) {
            capture_changed = true;
            s_captureDevicePriorities = u->newCaptureDevicePriorities;
        }

        if (s_instance) {
            // This wont be emitted durring the connection probe phase
            // which is intensional
            if (output_changed)
                s_instance->emitObjectDescriptionChanged(AudioOutputDeviceType);
            if (capture_changed)
                s_instance->emitObjectDescriptionChanged(AudioCaptureDeviceType);
        }

        // We can free the user data as we will not be called again.
        delete u;

        // Some debug
        logMessage(QString("Output Device Priority List:"));
        for (int i = Phonon::NoCategory; i <= Phonon::LastCategory; ++i) {
            Phonon::Category cat = static_cast<Phonon::Category>(i);
            if (s_outputDevicePriorities.contains(cat)) {
                logMessage(QString("  Phonon Category %1").arg(cat));
                int count = 0;
                for (int j : s_outputDevicePriorities[cat]) {
                    QHash<QByteArray, QVariant> &props = s_outputDevices[j].properties;
                    logMessage(QString("    %1. %2 (Available: %3)").arg(++count).arg(props["name"].toString()).arg(props["available"].toBool()));
                }
            }
        }
        logMessage(QString("Capture Device Priority List:"));
        for (int i = Phonon::NoCategory; i <= Phonon::LastCategory; ++i) {
            Phonon::Category cat = static_cast<Phonon::Category>(i);
            if (s_captureDevicePriorities.contains(cat)) {
                logMessage(QString("  Phonon Category %1").arg(cat));
                int count = 0;
                for (int j : s_captureDevicePriorities[cat]) {
                    QHash<QByteArray, QVariant> &props = s_captureDevices[j].properties;
                    logMessage(QString("    %1. %2 (Available: %3)").arg(++count).arg(props["name"].toString()).arg(props["available"].toBool()));
                }
            }
        }

        // If this is our probe phase, exit now as we're finished reading
        // our device info and can exit and reconnect
        if (s_context != c)
            pa_context_disconnect(c);
    }

    if (!info)
        return;

    Q_ASSERT(info->name);
    Q_ASSERT(info->description);
    Q_ASSERT(info->icon);

    // QString wrapper
    QString name(info->name);
    int index;
    QMap<Phonon::Category, QMap<int, int> > *new_prio_map_cats; // prio, device
    QMap<QString, AudioDevice> *new_devices;

    if (name.startsWith("sink:")) {
        new_devices = &u->newOutputDevices;
        new_prio_map_cats = &u->newOutputDevicePriorities;

        if (s_outputDeviceIndexes.contains(name))
            index = s_outputDeviceIndexes[name];
        else
            index = s_outputDeviceIndexes[name] = s_deviceIndexCounter++;
    } else if (name.startsWith("source:")) {
        new_devices = &u->newCaptureDevices;
        new_prio_map_cats = &u->newCaptureDevicePriorities;

        if (s_captureDeviceIndexes.contains(name))
            index = s_captureDeviceIndexes[name];
        else
            index = s_captureDeviceIndexes[name] = s_deviceIndexCounter++;
    } else {
        // This indicates a bug in pulseaudio.
        return;
    }

    // Add the new device itself.
    new_devices->insert(name, AudioDevice(name, info->description, info->icon, info->index));

    // For each role in the priority, map it to a phonon category and store the order.
    for (uint32_t i = 0; i < info->n_role_priorities; ++i) {
        pa_ext_device_manager_role_priority_info* role_prio = &info->role_priorities[i];
        Q_ASSERT(role_prio->role);

        if (s_roleCategoryMap.contains(role_prio->role)) {
            Phonon::Category cat = s_roleCategoryMap[role_prio->role];

            (*new_prio_map_cats)[cat].insert(role_prio->priority, index);
        }
    }
}

static void ext_device_manager_subscribe_cb(pa_context *c, void *) {
    Q_ASSERT(c);

    pa_operation *o;
    PulseUserData *u = new PulseUserData;
    if (!(o = pa_ext_device_manager_read(c, ext_device_manager_read_cb, u))) {
        logMessage(QString("pa_ext_device_manager_read() failed."));
        delete u;
        return;
    }
    pa_operation_unref(o);
}
#endif

void sink_input_cb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata) {
    Q_UNUSED(userdata);
    Q_ASSERT(c);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        logMessage(QLatin1String("Sink input callback failure"));
        return;
    }

    if (eol > 0)
        return;

    Q_ASSERT(i);

    // loop through (*i) and extract phonon->streamindex...
    const char *t;
    if ((t = pa_proplist_gets(i->proplist, "phonon.streamid"))) {
        logMessage(QString::fromLatin1("Found PulseAudio stream index %1 for Phonon Output Stream %2").arg(i->index).arg(QLatin1String(t)));
        s_outputStreamIndexMap[QLatin1String(t)] = i->index;

        // Find the sink's phonon index and notify whoever cares...
        if (PA_INVALID_INDEX != i->sink) {
            bool found = false;
            int device;
            QMap<int, AudioDevice>::iterator it;
            for (it = s_outputDevices.begin(); it != s_outputDevices.end(); ++it) {
                if ((*it).pulseIndex == i->sink) {
                    found = true;
                    device = it.key();
                    break;
                }
            }
            if (found) {
                // OK so we just emit our signal
                logMessage(QLatin1String("Letting the rest of phonon know about this"));
                s_instance->emitUsingDevice(QLatin1String(t), device);
            }
        }
    }
}

void source_output_cb(pa_context *c, const pa_source_output_info *i, int eol, void *userdata) {
    Q_UNUSED(userdata);
    Q_ASSERT(c);

    if (eol < 0) {
        if (pa_context_errno(c) == PA_ERR_NOENTITY)
            return;

        logMessage(QLatin1String("Source output callback failure"));
        return;
    }

    if (eol > 0)
        return;

    Q_ASSERT(i);

    // loop through (*i) and extract phonon->streamindex...
    const char *t;
    if ((t = pa_proplist_gets(i->proplist, "phonon.streamid"))) {
        logMessage(QString::fromLatin1("Found PulseAudio stream index %1 for Phonon Capture Stream %2").arg(i->index).arg(QLatin1String(t)));
        s_captureStreamIndexMap[QLatin1String(t)] = i->index;

        // Find the source's phonon index and notify whoever cares...
        if (PA_INVALID_INDEX != i->source) {
            bool found = false;
            int device;
            QMap<int, AudioDevice>::iterator it;
            for (it = s_captureDevices.begin(); it != s_captureDevices.end(); ++it) {
                if ((*it).pulseIndex == i->source) {
                    found = true;
                    device = it.key();
                    break;
                }
            }
            if (found) {
                // OK so we just emit our signal
                logMessage(QLatin1String("Letting the rest of phonon know about this"));
                s_instance->emitUsingDevice(QLatin1String(t), device);
            }
        }
    }
}

static void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    Q_UNUSED(userdata);

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                QString phononid = s_outputStreamIndexMap.key(index);
                if (!phononid.isEmpty()) {
                    if (s_outputStreamIndexMap.contains(phononid)) {
                        logMessage(QString::fromLatin1("Phonon Output Stream %1 is gone at the PA end. Marking it as invalid in our cache as we may reuse it.").arg(phononid));
                        s_outputStreamIndexMap[phononid] = PA_INVALID_INDEX;
                    } else {
                        logMessage(QString::fromLatin1("Removing Phonon Output Stream %1 (it's gone!)").arg(phononid));
                        s_outputStreamIndexMap.remove(phononid);
                    }
                }
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_sink_input_info(c, index, sink_input_cb, NULL))) {
                    logMessage(QLatin1String("pa_context_get_sink_input_info() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;

        case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
                QString phononid = s_captureStreamIndexMap.key(index);
                if (!phononid.isEmpty()) {
                    if (s_captureStreamIndexMap.contains(phononid)) {
                        logMessage(QString::fromLatin1("Phonon Capture Stream %1 is gone at the PA end. Marking it as invalid in our cache as we may reuse it.").arg(phononid));
                        s_captureStreamIndexMap[phononid] = PA_INVALID_INDEX;
                    } else {
                        logMessage(QString::fromLatin1("Removing Phonon Capture Stream %1 (it's gone!)").arg(phononid));
                        s_captureStreamIndexMap.remove(phononid);
                    }
                }
            } else {
                pa_operation *o;
                if (!(o = pa_context_get_source_output_info(c, index, source_output_cb, NULL))) {
                    logMessage(QLatin1String("pa_context_get_sink_input_info() failed"));
                    return;
                }
                pa_operation_unref(o);
            }
            break;
    }
}


static QString statename(pa_context_state_t state)
{
    switch (state)
    {
        case PA_CONTEXT_UNCONNECTED:  return QLatin1String("Unconnected");
        case PA_CONTEXT_CONNECTING:   return QLatin1String("Connecting");
        case PA_CONTEXT_AUTHORIZING:  return QLatin1String("Authorizing");
        case PA_CONTEXT_SETTING_NAME: return QLatin1String("Setting Name");
        case PA_CONTEXT_READY:        return QLatin1String("Ready");
        case PA_CONTEXT_FAILED:       return QLatin1String("Failed");
        case PA_CONTEXT_TERMINATED:   return QLatin1String("Terminated");
    }

    return QString::fromLatin1("Unknown state: %0").arg(state);
}

static void context_state_callback(pa_context *c, void *)
{
    Q_ASSERT(c);

    logMessage(QString::fromLatin1("context_state_callback %1").arg(statename(pa_context_get_state(c))));
    pa_context_state_t state = pa_context_get_state(c);
    if (state == PA_CONTEXT_READY) {
        // We've connected to PA, so it is active
        s_pulseActive = true;

        // Attempt to load things up
        pa_operation *o;

        // 1. Register for the stream changes (except during probe)
        if (s_context == c) {
            pa_context_set_subscribe_callback(c, subscribe_cb, NULL);

            if (!(o = pa_context_subscribe(c, (pa_subscription_mask_t)
                                              (PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                               PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT), NULL, NULL))) {
                logMessage(QLatin1String("pa_context_subscribe() failed"));
                return;
            }
            pa_operation_unref(o);
        }

#ifdef HAVE_PULSEAUDIO_DEVICE_MANAGER
        // 2a. Attempt to initialise Device Manager info (except during probe)
        if (s_context == c) {
            pa_ext_device_manager_set_subscribe_cb(c, ext_device_manager_subscribe_cb, NULL);
            if (!(o = pa_ext_device_manager_subscribe(c, 1, NULL, NULL))) {
                logMessage(QString("pa_ext_device_manager_subscribe() failed"));
                return;
            }
            pa_operation_unref(o);
        }

        // 3. Attempt to read info from Device Manager
        PulseUserData *u = new PulseUserData;
        if (!(o = pa_ext_device_manager_read(c, ext_device_manager_read_cb, u))) {
            logMessage(QString("pa_ext_device_manager_read() failed. Attempting to continue without device manager support"));
            createGenericDevices();
            delete u;

            // If this is our probe phase, exit immediately
            if (s_context != c)
                pa_context_disconnect(c);

            return;
        }
        pa_operation_unref(o);

#else
        // If we know do not have Device Manager support, we just create our dummy devices now
        createGenericDevices();

        // If this is our probe phase, exit immediately
        if (s_context != c)
            pa_context_disconnect(c);
#endif
    } else if (!PA_CONTEXT_IS_GOOD(state)) {
        /// @todo Deal with reconnection...
        //logMessage("Connection to PulseAudio lost");

        // If this is our probe phase, exit our context immediately
        if (s_context != c)
            pa_context_disconnect(c);
    }
}
#endif // HAVE_PULSEAUDIO


PulseSupport* PulseSupport::getInstance()
{
    if (NULL == s_instance) {
        s_instance = new PulseSupport();
    }
    return s_instance;
}

void PulseSupport::shutdown()
{
    if (NULL != s_instance) {
        delete s_instance;
        s_instance = NULL;
    }
}

PulseSupport::PulseSupport()
 : QObject(), mEnabled(false)
{
#ifdef HAVE_PULSEAUDIO
    // Initialise our map (is there a better way to do this?)
    s_roleCategoryMap[QLatin1String("none")] = Phonon::NoCategory;
    s_roleCategoryMap[QLatin1String("video")] = Phonon::VideoCategory;
    s_roleCategoryMap[QLatin1String("music")] = Phonon::MusicCategory;
    s_roleCategoryMap[QLatin1String("game")] = Phonon::GameCategory;
    s_roleCategoryMap[QLatin1String("event")] = Phonon::NotificationCategory;
    s_roleCategoryMap[QLatin1String("phone")] = Phonon::CommunicationCategory;
    //s_roleCategoryMap[QLatin1String("animation")]; // No Mapping
    //s_roleCategoryMap[QLatin1String("production")]; // No Mapping
    s_roleCategoryMap[QLatin1String("a11y")] = Phonon::AccessibilityCategory;

    // To allow for easy debugging, give an easy way to disable this pulseaudio check
    QByteArray pulseenv = qgetenv("PHONON_PULSEAUDIO_DISABLE");
    if (pulseenv.toInt()) {
        logMessage(QLatin1String("PulseAudio support disabled: PHONON_PULSEAUDIO_DISABLE is set"));
        return;
    }

    // We require a glib event loop
    if (strcmp(QAbstractEventDispatcher::instance()->metaObject()->className(),
            "QGuiEventDispatcherGlib") != 0) {
        logMessage(QLatin1String("Disabling PulseAudio integration for lack of GLib event loop."));
        return;
    }

    // First of all conenct to PA via simple/blocking means and if that succeeds,
    // use a fully async integrated mainloop method to connect and get proper support.
    pa_mainloop *p_test_mainloop;
    if (!(p_test_mainloop = pa_mainloop_new())) {
        logMessage(QLatin1String("PulseAudio support disabled: Unable to create mainloop"));
        return;
    }

    pa_context *p_test_context;
    if (!(p_test_context = pa_context_new(pa_mainloop_get_api(p_test_mainloop), "libphonon-probe"))) {
        logMessage(QLatin1String("PulseAudio support disabled: Unable to create context"));
        pa_mainloop_free(p_test_mainloop);
        return;
    }

    logMessage(QLatin1String("Probing for PulseAudio..."));
    // (cg) Convert to PA_CONTEXT_NOFLAGS when PulseAudio 0.9.19 is required
    if (pa_context_connect(p_test_context, NULL, static_cast<pa_context_flags_t>(0), NULL) < 0) {
        logMessage(QString::fromLatin1("PulseAudio support disabled: %1").arg(QString::fromLocal8Bit(pa_strerror(pa_context_errno(p_test_context)))));
        pa_context_disconnect(p_test_context);
        pa_context_unref(p_test_context);
        pa_mainloop_free(p_test_mainloop);
        return;
    }

    pa_context_set_state_callback(p_test_context, &context_state_callback, NULL);
    for (;;) {
        pa_mainloop_iterate(p_test_mainloop, 1, NULL);

        if (!PA_CONTEXT_IS_GOOD(pa_context_get_state(p_test_context))) {
            logMessage(QLatin1String("PulseAudio probe complete."));
            break;
        }
    }
    pa_context_disconnect(p_test_context);
    pa_context_unref(p_test_context);
    pa_mainloop_free(p_test_mainloop);

    if (!s_pulseActive) {
        logMessage(QLatin1String("PulseAudio support is not available."));
        return;
    }

    // If we're still here, PA is available.
    logMessage(QLatin1String("PulseAudio support enabled"));

    // Now we connect for real using a proper main loop that we can forget
    // all about processing.
    s_mainloop = pa_glib_mainloop_new(NULL);
    Q_ASSERT(s_mainloop);
    pa_mainloop_api *api = pa_glib_mainloop_get_api(s_mainloop);

    s_context = pa_context_new(api, "libphonon");
    // (cg) Convert to PA_CONTEXT_NOFLAGS when PulseAudio 0.9.19 is required
    if (pa_context_connect(s_context, NULL, static_cast<pa_context_flags_t>(0), 0) >= 0)
        pa_context_set_state_callback(s_context, &context_state_callback, NULL);
#endif
}

PulseSupport::~PulseSupport()
{
#ifdef HAVE_PULSEAUDIO
    if (s_context) {
        pa_context_disconnect(s_context);
        s_context = NULL;
    }

    if (s_mainloop) {
        pa_glib_mainloop_free(s_mainloop);
        s_mainloop = NULL;
    }
#endif
}

bool PulseSupport::isActive()
{
#ifdef HAVE_PULSEAUDIO
    return mEnabled && s_pulseActive;
#else
    return false;
#endif
}

void PulseSupport::enable(bool enabled)
{
    mEnabled = enabled;
}

QList<int> PulseSupport::objectDescriptionIndexes(ObjectDescriptionType type) const
{
    QList<int> list;

    if (type != AudioOutputDeviceType && type != AudioCaptureDeviceType)
        return list;

#ifdef HAVE_PULSEAUDIO
    if (s_pulseActive) {
        switch (type) {

            case AudioOutputDeviceType: {
                QMap<QString, int>::iterator it;
                for (it = s_outputDeviceIndexes.begin(); it != s_outputDeviceIndexes.end(); ++it) {
                    list.append(*it);
                }
                break;
            }
            case AudioCaptureDeviceType: {
                QMap<QString, int>::iterator it;
                for (it = s_captureDeviceIndexes.begin(); it != s_captureDeviceIndexes.end(); ++it) {
                    list.append(*it);
                }
                break;
            }
            default:
                break;
        }
    }
#endif

    return list;
}

QHash<QByteArray, QVariant> PulseSupport::objectDescriptionProperties(ObjectDescriptionType type, int index) const
{
    QHash<QByteArray, QVariant> ret;

    if (type != AudioOutputDeviceType && type != AudioCaptureDeviceType)
        return ret;

#ifndef HAVE_PULSEAUDIO
    Q_UNUSED(index);
#else
    if (s_pulseActive) {
        switch (type) {

            case AudioOutputDeviceType:
                Q_ASSERT(s_outputDevices.contains(index));
                ret = s_outputDevices[index].properties;
                break;

            case AudioCaptureDeviceType:
                Q_ASSERT(s_captureDevices.contains(index));
                ret = s_captureDevices[index].properties;
                break;

            default:
                break;
        }
    }
#endif

    return ret;
}

QList<int> PulseSupport::objectIndexesByCategory(ObjectDescriptionType type, Category category) const
{
    QList<int> ret;

    if (type != AudioOutputDeviceType && type != AudioCaptureDeviceType)
        return ret;

#ifndef HAVE_PULSEAUDIO
    Q_UNUSED(category);
#else
    if (s_pulseActive) {
        switch (type) {

            case AudioOutputDeviceType:
                if (s_outputDevicePriorities.contains(category))
                    ret = s_outputDevicePriorities[category].values();
                break;

            case AudioCaptureDeviceType:
                if (s_captureDevicePriorities.contains(category))
                    ret = s_captureDevicePriorities[category].values();
                break;

            default:
                break;
        }
    }
#endif

    return ret;
}

#ifdef HAVE_PULSEAUDIO
static void setDevicePriority(Category category, QStringList list)
{
    QString role = s_roleCategoryMap.key(category);
    if (role.isEmpty())
        return;

    logMessage(QString::fromLatin1("Reindexing %1: %2").arg(role).arg(list.join(QLatin1String(", "))));

    char **devices;
    devices = pa_xnew(char *, list.size()+1);
    int i = 0;
    for (QString str : list) {
        devices[i++] = pa_xstrdup(str.toUtf8().constData());
    }
    devices[list.size()] = NULL;

#ifdef HAVE_PULSEAUDIO_DEVICE_MANAGER 
    pa_operation *o;
    if (!(o = pa_ext_device_manager_reorder_devices_for_role(s_context, role.toUtf8().constData(), (const char**)devices, NULL, NULL)))
        logMessage(QString("pa_ext_device_manager_reorder_devices_for_role() failed"));
    else
        pa_operation_unref(o);
#endif

    for (i = 0; i < list.size(); ++i)
        pa_xfree(devices[i]);
    pa_xfree(devices);
}
#endif

void PulseSupport::setOutputDevicePriorityForCategory(Category category, QList<int> order)
{
#ifndef HAVE_PULSEAUDIO
    Q_UNUSED(category);
    Q_UNUSED(order);
#else
    QStringList list;
    QList<int>::iterator it;

    for (it = order.begin(); it != order.end(); ++it) {
        if (s_outputDevices.contains(*it)) {
            list << s_outputDeviceIndexes.key(*it);
        }
    }
    setDevicePriority(category, list);
#endif
}

void PulseSupport::setCaptureDevicePriorityForCategory(Category category, QList<int> order)
{
#ifndef HAVE_PULSEAUDIO
    Q_UNUSED(category);
    Q_UNUSED(order);
#else
    QStringList list;
    QList<int>::iterator it;

    for (it = order.begin(); it != order.end(); ++it) {
        if (s_captureDevices.contains(*it)) {
            list << s_captureDeviceIndexes.key(*it);
        }
    }
    setDevicePriority(category, list);
#endif
}

void PulseSupport::setStreamPropList(Category category, QString streamUuid)
{
#ifndef HAVE_PULSEAUDIO
    Q_UNUSED(category);
    Q_UNUSED(streamUuid);
#else
    QString role = s_roleCategoryMap.key(category);
    if (role.isEmpty())
        return;

    logMessage(QString::fromLatin1("Setting role to %1 for streamindex %2").arg(role).arg(streamUuid));
    setenv("PULSE_PROP_media.role", role.toLatin1().constData(), 1);
    setenv("PULSE_PROP_phonon.streamid", streamUuid.toLatin1().constData(), 1);
#endif
}

void PulseSupport::emitObjectDescriptionChanged(ObjectDescriptionType type)
{
    emit objectDescriptionChanged(type);
}

void PulseSupport::emitUsingDevice(QString streamUuid, int device)
{
    emit usingDevice(streamUuid, device);
}

bool PulseSupport::setOutputDevice(QString streamUuid, int device) {
#ifndef HAVE_PULSEAUDIO
    Q_UNUSED(streamUuid);
    Q_UNUSED(device);
    return false;
#else
    if (s_outputDevices.size() < 2)
        return true;

    if (!s_outputDevices.contains(device)) {
        logMessage(QString::fromLatin1("Attempting to set Output Device for invalid device id %1.").arg(device));
        return false;
    }
    const QVariant var = s_outputDevices[device].properties["name"];
    logMessage(QString::fromLatin1("Attempting to set Output Device to '%1' for Output Stream %2").arg(var.toString()).arg(streamUuid));

    // Attempt to look up the pulse stream index.
    if (s_outputStreamIndexMap.contains(streamUuid) && s_outputStreamIndexMap[streamUuid] != PA_INVALID_INDEX) {
        logMessage(QLatin1String("... Found in map. Moving now"));

        uint32_t pulse_device_index = s_outputDevices[device].pulseIndex;
        uint32_t pulse_stream_index = s_outputStreamIndexMap[streamUuid];

        logMessage(QString::fromLatin1("Moving Pulse Sink Input %1 to '%2' (Pulse Sink %3)").arg(pulse_stream_index).arg(var.toString()).arg(pulse_device_index));

        /// @todo Find a way to move the stream without saving it... We don't want to pollute the stream restore db.
        pa_operation* o;
        if (!(o = pa_context_move_sink_input_by_index(s_context, pulse_stream_index, pulse_device_index, NULL, NULL))) {
            logMessage(QLatin1String("pa_context_move_sink_input_by_index() failed"));
            return false;
        }
        pa_operation_unref(o);
    } else {
        logMessage(QLatin1String("... Not found in map. We will be notified of the device when the stream appears and we can process any moves needed then"));
    }
    return true;
#endif
}

bool PulseSupport::setCaptureDevice(QString streamUuid, int device) {
#ifndef HAVE_PULSEAUDIO
    Q_UNUSED(streamUuid);
    Q_UNUSED(device);
    return false;
#else
    if (s_captureDevices.size() < 2)
        return true;

    if (!s_captureDevices.contains(device)) {
        logMessage(QString::fromLatin1("Attempting to set Capture Device for invalid device id %1.").arg(device));
        return false;
    }
    const QVariant var = s_captureDevices[device].properties["name"];
    logMessage(QString::fromLatin1("Attempting to set Capture Device to '%1' for Capture Stream %2").arg(var.toString()).arg(streamUuid));

    // Attempt to look up the pulse stream index.
    if (s_captureStreamIndexMap.contains(streamUuid) && s_captureStreamIndexMap[streamUuid] == PA_INVALID_INDEX) {
        logMessage(QString::fromLatin1("... Found in map. Moving now"));

        uint32_t pulse_device_index = s_captureDevices[device].pulseIndex;
        uint32_t pulse_stream_index = s_captureStreamIndexMap[streamUuid];

        logMessage(QString::fromLatin1("Moving Pulse Source Output %1 to '%2' (Pulse Sink %3)").arg(pulse_stream_index).arg(var.toString()).arg(pulse_device_index));

        /// @todo Find a way to move the stream without saving it... We don't want to pollute the stream restore db.
        pa_operation* o;
        if (!(o = pa_context_move_source_output_by_index(s_context, pulse_stream_index, pulse_device_index, NULL, NULL))) {
            logMessage(QString::fromLatin1("pa_context_move_source_output_by_index() failed"));
            return false;
        }
        pa_operation_unref(o);
    } else {
        logMessage(QString::fromLatin1("... Not found in map. We will be notified of the device when the stream appears and we can process any moves needed then"));
    }
    return true;
#endif
}

void PulseSupport::clearStreamCache(QString streamUuid) {
#ifndef HAVE_PULSEAUDIO
    Q_UNUSED(streamUuid);
    return;
#else
    logMessage(QString::fromLatin1("Clearing stream cache for stream %1").arg(streamUuid));
    s_outputStreamIndexMap.remove(streamUuid);
    s_captureStreamIndexMap.remove(streamUuid);
#endif
}

} // namespace Phonon

QT_END_NAMESPACE
