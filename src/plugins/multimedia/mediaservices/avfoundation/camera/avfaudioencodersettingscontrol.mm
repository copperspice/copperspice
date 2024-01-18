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

#include "avfaudioencodersettingscontrol.h"

#include "avfcameraservice.h"
#include "avfcamerasession.h"

#include <AVFoundation/AVFoundation.h>
#include <CoreAudio/CoreAudioTypes.h>

struct AudioCodecInfo
{
    QString description;
    int id;

    AudioCodecInfo()
      : id(0)
    {
    }

    AudioCodecInfo(const QString &desc, int i)
        : description(desc), id(i)
    {
    }
};

using SupportedAudioCodecs = QMap<QString, AudioCodecInfo>;

static QString *defaultCodec() {
   static QString retval = "aac";
   return &retval;
}

static SupportedAudioCodecs *supportedCodecs() {
   static SupportedAudioCodecs retval;
   return &retval;
}

AVFAudioEncoderSettingsControl::AVFAudioEncoderSettingsControl(AVFCameraService *service)
    : QAudioEncoderSettingsControl(), m_service(service)
{
    if (supportedCodecs()->isEmpty()) {
        supportedCodecs()->insert("lpcm", AudioCodecInfo("Linear PCM", kAudioFormatLinearPCM));
        supportedCodecs()->insert("ulaw", AudioCodecInfo("PCM Mu-Law 2:1", kAudioFormatULaw));
        supportedCodecs()->insert("alaw", AudioCodecInfo("PCM A-Law 2:1", kAudioFormatALaw));
        supportedCodecs()->insert("ima4", AudioCodecInfo("IMA 4:1 ADPCM", kAudioFormatAppleIMA4));
        supportedCodecs()->insert("alac", AudioCodecInfo("Apple Lossless Audio Codec", kAudioFormatAppleLossless));
        supportedCodecs()->insert("aac",  AudioCodecInfo("MPEG-4 Low Complexity AAC", kAudioFormatMPEG4AAC));
        supportedCodecs()->insert("aach", AudioCodecInfo("MPEG-4 High Efficiency AAC", kAudioFormatMPEG4AAC_HE));
        supportedCodecs()->insert("aacl", AudioCodecInfo("MPEG-4 AAC Low Delay", kAudioFormatMPEG4AAC_LD));
        supportedCodecs()->insert("aace", AudioCodecInfo("MPEG-4 AAC Enhanced Low Delay", kAudioFormatMPEG4AAC_ELD));
        supportedCodecs()->insert("aacf", AudioCodecInfo("MPEG-4 AAC Enhanced Low Delay with SBR", kAudioFormatMPEG4AAC_ELD_SBR));
        supportedCodecs()->insert("aacp", AudioCodecInfo("MPEG-4 HE AAC V2", kAudioFormatMPEG4AAC_HE_V2));
        supportedCodecs()->insert("ilbc", AudioCodecInfo("iLBC", kAudioFormatiLBC));
    }
}

QStringList AVFAudioEncoderSettingsControl::supportedAudioCodecs() const
{
    return supportedCodecs()->keys();
}

QString AVFAudioEncoderSettingsControl::codecDescription(const QString &codecName) const
{
    return supportedCodecs()->value(codecName).description;
}

QList<int> AVFAudioEncoderSettingsControl::supportedSampleRates(const QAudioEncoderSettings &settings, bool *continuous) const
{
    (void) settings;

    if (continuous)
        *continuous = true;

    return QList<int>() << 8000 << 96000;
}

QAudioEncoderSettings AVFAudioEncoderSettingsControl::audioSettings() const
{
    return m_actualSettings;
}

void AVFAudioEncoderSettingsControl::setAudioSettings(const QAudioEncoderSettings &settings)
{
    if (m_requestedSettings == settings)
        return;

    m_requestedSettings = m_actualSettings = settings;
}

NSDictionary *AVFAudioEncoderSettingsControl::applySettings()
{
    if (m_service->session()->state() != QCamera::LoadedState &&
        m_service->session()->state() != QCamera::ActiveState) {
        return nil;
    }

    NSMutableDictionary *settings = [NSMutableDictionary dictionary];

    QString codec = m_requestedSettings.codec().isEmpty() ? *defaultCodec() : m_requestedSettings.codec();

    if (! supportedCodecs()->contains(codec)) {
        qWarning("Unsupported codec: '%s'", csPrintable(codec));
        codec = *defaultCodec();
    }

    [settings setObject:[NSNumber numberWithInt:supportedCodecs()->value(codec).id] forKey:AVFormatIDKey];
    m_actualSettings.setCodec(codec);

#ifdef Q_OS_DARWIN
    if (m_requestedSettings.encodingMode() == QMultimedia::ConstantQualityEncoding) {
        int quality;

        switch (m_requestedSettings.quality()) {
        case QMultimedia::VeryLowQuality:
            quality = AVAudioQualityMin;
            break;

        case QMultimedia::LowQuality:
            quality = AVAudioQualityLow;
            break;

        case QMultimedia::HighQuality:
            quality = AVAudioQualityHigh;
            break;

        case QMultimedia::VeryHighQuality:
            quality = AVAudioQualityMax;
            break;

        case QMultimedia::NormalQuality:
        default:
            quality = AVAudioQualityMedium;
            break;
        }
        [settings setObject:[NSNumber numberWithInt:quality] forKey:AVEncoderAudioQualityKey];

    } else
#endif

    if (m_requestedSettings.bitRate() > 0){
        [settings setObject:[NSNumber numberWithInt:m_requestedSettings.bitRate()] forKey:AVEncoderBitRateKey];
    }

    int sampleRate = m_requestedSettings.sampleRate();
    int channelCount = m_requestedSettings.channelCount();

    if (sampleRate > 0) {
        [settings setObject:[NSNumber numberWithInt:sampleRate] forKey:AVSampleRateKey];
        m_actualSettings.setSampleRate(sampleRate);
    }

    if (channelCount > 0) {
        [settings setObject:[NSNumber numberWithInt:channelCount] forKey:AVNumberOfChannelsKey];
        m_actualSettings.setChannelCount(channelCount);
    }

    return settings;
}

void AVFAudioEncoderSettingsControl::unapplySettings()
{
    m_actualSettings = m_requestedSettings;
}

