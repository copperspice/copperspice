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

#ifndef CAMERABINAUDIOENCODE_H
#define CAMERABINAUDIOENCODE_H

#include <qaudioencodersettingscontrol.h>

#include <qstringlist.h>
#include <qmap.h>
#include <qset.h>

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

#ifdef HAVE_GST_ENCODING_PROFILES
#include <gst/pbutils/encoding-profile.h>
#include <private/qgstcodecsinfo_p.h>
#endif

#include <qaudioformat.h>

class CameraBinSession;

class CameraBinAudioEncoder : public QAudioEncoderSettingsControl
{
   CS_OBJECT(CameraBinAudioEncoder)

 public:
   CameraBinAudioEncoder(QObject *parent);
   virtual ~CameraBinAudioEncoder();

   QStringList supportedAudioCodecs() const override;
   QString codecDescription(const QString &codecName) const override;

   QStringList supportedEncodingOptions(const QString &codec) const;
   QVariant encodingOption(const QString &codec, const QString &name) const;
   void setEncodingOption(const QString &codec, const QString &name, const QVariant &value);

   QList<int> supportedSampleRates(const QAudioEncoderSettings &settings = QAudioEncoderSettings(),
                                   bool *isContinuous = nullptr) const override;

   QList<int> supportedChannelCounts(const QAudioEncoderSettings &settings = QAudioEncoderSettings()) const;
   QList<int> supportedSampleSizes(const QAudioEncoderSettings &settings = QAudioEncoderSettings()) const;

   QAudioEncoderSettings audioSettings() const override;
   void setAudioSettings(const QAudioEncoderSettings &) override;

   QAudioEncoderSettings actualAudioSettings() const;
   void setActualAudioSettings(const QAudioEncoderSettings &);
   void resetActualSettings();

#ifdef HAVE_GST_ENCODING_PROFILES
   GstEncodingProfile *createProfile();
#endif

   void applySettings(GstElement *element);

   CS_SIGNAL_1(Public, void settingsChanged())
   CS_SIGNAL_2(settingsChanged)

 private:

#ifdef HAVE_GST_ENCODING_PROFILES
   QGstCodecsInfo m_codecs;
#endif

   QAudioEncoderSettings m_actualAudioSettings;
   QAudioEncoderSettings m_audioSettings;
};

#endif
