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

#include "common.h"
#include "backend.h"
#include "audiooutput.h"
#include "audiodataoutput.h"
#include "audioeffect.h"
#include "mediaobject.h"
#include "videowidget.h"
#include "devicemanager.h"
#include "effectmanager.h"
#include "message.h"
#include "volumefadereffect.h"
#include <gst/interfaces/propertyprobe.h>
#include <phonon/pulsesupport.h>

#include <QtCore/QSet>
#include <QtCore/QVariant>
#include <QtCore/QtPlugin>

QT_BEGIN_NAMESPACE

Q_EXPORT_PLUGIN2(phonon_gstreamer, Phonon::Gstreamer::Backend)

namespace Phonon
{
namespace Gstreamer
{

class MediaNode;

Backend::Backend(QObject *parent, const QVariantList &)
        : QObject(parent)
        , m_deviceManager(0)
        , m_effectManager(0)
        , m_debugLevel(Warning)
        , m_isValid(false)
{
    // Initialise PulseAudio support
    PulseSupport *pulse = PulseSupport::getInstance();
    pulse->enable();

    connect(pulse, SIGNAL(objectDescriptionChanged(ObjectDescriptionType)), this, SLOT(objectDescriptionChanged(ObjectDescriptionType)));

    // In order to support reloading, we only set the app name once...
    static bool first = true;
    if (first) {
        first = false;
        g_set_application_name(qApp->applicationName().toUtf8().constData());
    }
    GError *err = 0;
    bool wasInit = gst_init_check(0, 0, &err);  //init gstreamer: must be called before any gst-related functions
    if (err)
        g_error_free(err);

    qRegisterMetaType<Message>("Message");
#ifndef QT_NO_PROPERTIES
    setProperty("identifier",     QLatin1String("phonon_gstreamer"));
    setProperty("backendName",    QLatin1String("Gstreamer"));
    setProperty("backendComment", QLatin1String("Gstreamer plugin for Phonon"));
    setProperty("backendVersion", QLatin1String("0.2"));
    setProperty("backendWebsite", QLatin1String("http://qt.nokia.com/"));
#endif //QT_NO_PROPERTIES

    //check if we should enable debug output
    QString debugLevelString = qgetenv("PHONON_GST_DEBUG");
    int debugLevel = debugLevelString.toInteger<int>();

    if (debugLevel > 3) //3 is maximum
        debugLevel = 3;

    m_debugLevel = (DebugLevel)debugLevel;

    if (wasInit) {
        m_isValid = checkDependencies();
        gchar *versionString = gst_version_string();
        logMessage(QString("Using %0").formatArg(QString::fromLatin1(versionString)));
        g_free(versionString);
    }

    if (! m_isValid) {
        qWarning("Phonon::GStreamer::Backend: Failed to initialize GStreamer");
    }

    m_deviceManager = new DeviceManager(this);
    m_effectManager = new EffectManager(this);
}

Backend::~Backend()
{
    delete m_effectManager;
    delete m_deviceManager;
    PulseSupport::shutdown();
}

gboolean Backend::busCall(GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_UNUSED(bus);
    Q_ASSERT(msg);

    MediaObject *mediaObject = static_cast<MediaObject*>(data);
    Q_ASSERT(mediaObject);

    Message message(msg, mediaObject);
    QMetaObject::invokeMethod(mediaObject->backend(), "handleBusMessage", Qt::QueuedConnection, Q_ARG(Message, message));

    return true;
}

/***
 * !reimp
 */
QObject *Backend::createObject(BackendInterface::Class c, QObject *parent, const QList<QVariant> &args)
{
    // Return nothing if dependencies are not met

    switch (c) {
    case MediaObjectClass:
        return new MediaObject(this, parent);

    case AudioOutputClass:
        return new AudioOutput(this, parent);

#ifndef QT_NO_PHONON_EFFECT
    case EffectClass:
        return new AudioEffect(this, args[0].toInt(), parent);
#endif //QT_NO_PHONON_EFFECT
    case AudioDataOutputClass:
        return new AudioDataOutput(this, parent);

#ifndef QT_NO_PHONON_VIDEO
    case VideoDataOutputClass:
        logMessage("createObject() : VideoDataOutput not implemented");
        break;

    case VideoWidgetClass: {
            QWidget *widget =  qobject_cast<QWidget*>(parent);
            return new VideoWidget(this, widget);
        }
#endif //QT_NO_PHONON_VIDEO
#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT
    case VolumeFaderEffectClass:
        return new VolumeFaderEffect(this, parent);
#endif //QT_NO_PHONON_VOLUMEFADEREFFECT

    case VisualizationClass:  //Fall through
    default:
        logMessage("createObject() : Backend object not available");
    }
    return 0;
}

// Returns true if all dependencies are met
// and gstreamer is usable, otherwise false
bool Backend::isValid() const
{
    return m_isValid;
}

bool Backend::supportsVideo() const
{
    return isValid();
}

bool Backend::checkDependencies() const
{
    bool success = false;
    // Verify that gst-plugins-base is installed
    GstElementFactory *acFactory = gst_element_factory_find ("audioconvert");
    if (acFactory) {
        gst_object_unref(acFactory);
        success = true;
        // Check if gst-plugins-good is installed
        GstElementFactory *csFactory = gst_element_factory_find ("videobalance");
        if (csFactory) {
            gst_object_unref(csFactory);
        } else {
            QString message = tr("Warning: You do not seem to have the package gstreamer0.10-plugins-good installed.\n"
                                 "          Some video features have been disabled.");
            qDebug() << message;
        }
    } else {
        qWarning() << tr("Warning: You do not seem to have the base GStreamer plugins installed.\n"
                         "          All audio and video support has been disabled");
    }
    return success;
}

/***
 * !reimp
 */
QStringList Backend::availableMimeTypes() const
{
    QStringList availableMimeTypes;

    if (!isValid())
        return availableMimeTypes;

    GstElementFactory *mpegFactory;
    // Add mp3 as a separate mime type as people are likely to look for it.
    if ((mpegFactory = gst_element_factory_find ("ffmpeg")) ||
        (mpegFactory = gst_element_factory_find ("mad"))) {
        availableMimeTypes << QLatin1String("audio/x-mp3");
        gst_object_unref(GST_OBJECT(mpegFactory));
    }

    // Iterate over all audio and video decoders and extract mime types from sink caps
    GList* factoryList = gst_registry_get_feature_list(gst_registry_get_default (), GST_TYPE_ELEMENT_FACTORY);

    for (GList* iter = g_list_first(factoryList) ; iter != NULL ; iter = g_list_next(iter)) {

        GstPluginFeature *feature = GST_PLUGIN_FEATURE(iter->data);
        QString klass = QString::fromLatin1(gst_element_factory_get_klass(GST_ELEMENT_FACTORY(feature)));

        if (klass == QLatin1String("Codec/Decoder") ||
            klass == QLatin1String("Codec/Decoder/Audio") ||
            klass == QLatin1String("Codec/Decoder/Video") ||
            klass == QLatin1String("Codec/Demuxer") ||
            klass == QLatin1String("Codec/Demuxer/Audio") ||
            klass == QLatin1String("Codec/Demuxer/Video") ||
            klass == QLatin1String("Codec/Parser") ||
            klass == QLatin1String("Codec/Parser/Audio") ||
            klass == QLatin1String("Codec/Parser/Video")) {

            const GList *static_templates;
            GstElementFactory *factory = GST_ELEMENT_FACTORY(feature);
            static_templates = gst_element_factory_get_static_pad_templates(factory);

            for (; static_templates != NULL ; static_templates = static_templates->next) {
                GstStaticPadTemplate *padTemplate = (GstStaticPadTemplate *) static_templates->data;

                if (padTemplate && padTemplate->direction == GST_PAD_SINK) {
                    GstCaps *caps = gst_static_pad_template_get_caps (padTemplate);

                    if (caps) {
                        for (unsigned int struct_idx = 0; struct_idx < gst_caps_get_size (caps); struct_idx++) {

                            const GstStructure* capsStruct = gst_caps_get_structure (caps, struct_idx);
                            QString mime = QString::fromUtf8(gst_structure_get_name (capsStruct));

                            if (! availableMimeTypes.contains(mime))
                                availableMimeTypes.append(mime);
                        }
                    }
                }
            }
        }
    }

    g_list_free(factoryList);

    if (availableMimeTypes.contains("audio/x-vorbis")
        && availableMimeTypes.contains("application/x-ogm-audio")) {
        if (!availableMimeTypes.contains("audio/x-vorbis+ogg"))
            availableMimeTypes.append("audio/x-vorbis+ogg");
        if (!availableMimeTypes.contains("application/ogg"))  /* *.ogg */
            availableMimeTypes.append("application/ogg");
        if (!availableMimeTypes.contains("audio/ogg")) /* *.oga */
            availableMimeTypes.append("audio/ogg");
    }
    availableMimeTypes.sort();
    return availableMimeTypes;
}

/***
 * !reimp
 */
QList<int> Backend::objectDescriptionIndexes(ObjectDescriptionType type) const
{
    QList<int> list;

    if (! isValid())
        return list;

    switch (type) {
    case Phonon::AudioOutputDeviceType: {
            QList<AudioDevice> deviceList = deviceManager()->audioOutputDevices();
            for (int dev = 0 ; dev < deviceList.size() ; ++dev)
                list.append(deviceList[dev].id);
            break;
        }
        break;

    case Phonon::EffectType: {
            QList<EffectInfo*> effectList = effectManager()->audioEffects();
            for (int eff = 0 ; eff < effectList.size() ; ++eff)
                list.append(eff);
            break;
        }
        break;
    default:
        break;
    }
    return list;
}

/***
 * !reimp
 */
QHash<QByteArray, QVariant> Backend::objectDescriptionProperties(ObjectDescriptionType type, int index) const
{

    QHash<QByteArray, QVariant> ret;

    if (!isValid())
        return ret;

    switch (type) {
    case Phonon::AudioOutputDeviceType: {
            AudioDevice* ad;
            if ((ad = deviceManager()->audioDevice(index))) {
                ret.insert("name", ad->gstId);
                ret.insert("description", ad->description);
                ret.insert("icon", ad->icon);
            }
        }
        break;

    case Phonon::EffectType: {
            QList<EffectInfo*> effectList = effectManager()->audioEffects();
            if (index >= 0 && index <= effectList.size()) {
                const EffectInfo *effect = effectList[index];
                ret.insert("name", effect->name());
                ret.insert("description", effect->description());
                ret.insert("author", effect->author());
            } else
                Q_ASSERT(1); // Since we use list position as ID, this should not happen
        }
    default:
        break;
    }
    return ret;
}

/***
 * !reimp
 */
bool Backend::startConnectionChange(QSet<QObject *> objects)
{
    for (QObject *object : objects) {
        MediaNode *sourceNode = qobject_cast<MediaNode *>(object);
        MediaObject *media = sourceNode->root();
        if (media) {
            media->saveState();
            return true;
        }
    }
    return true;
}

/***
 * !reimp
 */
bool Backend::connectNodes(QObject *source, QObject *sink)
{
    if (isValid()) {
        MediaNode *sourceNode = qobject_cast<MediaNode *>(source);
        MediaNode *sinkNode = qobject_cast<MediaNode *>(sink);
        if (sourceNode && sinkNode) {
            if (sourceNode->connectNode(sink)) {
                sourceNode->root()->invalidateGraph();

                logMessage(QString("Backend connected %0 to %1")
                     .formatArg(source->metaObject()->className()).formatArg(sink->metaObject()->className()));
                return true;
            }
        }
    }

    logMessage(QString("Linking %0 to %1 failed")
         .formatArg(source->metaObject()->className()).formatArg(sink->metaObject()->className()), Warning);
    return false;
}

/***
 * !reimp
 */
bool Backend::disconnectNodes(QObject *source, QObject *sink)
{
    MediaNode *sourceNode = qobject_cast<MediaNode *>(source);
    MediaNode *sinkNode = qobject_cast<MediaNode *>(sink);

    if (sourceNode && sinkNode)
        return sourceNode->disconnectNode(sink);
    else
        return false;
}

/***
 * !reimp
 */
bool Backend::endConnectionChange(QSet<QObject *> objects)
{
    for (QObject *object : objects) {
        MediaNode *sourceNode = qobject_cast<MediaNode *>(object);
        MediaObject *media = sourceNode->root();
        if (media) {
            media->resumeState();
            return true;
        }
    }
    return true;
}

/***
 * Request bus messages for this mediaobject
 */
void Backend::addBusWatcher(MediaObject* node)
{
    Q_ASSERT(node);
    GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE(node->pipeline()));
    gst_bus_add_watch (bus, busCall, node);
    gst_object_unref(bus);
}

/***
 * Ignore bus messages for this mediaobject
 */
void Backend::removeBusWatcher(MediaObject* node)
{
    Q_ASSERT(node);
    g_source_remove_by_user_data(node);
}

/***
 * Polls each mediaobject's pipeline and delivers
 * pending any pending messages
 */
void Backend::handleBusMessage(Message message)
{
    MediaObject *mediaObject = message.source();
    mediaObject->handleBusMessage(message);
}

DeviceManager* Backend::deviceManager() const
{
    return m_deviceManager;
}

EffectManager* Backend::effectManager() const
{
    return m_effectManager;
}

/**
 * Returns a debuglevel that is determined by the
 * PHONON_GST_DEBUG environment variable.
 *
 *  Warning - important warnings
 *  Info    - general info
 *  Debug   - gives extra info
 */
Backend::DebugLevel Backend::debugLevel() const
{
    return m_debugLevel;
}

/***
 * Prints a conditional debug message based on the current debug level
 * If obj is provided, classname and objectname will be printed as well
 *
 * see debugLevel()
 */
void Backend::logMessage(const QString &message, int priority, QObject *obj) const
{
    if (debugLevel() > 0) {
        QString output;

        if (obj) {
            // Strip away namespace from className
            QString className(obj->metaObject()->className());

            int nameLength = className.length() - className.lastIndexOf(':') - 1;
            className = className.right(nameLength);

            output = QString("%1 %2 %3").formatArgs(message, obj->objectName(), className);
        }

        else {
            output = message;
        }

        if (priority <= (int)debugLevel()) {
            qDebug() << QString("PGST(%1): %2").formatArg(priority).formatArg(output);
        }
    }
}

}
}

QT_END_NAMESPACE

