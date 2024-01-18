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

#include <camera_audioencoder.h>
#include <camera_container.h>
#include <qdebug.h>

#include <qgstutils_p.h>

CameraBinAudioEncoder::CameraBinAudioEncoder(QObject *parent)
   : QAudioEncoderSettingsControl(parent)

#ifdef HAVE_GST_ENCODING_PROFILES
   , m_codecs(QGstCodecsInfo::AudioEncoder)
#endif
{
}

CameraBinAudioEncoder::~CameraBinAudioEncoder()
{
}

QStringList CameraBinAudioEncoder::supportedAudioCodecs() const
{
#ifdef HAVE_GST_ENCODING_PROFILES
   return m_codecs.supportedCodecs();
#else
   return QStringList();
#endif
}

QString CameraBinAudioEncoder::codecDescription(const QString &codecName) const
{
#ifdef HAVE_GST_ENCODING_PROFILES
   return m_codecs.codecDescription(codecName);
#else
   (void) codecName;

   return QString();
#endif
}

QList<int> CameraBinAudioEncoder::supportedSampleRates(const QAudioEncoderSettings &, bool *) const
{
   //TODO check element caps to find actual values

   return QList<int>();
}

QAudioEncoderSettings CameraBinAudioEncoder::audioSettings() const
{
   return m_audioSettings;
}

void CameraBinAudioEncoder::setAudioSettings(const QAudioEncoderSettings &settings)
{
   if (m_audioSettings != settings) {
      m_audioSettings = settings;
      m_actualAudioSettings = settings;
      emit settingsChanged();
   }
}

QAudioEncoderSettings CameraBinAudioEncoder::actualAudioSettings() const
{
   return m_actualAudioSettings;
}

void CameraBinAudioEncoder::setActualAudioSettings(const QAudioEncoderSettings &settings)
{
   m_actualAudioSettings = settings;
}

void CameraBinAudioEncoder::resetActualSettings()
{
   m_actualAudioSettings = m_audioSettings;
}

#ifdef HAVE_GST_ENCODING_PROFILES

GstEncodingProfile *CameraBinAudioEncoder::createProfile()
{
   QString codec = m_actualAudioSettings.codec();
   QString preset = m_actualAudioSettings.encodingOption("preset").toString();
   GstCaps *caps;

   if (codec.isEmpty()) {
      return 0;
   } else {
      caps = gst_caps_from_string(codec.toLatin1());
   }

   GstEncodingProfile *profile = (GstEncodingProfile *)gst_encoding_audio_profile_new(
                                    caps,
                                    !preset.isEmpty() ? preset.toLatin1().constData() : NULL, //preset
                                    NULL,   //restriction
                                    0);     //presence

   gst_caps_unref(caps);

   return profile;
}

#endif

void CameraBinAudioEncoder::applySettings(GstElement *encoder)
{
   GObjectClass *const objectClass = G_OBJECT_GET_CLASS(encoder);
   const char *const name = qt_gst_element_get_factory_name(encoder);

   const bool isVorbis = qstrcmp(name, "vorbisenc") == 0;

   const int bitRate = m_actualAudioSettings.bitRate();
   if (!isVorbis && bitRate == -1) {
      // Bit rate is invalid, don't evaluate the remaining conditions unless the encoder is
      // vorbisenc which is known to accept -1 as an unspecified bitrate.
   } else if (g_object_class_find_property(objectClass, "bitrate")) {
      g_object_set(G_OBJECT(encoder), "bitrate", bitRate, NULL);
   } else if (g_object_class_find_property(objectClass, "target-bitrate")) {
      g_object_set(G_OBJECT(encoder), "target-bitrate", bitRate, NULL);
   }

   if (isVorbis) {
      static const double qualities[] = { 0.1, 0.3, 0.5, 0.7, 1.0 };
      g_object_set(G_OBJECT(encoder), "quality", qualities[m_actualAudioSettings.quality()], NULL);
   }
}

