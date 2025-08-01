/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef CAMERABINCAPTURESERVICE_H
#define CAMERABINCAPTURESERVICE_H

#include <qmediaservice.h>

#include <gst/gst.h>

class CameraBinControl;
class CameraBinImageCapture;
class CameraBinMetaData;
class CameraBinMetaData;
class CameraBinSession;
class CameraBinViewfinderSettings2;
class CameraBinViewfinderSettings;
class QAudioInputSelectorControl;
class QGstreamerBusHelper;
class QGstreamerElementFactory;
class QGstreamerMessage;
class QGstreamerVideoRenderer;
class QGstreamerVideoWidgetControl;
class QGstreamerVideoWindow;
class QVideoDeviceSelectorControl;

class CameraBinService : public QMediaService
{
   CS_OBJECT(CameraBinService)

 public:
   CameraBinService(GstElementFactory *sourceFactory, QObject *parent = nullptr);
   virtual ~CameraBinService();

   QMediaControl *requestControl(const QString &name) override;
   void releaseControl(QMediaControl *) override;

   static bool isCameraBinAvailable();

 private:
   void setAudioPreview(GstElement *);

   CameraBinSession *m_captureSession;
   CameraBinMetaData *m_metaDataControl;

   QAudioInputSelectorControl *m_audioInputSelector;
   QVideoDeviceSelectorControl *m_videoInputDevice;

   QMediaControl *m_videoOutput;

   QMediaControl *m_videoRenderer;
   QGstreamerVideoWindow *m_videoWindow;
   QGstreamerVideoWidgetControl *m_videoWidgetControl;

   CameraBinImageCapture *m_imageCaptureControl;
   QMediaControl *m_cameraInfoControl;

   CameraBinViewfinderSettings *m_viewfinderSettingsControl;
   CameraBinViewfinderSettings2 *m_viewfinderSettingsControl2;
};

#endif
