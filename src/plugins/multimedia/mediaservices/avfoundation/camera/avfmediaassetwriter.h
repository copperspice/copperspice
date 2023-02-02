/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#ifndef AVFMEDIAASSETWRITER_H
#define AVFMEDIAASSETWRITER_H

#include <avfcamerautility.h>
#include <qglobal.h>
#include <qatomic.h>

#include <AVFoundation/AVFoundation.h>

class AVFMediaRecorderControlIOS;
class AVFCameraService;

using AVFAtomicInt64 = QAtomicInteger<qint64>;

@interface QT_MANGLE_NAMESPACE(AVFMediaAssetWriter) : NSObject<AVCaptureVideoDataOutputSampleBufferDelegate,
      AVCaptureAudioDataOutputSampleBufferDelegate>
{
 @private
   AVFCameraService *m_service;

   QT_PREPEND_NAMESPACE(AVFScopedPointer)<AVAssetWriterInput> m_cameraWriterInput;
   QT_PREPEND_NAMESPACE(AVFScopedPointer)<AVCaptureDeviceInput> m_audioInput;
   QT_PREPEND_NAMESPACE(AVFScopedPointer)<AVCaptureAudioDataOutput> m_audioOutput;
   QT_PREPEND_NAMESPACE(AVFScopedPointer)<AVAssetWriterInput> m_audioWriterInput;
   AVCaptureDevice *m_audioCaptureDevice;

   // Queue to write sample buffers:
   QT_PREPEND_NAMESPACE(AVFScopedPointer)<dispatch_queue_t> m_writerQueue;

   // High priority serial queue for video output:
   QT_PREPEND_NAMESPACE(AVFScopedPointer)<dispatch_queue_t> m_videoQueue;

   // Serial queue for audio output:
   QT_PREPEND_NAMESPACE(AVFScopedPointer)<dispatch_queue_t> m_audioQueue;

   QT_PREPEND_NAMESPACE(AVFScopedPointer)<AVAssetWriter> m_assetWriter;

   QT_PREPEND_NAMESPACE(AVFMediaRecorderControlIOS) *m_delegate;

   bool m_setStartTime;

   QT_PREPEND_NAMESPACE(QAtomicInt) m_state;

 @public
   QT_PREPEND_NAMESPACE(AVFAtomicInt64) m_durationInMs;
 @private

   CMTime m_startTime;
   CMTime m_lastTimeStamp;

   NSDictionary *m_audioSettings;
   NSDictionary *m_videoSettings;
}

- (id)initWithDelegate: (QT_PREPEND_NAMESPACE(AVFMediaRecorderControlIOS) *)delegate;

- (bool)setupWithFileURL: (NSURL *)fileURL
           cameraService: (QT_PREPEND_NAMESPACE(AVFCameraService) *)service
           audioSettings: (NSDictionary *)audioSettings
           videoSettings: (NSDictionary *)videoSettings
               transform: (CGAffineTransform)transform;

// This to be called from the recorder control's thread:
- (void)start;
- (void)stop;

// This to be called from the recorder control's dtor:
- (void)abort;

@end

#endif
