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

#ifndef DSCAMERASESSION_H
#define DSCAMERASESSION_H

#include <qmutex.h>
#include <qobject.h>
#include <qtime.h>
#include <qurl.h>

#include <qabstractvideosurface.h>
#include <qcamera.h>
#include <qcameraimageprocessingcontrol.h>
#include <qvideoframe.h>
#include <qvideosurfaceformat.h>

#include <qmediastoragelocation_p.h>

#include <tchar.h>
#include <dshow.h>
#include <initguid.h>
#include <objbase.h>

#include <windows.h>

#define __IDxtCompositor_INTERFACE_DEFINED__
#define __IDxtAlphaSetter_INTERFACE_DEFINED__
#define __IDxtJpeg_INTERFACE_DEFINED__
#define __IDxtKey_INTERFACE_DEFINED__

struct ICaptureGraphBuilder2;
struct ISampleGrabber;

class SampleGrabberCallbackPrivate;

class DSCameraSession : public QObject
{
   CS_OBJECT(DSCameraSession)

 public:
   DSCameraSession(QObject *parent = nullptr);
   ~DSCameraSession();

   QCamera::Status status() const {
      return m_status;
   }

   void setDevice(const QString &device);

   bool load();
   bool unload();
   bool startPreview();
   bool stopPreview();

   bool isReadyForCapture();
   int captureImage(const QString &fileName);

   void setSurface(QAbstractVideoSurface *surface);

   QCameraViewfinderSettings viewfinderSettings() const;
   void setViewfinderSettings(const QCameraViewfinderSettings &settings);

   QList<QCameraViewfinderSettings> supportedViewfinderSettings() const {
      return m_supportedViewfinderSettings;
   }

   bool isImageProcessingParameterSupported(QCameraImageProcessingControl::ProcessingParameter) const;

   bool isImageProcessingParameterValueSupported(QCameraImageProcessingControl::ProcessingParameter, const QVariant &) const;
   QVariant imageProcessingParameter(QCameraImageProcessingControl::ProcessingParameter) const;
   void setImageProcessingParameter(QCameraImageProcessingControl::ProcessingParameter, const QVariant &);

   CS_SIGNAL_1(Public, void statusChanged(QCamera::Status newStatus))
   CS_SIGNAL_2(statusChanged, newStatus)

   CS_SIGNAL_1(Public, void imageExposed(int id))
   CS_SIGNAL_2(imageExposed, id)

   CS_SIGNAL_1(Public, void imageCaptured(int id, const QImage &preview))
   CS_SIGNAL_2(imageCaptured, id, preview)

   CS_SIGNAL_1(Public, void imageSaved(int id, const QString &fileName))
   CS_SIGNAL_2(imageSaved, id, fileName)

   CS_SIGNAL_1(Public, void readyForCaptureChanged(bool isReady))
   CS_SIGNAL_2(readyForCaptureChanged, isReady)

   CS_SIGNAL_1(Public, void captureError(int id, int error, const QString &errorString))
   CS_SIGNAL_2(captureError, id, error, errorString)

 private:
   struct ImageProcessingParameterInfo {
      ImageProcessingParameterInfo()
         : minimumValue(0), maximumValue(0), defaultValue(0), currentValue(0), capsFlags(0),
           hasBeenExplicitlySet(false), videoProcAmpProperty(VideoProcAmp_Brightness) {
      }

      LONG minimumValue;
      LONG maximumValue;
      LONG defaultValue;
      LONG currentValue;
      LONG capsFlags;
      bool hasBeenExplicitlySet;
      VideoProcAmpProperty videoProcAmpProperty;
   };

   void setStatus(QCamera::Status status);

   void onFrameAvailable(const char *frameData, long len);
   void saveCapturedImage(int id, const QImage &image, const QString &path);

   bool createFilterGraph();
   bool connectGraph();
   void disconnectGraph();
   void updateSourceCapabilities();
   bool configurePreviewFormat();
   void updateImageProcessingParametersInfos();

   // These static functions are used for scaling of adjustable parameters,
   // which have the ranges from -1.0 to +1.0 in the QCameraImageProcessing API.
   static qreal scaledImageProcessingParameterValue(const ImageProcessingParameterInfo &sourceValueInfo);
   static qint32 sourceImageProcessingParameterValue(qreal scaledValue, const ImageProcessingParameterInfo &sourceValueInfo);

   QMutex m_presentMutex;
   QMutex m_captureMutex;

   // Capture Graph
   ICaptureGraphBuilder2 *m_graphBuilder;
   IGraphBuilder *m_filterGraph;

   // Source (camera)
   QString m_sourceDeviceName;
   IBaseFilter *m_sourceFilter;
   bool m_needsHorizontalMirroring;
   QList<AM_MEDIA_TYPE> m_supportedFormats;
   QList<QCameraViewfinderSettings> m_supportedViewfinderSettings;
   AM_MEDIA_TYPE m_sourceFormat;
   QMap<QCameraImageProcessingControl::ProcessingParameter, ImageProcessingParameterInfo> m_imageProcessingParametersInfos;

   // Preview
   IBaseFilter *m_previewFilter;
   ISampleGrabber *m_previewSampleGrabber;
   IBaseFilter *m_nullRendererFilter;
   QVideoFrame m_currentFrame;
   bool m_previewStarted;
   QAbstractVideoSurface *m_surface;
   QVideoSurfaceFormat m_previewSurfaceFormat;
   QVideoFrame::PixelFormat m_previewPixelFormat;
   QSize m_previewSize;
   QCameraViewfinderSettings m_viewfinderSettings;
   QCameraViewfinderSettings m_actualViewfinderSettings;

   // Image capture
   QString m_imageCaptureFileName;
   QMediaStorageLocation m_fileNameGenerator;
   bool m_readyForCapture;
   int m_imageIdCounter;
   int m_currentImageId;
   QVideoFrame m_capturedFrame;

   // Internal state
   QCamera::Status m_status;

   CS_SLOT_1(Private, void presentFrame())
   CS_SLOT_2(presentFrame)

   CS_SLOT_1(Private, void updateReadyForCapture())
   CS_SLOT_2(updateReadyForCapture)

   friend class SampleGrabberCallbackPrivate;
};

#endif
