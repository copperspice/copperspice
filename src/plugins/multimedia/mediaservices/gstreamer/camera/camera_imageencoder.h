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

#ifndef CAMERABINIMAGEENCODE_H
#define CAMERABINIMAGEENCODE_H

#include <qimageencodercontrol.h>
#include <qstringlist.h>
#include <qmap.h>

#include <gst/gst.h>

class CameraBinSession;

class CameraBinImageEncoder : public QImageEncoderControl
{
   CS_OBJECT(CameraBinImageEncoder)

 public:
   CameraBinImageEncoder(CameraBinSession *session);
   virtual ~CameraBinImageEncoder();

   QList<QSize> supportedResolutions(const QImageEncoderSettings &settings = QImageEncoderSettings(),
         bool *continuous = nullptr) const override;

   QStringList supportedImageCodecs() const override;
   QString imageCodecDescription(const QString &formatName) const override;

   QImageEncoderSettings imageSettings() const override;
   void setImageSettings(const QImageEncoderSettings &settings) override;

   CS_SIGNAL_1(Public, void settingsChanged())
   CS_SIGNAL_2(settingsChanged)

 private:
   QImageEncoderSettings m_settings;

   CameraBinSession *m_session;

   QStringList m_codecs;
   QMap<QString, QByteArray> m_elementNames;
   QMap<QString, QString> m_codecDescriptions;
   QMap<QString, QStringList> m_codecOptions;
};

#endif
