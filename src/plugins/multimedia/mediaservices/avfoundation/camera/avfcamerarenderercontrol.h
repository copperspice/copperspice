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

#ifndef AVFCAMERARENDERERCONTROL_H
#define AVFCAMERARENDERERCONTROL_H

#include <qvideorenderercontrol.h>
#include <qvideoframe.h>
#include <qmutex.h>

#import <AVFoundation/AVFoundation.h>

@class AVFCaptureFramesDelegate;

class AVFCameraSession;
class AVFCameraService;
class AVFCameraRendererControl;

class AVFCameraRendererControl : public QVideoRendererControl
{
   CS_OBJECT(AVFCameraRendererControl)

 public:
   AVFCameraRendererControl(QObject *parent = nullptr);
   ~AVFCameraRendererControl();

   QAbstractVideoSurface *surface() const override;
   void setSurface(QAbstractVideoSurface *surface) override;

   void configureAVCaptureSession(AVFCameraSession *cameraSession);
   void syncHandleViewfinderFrame(const QVideoFrame &frame);

   AVCaptureVideoDataOutput *videoDataOutput() const;

   bool supportsTextures() const {
      return m_supportsTextures;
   }

#ifdef Q_OS_IOS
   AVFCaptureFramesDelegate *captureDelegate() const;
   void resetCaptureDelegate() const;
#endif

   CS_SIGNAL_1(Public, void surfaceChanged(QAbstractVideoSurface *surface))
   CS_SIGNAL_2(surfaceChanged, surface)

 private:
   CS_SLOT_1(Private, void handleViewfinderFrame())
   CS_SLOT_2(handleViewfinderFrame)

   CS_SLOT_1(Private, void updateCaptureConnection())
   CS_SLOT_2(updateCaptureConnection)

   QAbstractVideoSurface *m_surface;
   AVFCaptureFramesDelegate *m_viewfinderFramesDelegate;
   AVFCameraSession *m_cameraSession;
   AVCaptureVideoDataOutput *m_videoDataOutput;

   bool m_supportsTextures;
   bool m_needsHorizontalMirroring;

#ifdef Q_OS_IOS
   CVOpenGLESTextureCacheRef m_textureCache;
#endif

   QVideoFrame m_lastViewfinderFrame;
   QMutex m_vfMutex;
   dispatch_queue_t m_delegateQueue;

   friend class CVImageVideoBuffer;
};

#endif
