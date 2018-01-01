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
**  This file is part of the KDE project.
********************************************************/

#include "backend.h"
#include <QtCore/QDebug>
#include <QtCore/QSet>
#include <QtCore/QVariant>
#include <QtCore/QtPlugin>

#include "backendheader.h"

#include "videowidget.h"
#include "audiooutput.h"
#include "mediaobject.h"
#include "videoeffect.h"
#include "medianode.h"
#include "audiodevice.h"
#include "audiomixer.h"
#include "backendinfo.h"
#include "quicktimeaudioplayer.h"

#include "audiograph.h"
#include "audiomixer.h"
#include "audiooutput.h"
#include "audiosplitter.h"
#include "audioeffects.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{

Backend::Backend()
{
    IMPLEMENTED << "Creating backend QT7";
}

Backend::Backend(QObject *parent, const QStringList &) : QObject(parent)
{
    IMPLEMENTED << "Creating backend QT7";
    setProperty("identifier",     QLatin1String("Mac OS X/QuickTime7"));
    setProperty("backendName",    QLatin1String("Mac OS X/QuickTime7"));
    setProperty("backendVersion", QLatin1String("0.1"));
    setProperty("backendIcon",    QLatin1String(""));
    setProperty("backendWebsite", QLatin1String("http://copperspice.com/"));
}

Backend::~Backend()
{
}

bool Backend::quickTime7Available()
{
    static bool ok = BackendInfo::isQuickTimeVersionAvailable(0x0700);
    if (!ok) {
        static bool messageWritten = false;

        if (!messageWritten && qgetenv("PHONON_DEBUG") == "1"){
            messageWritten = true;
            qWarning("WARNING: Phonon backend plugin need QuickTime 7 or newer to work."
                     " This computer has version %s installed.",
                     BackendInfo::quickTimeVersionString().toLatin1().data());
        }
        return false;
    }
    return true;
}

QObject *Backend::createObject(BackendInterface::Class c, QObject *parent, const QList<QVariant> &args)
{
    if (!quickTime7Available())
        return 0;

    switch (c) {
    case MediaObjectClass:
        IMPLEMENTED << "Creating new MediaObjectClass.";
        return new MediaObject(parent);
        break;
    case VolumeFaderEffectClass:
        IMPLEMENTED << "Creating new VolumeFaderEffectClass.";
        return new AudioMixer(parent);
        break;
    case AudioOutputClass:
        IMPLEMENTED << "Creating new AudioOutputClass.";
        return new AudioOutput(parent);
        break;
    case AudioDataOutputClass:
        NOT_IMPLEMENTED << "Creating new AudioDataOutputClass.";
        break;
    case VisualizationClass:
        NOT_IMPLEMENTED << "Creating new VisualizationClass.";
        break;
    case VideoDataOutputClass:
        NOT_IMPLEMENTED << "Creating new VideoDataOutputClass.";
        break;
    case EffectClass:
        IMPLEMENTED << "Creating new EffectClass.";
        return new AudioEffect(args[0].toInt());
        break;
    case VideoWidgetClass:
        IMPLEMENTED << "Creating new VideoWidget.";
        return new VideoWidget(parent);
        break;
    default:
        return 0;
    }
    return 0;
}

bool Backend::startConnectionChange(QSet<QObject *> objects)
{
    IMPLEMENTED;
    QList<AudioGraph *> notifiedGraphs;
    for (int i=0; i<objects.size(); i++){
        MediaNode *node = qobject_cast<MediaNode*>(objects.values()[i]);
        if (node && node->m_audioGraph && !notifiedGraphs.contains(node->m_audioGraph)){
            MediaNodeEvent event(MediaNodeEvent::StartConnectionChange);
            node->m_audioGraph->notify(&event);
            notifiedGraphs << node->m_audioGraph;
        }
    }
    return true;
}

bool Backend::endConnectionChange(QSet<QObject *> objects)
{
    IMPLEMENTED;
    QList<AudioGraph *> notifiedGraphs;
    for (int i=0; i<objects.size(); i++){
        MediaNode *node = qobject_cast<MediaNode*>(objects.values()[i]);
        if (node && node->m_audioGraph && !notifiedGraphs.contains(node->m_audioGraph)){
            MediaNodeEvent event(MediaNodeEvent::EndConnectionChange);
            node->m_audioGraph->notify(&event);
            notifiedGraphs << node->m_audioGraph;
        }
    }
    return true;
}

