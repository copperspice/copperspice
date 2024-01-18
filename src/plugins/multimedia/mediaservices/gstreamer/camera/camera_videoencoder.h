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

#ifndef CAMERA_VIDEOENCODER_H
#define CAMERA_VIDEOENCODER_H

#include <qmap.h>
#include <qset.h>
#include <qstringlist.h>
#include <qvideoencodersettingscontrol.h>

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

#ifdef HAVE_GST_ENCODING_PROFILES
#include <gst/pbutils/encoding-profile.h>
#include <qgstcodecsinfo_p.h>
#endif

class CameraBinSession;

class CameraBinVideoEncoder : public QVideoEncoderSettingsControl
{
   CS_OBJECT(CameraBinVideoEncoder)

 public:
   CameraBinVideoEncoder(CameraBinSession *session);
   virtual ~CameraBinVideoEncoder();

   QList<QSize> supportedResolutions(const QVideoEncoderSettings &settings = QVideoEncoderSettings(),
         bool *continuous = nullptr) const override;

   QList< qreal > supportedFrameRates(const QVideoEncoderSettings &settings = QVideoEncoderSettings(),
         bool *continuous = nullptr) const override;

   QPair<int, int> rateAsRational(qreal) const;

   QStringList supportedVideoCodecs() const override;
   QString videoCodecDescription(const QString &codecName) const override;

   QVideoEncoderSettings videoSettings() const override;
   void setVideoSettings(const QVideoEncoderSettings &settings) override;

   QVideoEncoderSettings actualVideoSettings() const;
   void setActualVideoSettings(const QVideoEncoderSettings &);
   void resetActualSettings();

#ifdef HAVE_GST_ENCODING_PROFILES
   GstEncodingProfile *createProfile();
#endif

   void applySettings(GstElement *encoder);

 public:
   CS_SIGNAL_1(Public, void settingsChanged())
   CS_SIGNAL_2(settingsChanged)

 private:
   CameraBinSession *m_session;

#ifdef HAVE_GST_ENCODING_PROFILES
   QGstCodecsInfo m_codecs;
#endif

   QVideoEncoderSettings m_actualVideoSettings;
   QVideoEncoderSettings m_videoSettings;
};

#endif
