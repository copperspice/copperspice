/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//! [snippet]
QObject *Backend::createObject(BackendInterface::Class c, QObject *parent, const QList<QVariant> &args)
{
    switch (c) {
    case MediaObjectClass:
        return new MediaObject(parent);
    case VolumeFaderEffectClass:
        return new VolumeFaderEffect(parent);
    case AudioOutputClass:
        return new AudioOutput(parent);
    case AudioDataOutputClass:
        return new AudioDataOutput(parent);
    case VisualizationClass:
        return new Visualization(parent);
    case VideoDataOutputClass:
        return new VideoDataOutput(parent);
    case EffectClass:
        return new Effect(args[0].toInt(), parent);
    case VideoWidgetClass:
        return new VideoWidget(qobject_cast<QWidget *>(parent));
    }
    return 0;
}

QSet<int> Backend::objectDescriptionIndexes(ObjectDescriptionType type) const
{
    QSet<int> set;
    switch(type)
    {
    case Phonon::AudioOutputDeviceType:
        // use AudioDeviceEnumerator to list ALSA and OSS devices
        set << 10000 << 10001;
        break;
    case Phonon::AudioCaptureDeviceType:
        set << 20000 << 20001;
        break;
    case Phonon::VideoOutputDeviceType:
        break;
    case Phonon::VideoCaptureDeviceType:
        set << 30000 << 30001;
        break;
    case Phonon::VisualizationType:
    case Phonon::AudioCodecType:
    case Phonon::VideoCodecType:
    case Phonon::ContainerFormatType:
        break;
    case Phonon::EffectType:
        set << 0x7F000001;
        break;
    }
    return set;
}

QHash<QByteArray, QVariant> Backend::objectDescriptionProperties(ObjectDescriptionType type, int index) const
{
    QHash<QByteArray, QVariant> ret;
    switch (type) {
    case Phonon::AudioOutputDeviceType:
        switch (index) {
        case 10000:
            ret.insert("name", QLatin1String("internal Soundcard"));
            break;
        case 10001:
            ret.insert("name", QLatin1String("USB Headset"));
            ret.insert("icon", KIcon("usb-headset"));
            ret.insert("available", false);
            break;
        }
        break;
    case Phonon::AudioCaptureDeviceType:
        switch (index) {
        case 20000:
            ret.insert("name", QLatin1String("Soundcard"));
            ret.insert("description", QLatin1String("first description"));
            break;
        case 20001:
            ret.insert("name", QLatin1String("DV"));
            ret.insert("description", QLatin1String("second description"));
            break;
        }
        break;
    case Phonon::VideoOutputDeviceType:
        break;
    case Phonon::VideoCaptureDeviceType:
        switch (index) {
        case 30000:
            ret.insert("name", QLatin1String("USB Webcam"));
            ret.insert("description", QLatin1String("first description"));
            break;
        case 30001:
            ret.insert("name", QLatin1String("DV"));
            ret.insert("description", QLatin1String("second description"));
            break;
        }
        break;
    case Phonon::VisualizationType:
        break;
    case Phonon::AudioCodecType:
        break;
    case Phonon::VideoCodecType:
        break;
    case Phonon::ContainerFormatType:
        break;
    case Phonon::EffectType:
        switch (index) {
        case 0x7F000001:
            ret.insert("name", QLatin1String("Delay"));
            ret.insert("description", QLatin1String("Simple delay effect with time, feedback and level controls."));
            break;
        }
        break;
    }
    return ret;
}
//! [snippet]
