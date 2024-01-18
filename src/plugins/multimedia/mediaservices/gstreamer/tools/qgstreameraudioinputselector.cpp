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

#include <qgstreameraudioinputselector_p.h>

#include <qdir.h>
#include <qdebug.h>

#include <gst/gst.h>

#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>
#endif

QGstreamerAudioInputSelector::QGstreamerAudioInputSelector(QObject *parent)
    : QAudioInputSelectorControl(parent)
{
    update();
}

QGstreamerAudioInputSelector::~QGstreamerAudioInputSelector()
{
}

QList<QString> QGstreamerAudioInputSelector::availableInputs() const
{
    return m_names;
}

QString QGstreamerAudioInputSelector::inputDescription(const QString& name) const
{
    QString desc;

    for (int i = 0; i < m_names.size(); i++) {
        if (m_names.at(i).compare(name) == 0) {
            desc = m_descriptions.at(i);
            break;
        }
    }

    return desc;
}

QString QGstreamerAudioInputSelector::defaultInput() const
{
    if (m_names.size() > 0)
        return m_names.at(0);

    return QString();
}

QString QGstreamerAudioInputSelector::activeInput() const
{
    return m_audioInput;
}

void QGstreamerAudioInputSelector::setActiveInput(const QString &name)
{
    if (m_audioInput.compare(name) != 0) {
        m_audioInput = name;
        emit activeInputChanged(name);
    }
}

void QGstreamerAudioInputSelector::update()
{
    m_names.clear();
    m_descriptions.clear();

    //use autoaudiosrc as the first default device
    m_names.append("default:");
    m_descriptions.append(tr("System default device"));

    updatePulseDevices();
    updateAlsaDevices();
    updateOssDevices();

    if (m_names.size() > 0)
        m_audioInput = m_names.at(0);
}

void QGstreamerAudioInputSelector::updateAlsaDevices()
{
#ifdef HAVE_ALSA
    void **hints, **n;

    if (snd_device_name_hint(-1, "pcm", &hints) < 0) {
        qWarning()<<"no alsa devices available";
        return;
    }
    n = hints;

    while (*n != NULL) {
        char *name  = snd_device_name_get_hint(*n, "NAME");
        char *descr = snd_device_name_get_hint(*n, "DESC");
        char *io    = snd_device_name_get_hint(*n, "IOID");

        if ((name != NULL) && (descr != NULL)) {
            if (io == NULL || qstrcmp(io, "Input") == 0 ) {
                m_names.append("alsa:" + QString::fromUtf8(name));
                m_descriptions.append(QString::fromUtf8(descr));
            }
        }

        if (name != NULL)
            free(name);

        if (descr != NULL)
            free(descr);

        if (io != NULL)
            free(io);

        n++;
    }
    snd_device_name_free_hint(hints);
#endif
}

void QGstreamerAudioInputSelector::updateOssDevices()
{
    QDir devDir("/dev");
    devDir.setFilter(QDir::System);
    QFileInfoList entries = devDir.entryInfoList(QStringList() << "dsp*");

    for (const QFileInfo &entryInfo : entries) {
        m_names.append("oss:" + entryInfo.filePath());
        m_descriptions.append(QString("OSS device %1").formatArg(entryInfo.fileName()));
    }
}

void QGstreamerAudioInputSelector::updatePulseDevices()
{
    GstElementFactory *factory = gst_element_factory_find("pulsesrc");

    if (factory) {
        m_names.append("pulseaudio:");
        m_descriptions.append("PulseAudio device.");
        gst_object_unref(GST_OBJECT(factory));
    }
}
