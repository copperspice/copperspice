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

#include <camera_videoencoder.h>
#include <camera_session.h>
#include <camera_container.h>
#include <qdebug.h>

#include <qgstutils_p.h>

CameraBinVideoEncoder::CameraBinVideoEncoder(CameraBinSession *session)
   : QVideoEncoderSettingsControl(session), m_session(session)
#ifdef HAVE_GST_ENCODING_PROFILES
   , m_codecs(QGstCodecsInfo::VideoEncoder)
#endif
{
}

CameraBinVideoEncoder::~CameraBinVideoEncoder()
{
}

QList<QSize> CameraBinVideoEncoder::supportedResolutions(const QVideoEncoderSettings &settings, bool *continuous) const
{
   if (continuous) {
      *continuous = false;
   }

   QPair<int, int> rate = rateAsRational(settings.frameRate());

   //select the closest supported rational rate to settings.frameRate()

   return m_session->supportedResolutions(rate, continuous, QCamera::CaptureVideo);
}

QList< qreal > CameraBinVideoEncoder::supportedFrameRates(const QVideoEncoderSettings &settings, bool *continuous) const
{
   if (continuous) {
      *continuous = false;
   }

   QList< qreal > retval;

   for (auto rate : m_session->supportedFrameRates(settings.resolution(), continuous)) {
      if (rate.second > 0) {
         retval << qreal(rate.first) / rate.second;
      }
   }

   return retval;
}

QStringList CameraBinVideoEncoder::supportedVideoCodecs() const
{
#ifdef HAVE_GST_ENCODING_PROFILES
   return m_codecs.supportedCodecs();
#else
   return QStringList();
#endif
}

QString CameraBinVideoEncoder::videoCodecDescription(const QString &codecName) const
{
#ifdef HAVE_GST_ENCODING_PROFILES
   return m_codecs.codecDescription(codecName);
#else
   (void) codecName;
   return QString();
#endif
}

QVideoEncoderSettings CameraBinVideoEncoder::videoSettings() const
{
   return m_videoSettings;
}

void CameraBinVideoEncoder::setVideoSettings(const QVideoEncoderSettings &settings)
{
   if (m_videoSettings != settings) {
      m_actualVideoSettings = settings;
      m_videoSettings = settings;
      emit settingsChanged();
   }
}

QVideoEncoderSettings CameraBinVideoEncoder::actualVideoSettings() const
{
   return m_actualVideoSettings;
}

void CameraBinVideoEncoder::setActualVideoSettings(const QVideoEncoderSettings &settings)
{
   m_actualVideoSettings = settings;
}

void CameraBinVideoEncoder::resetActualSettings()
{
   m_actualVideoSettings = m_videoSettings;
}


QPair<int, int> CameraBinVideoEncoder::rateAsRational(qreal frameRate) const
{
   if (frameRate > 0.001) {
      //convert to rational number
      QList<int> denumCandidates;
      denumCandidates << 1 << 2 << 3 << 5 << 10 << 25 << 30 << 50 << 100 << 1001 << 1000;

      qreal error = 1.0;
      int num = 1;
      int denum = 1;

      for (int curDenum : denumCandidates) {
         int curNum = qRound(frameRate * curDenum);
         qreal curError = qAbs(qreal(curNum) / curDenum - frameRate);

         if (curError < error) {
            error = curError;
            num = curNum;
            denum = curDenum;
         }

         if (curError < 1e-8) {
            break;
         }
      }

      return QPair<int, int>(num, denum);
   }

   return QPair<int, int>();
}

#ifdef HAVE_GST_ENCODING_PROFILES

GstEncodingProfile *CameraBinVideoEncoder::createProfile()
{
   QString codec = m_actualVideoSettings.codec();
   QString preset = m_actualVideoSettings.encodingOption("preset").toString();

   GstCaps *caps;

   if (codec.isEmpty()) {
      caps = 0;
   } else {
      caps = gst_caps_from_string(codec.toLatin1());
   }

   GstEncodingVideoProfile *profile = gst_encoding_video_profile_new(
                                         caps,
                                         !preset.isEmpty() ? preset.toLatin1().constData() : NULL, //preset
                                         NULL, //restriction
                                         1); //presence

   gst_caps_unref(caps);

   gst_encoding_video_profile_set_pass(profile, 0);
   gst_encoding_video_profile_set_variableframerate(profile, TRUE);

   return (GstEncodingProfile *)profile;
}

#endif

void CameraBinVideoEncoder::applySettings(GstElement *encoder)
{
   GObjectClass *const objectClass = G_OBJECT_GET_CLASS(encoder);
   const char *const name = qt_gst_element_get_factory_name(encoder);

   const int bitRate = m_actualVideoSettings.bitRate();
   if (bitRate == -1) {
      // Bit rate is invalid, don't evaluate the remaining conditions.
   } else if (g_object_class_find_property(objectClass, "bitrate")) {
      g_object_set(G_OBJECT(encoder), "bitrate", bitRate, NULL);
   } else if (g_object_class_find_property(objectClass, "target-bitrate")) {
      g_object_set(G_OBJECT(encoder), "target-bitrate", bitRate, NULL);
   }

   if (qstrcmp(name, "theoraenc") == 0) {
      static const int qualities[] = { 8, 16, 32, 45, 60 };
      g_object_set(G_OBJECT(encoder), "quality", qualities[m_actualVideoSettings.quality()], NULL);
   } else if (qstrncmp(name, "avenc_", 6) == 0) {
      if (g_object_class_find_property(objectClass, "pass")) {
         static const int modes[] = { 0, 2, 512, 1024 };
         g_object_set(G_OBJECT(encoder), "pass", modes[m_actualVideoSettings.encodingMode()], NULL);
      }
      if (g_object_class_find_property(objectClass, "quantizer")) {
         static const double qualities[] = { 20, 8.0, 3.0, 2.5, 2.0 };
         g_object_set(G_OBJECT(encoder), "quantizer", qualities[m_actualVideoSettings.quality()], NULL);
      }
   } else if (qstrncmp(name, "omx", 3) == 0) {
      if (!g_object_class_find_property(objectClass, "control-rate")) {
      } else switch (m_actualVideoSettings.encodingMode()) {
            case QMultimedia::ConstantBitRateEncoding:
               g_object_set(G_OBJECT(encoder), "control-rate", 2, NULL);
               break;
            case QMultimedia::AverageBitRateEncoding:
               g_object_set(G_OBJECT(encoder), "control-rate", 1, NULL);
               break;
            default:
               g_object_set(G_OBJECT(encoder), "control-rate", 0, NULL);
         }
   }
}