bool Backend::connectNodes(QObject *aSource, QObject *aSink)
{
    IMPLEMENTED;
    MediaNode *source = qobject_cast<MediaNode*>(aSource);
    if (!source) return false;
    MediaNode *sink = qobject_cast<MediaNode*>(aSink);
    if (!sink) return false;

    return source->connectToSink(sink);
}


bool Backend::disconnectNodes(QObject *aSource, QObject *aSink)
{
    IMPLEMENTED;
    MediaNode *source = qobject_cast<MediaNode*>(aSource);
    if (!source) return false;
    MediaNode *sink = qobject_cast<MediaNode*>(aSink);
    if (!sink) return false;

    return source->disconnectToSink(sink);
}


QStringList Backend::availableMimeTypes() const
{
    IMPLEMENTED;
    return BackendInfo::quickTimeMimeTypes(BackendInfo::In);
}

/**
* Returns a set of indexes that acts as identifiers for the various properties
* this backend supports for the given ObjectDescriptionType.
* More information for a given property/index can be
* looked up in Backend::objectDescriptionProperties(...).
*/
QList<int> Backend::objectDescriptionIndexes(ObjectDescriptionType type) const
{
    QList<int> ret;

    switch (type){
    case AudioOutputDeviceType:{
        IMPLEMENTED_SILENT << "Creating index set for type: AudioOutputDeviceType";
        QList<AudioDeviceID> devices = AudioDevice::devices(AudioDevice::Out);
        for (int i=0; i<devices.size(); i++)
            ret << int(devices[i]);
        break; }
    case EffectType:{
        IMPLEMENTED_SILENT << "Creating index set for type: EffectType";
        if (QuickTimeAudioPlayer::soundPlayerIsAwailable())
            ret = AudioEffect::effectList();
        break; }

#if 0 // will be awailable in a later version of phonon.
    case AudioCaptureDeviceType:{
        IMPLEMENTED_SILENT << "Creating index set for type: AudioCaptureDeviceType";
        QList<AudioDeviceID> devices = AudioDevice::devices(AudioDevice::In).keys();
        for (int i=0; i<devices.size(); i++)
            ret <<int(devices[i]);
        break; }
    case VideoEffectType:{
        // Just count the number of filters awailable (c), and
        // add return a set with the numbers 1..c inserted:
        IMPLEMENTED_SILENT << "Creating index set for type: VideoEffectType";
        QList<QtCore/QString> filters = objc_getCiFilterInfo()->filterDisplayNames;
        for (int i=0; i<filters.size(); i++)
            ret << insert(i);
        break; }
#endif
    default:
        NOT_IMPLEMENTED;
        break;
    }
    return ret;
}

QHash<QByteArray, QVariant> Backend::objectDescriptionProperties(ObjectDescriptionType type, int index) const
{
    QHash<QByteArray, QVariant> ret;

    switch (type){
    case AudioOutputDeviceType:{
        IMPLEMENTED_SILENT << "Creating description hash for type: AudioOutputDeviceType";
        ret.insert("name", AudioDevice::deviceSourceNameElseDeviceName(index));
        ret.insert("description", AudioDevice::deviceNameElseDeviceSourceName(index));
        break; }
    case EffectType:{
        AudioEffect e(index);
        ret.insert("name", e.name());
        ret.insert("description", e.description());
        break; }

#if 0 // will be awailable in a later version of phonon.
    case VideoEffectType:{
        // Get list of effects, pick out filter at index, and return its name:
        IMPLEMENTED_SILENT << "Creating description hash for type: VideoEffectType";
        QList<QtCore/QString> filters = objc_getCiFilterInfo()->filterDisplayNames;
        ret.insert("name", filters[index]);
    case AudioCaptureDeviceType:{
        IMPLEMENTED_SILENT << "Creating description hash for type: AudioCaptureDeviceType";
        QMap<AudioDeviceID, QString> devices = AudioDevice::devices(AudioDevice::In);
        ret.insert("name", devices.value(index));
        break; }
#endif
    default:
        NOT_IMPLEMENTED;
        break;
    }

    return ret;
}

Q_EXPORT_PLUGIN2(phonon_qt7, Backend)
}}

QT_END_NAMESPACE



