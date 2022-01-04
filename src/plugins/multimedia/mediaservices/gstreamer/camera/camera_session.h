/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

#ifndef CAMERA_SESSION_H
#define CAMERA_SESSION_H

#include <qcamera.h>
#include <qmediarecordercontrol.h>
#include <qurl.h>
#include <qdir.h>

#include <qgstreamerbushelper_p.h>
#include <qgstreamerbufferprobe_p.h>
#include <qmediastoragelocation_p.h>

#include <gst/gst.h>
#ifdef HAVE_GST_PHOTOGRAPHY
#include <gst/interfaces/photography.h>
#endif

class QGstreamerMessage;
class QGstreamerBusHelper;
class CameraBinControl;
class CameraBinAudioEncoder;
class CameraBinVideoEncoder;
class CameraBinImageEncoder;
class CameraBinRecorder;
class CameraBinContainer;
class CameraBinExposure;
class CameraBinFlash;
class CameraBinFocus;
class CameraBinImageProcessing;
class CameraBinLocks;
class CameraBinZoom;
class CameraBinCaptureDestination;
class CameraBinCaptureBufferFormat;
class QGstreamerVideoRendererInterface;
class CameraBinViewfinderSettings;

class QGstreamerElementFactory
{
 public:
   virtual GstElement *buildElement() = 0;
};

class CameraBinSession : public QObject, public QGstreamerBusMessageFilter, public QGstreamerSyncMessageFilter
{
   CS_OBJECT(CameraBinSession)

   CS_PROPERTY_READ(duration, duration)
   CS_PROPERTY_NOTIFY(duration, durationChanged)

   CS_INTERFACES(QGstreamerBusMessageFilter, QGstreamerSyncMessageFilter)

 public:
   CameraBinSession(GstElementFactory *sourceFactory, QObject *parent);
   ~CameraBinSession();

#ifdef HAVE_GST_PHOTOGRAPHY
   GstPhotography *photography();
#endif

   GstElement *cameraBin() {
      return m_camerabin;
   }

   GstElement *cameraSource() {
      return m_cameraSrc;
   }

   QGstreamerBusHelper *bus() {
      return m_busHelper;
   }

   QList< QPair<int, int> > supportedFrameRates(const QSize &frameSize, bool *continuous) const;
   QList<QSize> supportedResolutions(QPair<int, int> rate, bool *continuous, QCamera::CaptureModes mode) const;

   QCamera::CaptureModes captureMode() {
      return m_captureMode;
   }

   void setCaptureMode(QCamera::CaptureModes mode);

   QUrl outputLocation() const;
   bool setOutputLocation(const QUrl &sink);

   GstElement *buildCameraSource();
   GstElementFactory *sourceFactory() const {
      return m_sourceFactory;
   }

   CameraBinControl *cameraControl() const {
      return m_cameraControl;
   }
   CameraBinAudioEncoder *audioEncodeControl() const {
      return m_audioEncodeControl;
   }
   CameraBinVideoEncoder *videoEncodeControl() const {
      return m_videoEncodeControl;
   }
   CameraBinImageEncoder *imageEncodeControl() const {
      return m_imageEncodeControl;
   }

#ifdef HAVE_GST_PHOTOGRAPHY
   CameraBinExposure *cameraExposureControl();
   CameraBinFlash *cameraFlashControl();
   CameraBinFocus *cameraFocusControl();
   CameraBinLocks *cameraLocksControl();
#endif

   CameraBinZoom *cameraZoomControl() const {
      return m_cameraZoomControl;
   }
   CameraBinImageProcessing *imageProcessingControl() const {
      return m_imageProcessingControl;
   }
   CameraBinCaptureDestination *captureDestinationControl() const {
      return m_captureDestinationControl;
   }
   CameraBinCaptureBufferFormat *captureBufferFormatControl() const {
      return m_captureBufferFormatControl;
   }

   CameraBinRecorder *recorderControl() const {
      return m_recorderControl;
   }
   CameraBinContainer *mediaContainerControl() const {
      return m_mediaContainerControl;
   }

   QGstreamerElementFactory *audioInput() const {
      return m_audioInputFactory;
   }
   void setAudioInput(QGstreamerElementFactory *audioInput);

   QGstreamerElementFactory *videoInput() const {
      return m_videoInputFactory;
   }
   void setVideoInput(QGstreamerElementFactory *videoInput);
   bool isReady() const;

   QObject *viewfinder() const {
      return m_viewfinder;
   }
   void setViewfinder(QObject *viewfinder);

   QList<QCameraViewfinderSettings> supportedViewfinderSettings() const;
   QCameraViewfinderSettings viewfinderSettings() const;
   void setViewfinderSettings(const QCameraViewfinderSettings &settings) {
      m_viewfinderSettings = settings;
   }

   void captureImage(int requestId, const QString &fileName);

   QCamera::Status status() const;
   QCamera::State pendingState() const;
   bool isBusy() const;

   qint64 duration() const;

   void recordVideo();
   void stopVideoRecording();

   bool isMuted() const;

   QString device() const {
      return m_inputDevice;
   }

   bool processSyncMessage(const QGstreamerMessage &message);
   bool processBusMessage(const QGstreamerMessage &message);

 public:
   CS_SIGNAL_1(Public, void statusChanged(QCamera::Status status))
   CS_SIGNAL_2(statusChanged, status)
   CS_SIGNAL_1(Public, void pendingStateChanged(QCamera::State state))
   CS_SIGNAL_2(pendingStateChanged, state)
   CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   CS_SIGNAL_2(durationChanged, duration)
   CS_SIGNAL_1(Public, void error(int error, const QString &errorString))
   CS_SIGNAL_2(error, error, errorString)
   CS_SIGNAL_1(Public, void imageExposed(int requestId))
   CS_SIGNAL_2(imageExposed, requestId)
   CS_SIGNAL_1(Public, void imageCaptured(int requestId, const QImage &img))
   CS_SIGNAL_2(imageCaptured, requestId, img)
   CS_SIGNAL_1(Public, void mutedChanged(bool un_named_arg1))
   CS_SIGNAL_2(mutedChanged, un_named_arg1)
   CS_SIGNAL_1(Public, void viewfinderChanged())
   CS_SIGNAL_2(viewfinderChanged)
   CS_SIGNAL_1(Public, void readyChanged(bool un_named_arg1))
   CS_SIGNAL_2(readyChanged, un_named_arg1)
   CS_SIGNAL_1(Public, void busyChanged(bool un_named_arg1))
   CS_SIGNAL_2(busyChanged, un_named_arg1)

 public :
   CS_SLOT_1(Public, void setDevice(const QString &device))
   CS_SLOT_2(setDevice)
   CS_SLOT_1(Public, void setState(QCamera::State un_named_arg1))
   CS_SLOT_2(setState)
   CS_SLOT_1(Public, void setCaptureDevice(const QString &deviceName))
   CS_SLOT_2(setCaptureDevice)
   CS_SLOT_1(Public, void setMetaData(const QMap <QByteArray, QVariant> &un_named_arg1))
   CS_SLOT_2(setMetaData)
   CS_SLOT_1(Public, void setMuted(bool un_named_arg1))
   CS_SLOT_2(setMuted)

 private :
   CS_SLOT_1(Private, void handleViewfinderChange())
   CS_SLOT_2(handleViewfinderChange)
   CS_SLOT_1(Private, void setupCaptureResolution())
   CS_SLOT_2(setupCaptureResolution)

 private:
   void load();
   void unload();
   void start();
   void stop();

   void setStatus(QCamera::Status status);
   void setStateHelper(QCamera::State state);
   void setError(int error, const QString &errorString);

   bool setupCameraBin();
   void setAudioCaptureCaps();
   GstCaps *supportedCaps(QCamera::CaptureModes mode) const;
   void updateSupportedViewfinderSettings();
   static void updateBusyStatus(GObject *o, GParamSpec *p, gpointer d);

   QString currentContainerFormat() const;

   static void elementAdded(GstBin *bin, GstElement *element, CameraBinSession *session);
   static void elementRemoved(GstBin *bin, GstElement *element, CameraBinSession *session);

   QUrl m_sink;
   QUrl m_actualSink;
   bool m_recordingActive;
   QString m_captureDevice;
   QCamera::Status m_status;
   QCamera::State m_pendingState;
   QString m_inputDevice;
   bool m_muted;
   bool m_busy;
   QMediaStorageLocation m_mediaStorageLocation;

   QCamera::CaptureModes m_captureMode;
   QMap<QByteArray, QVariant> m_metaData;

   QGstreamerElementFactory *m_audioInputFactory;
   QGstreamerElementFactory *m_videoInputFactory;
   QObject *m_viewfinder;
   QGstreamerVideoRendererInterface *m_viewfinderInterface;
   QList<QCameraViewfinderSettings> m_supportedViewfinderSettings;
   QCameraViewfinderSettings m_viewfinderSettings;
   QCameraViewfinderSettings m_actualViewfinderSettings;

   CameraBinControl *m_cameraControl;
   CameraBinAudioEncoder *m_audioEncodeControl;
   CameraBinVideoEncoder *m_videoEncodeControl;
   CameraBinImageEncoder *m_imageEncodeControl;
   CameraBinRecorder *m_recorderControl;
   CameraBinContainer *m_mediaContainerControl;
#ifdef HAVE_GST_PHOTOGRAPHY
   CameraBinExposure *m_cameraExposureControl;
   CameraBinFlash *m_cameraFlashControl;
   CameraBinFocus *m_cameraFocusControl;
   CameraBinLocks *m_cameraLocksControl;
#endif
   CameraBinZoom *m_cameraZoomControl;
   CameraBinImageProcessing *m_imageProcessingControl;
   CameraBinCaptureDestination *m_captureDestinationControl;
   CameraBinCaptureBufferFormat *m_captureBufferFormatControl;

   QGstreamerBusHelper *m_busHelper;
   GstBus *m_bus;
   GstElement *m_camerabin;
   GstElement *m_cameraSrc;
   GstElement *m_videoSrc;
   GstElement *m_viewfinderElement;
   GstElementFactory *m_sourceFactory;
   bool m_viewfinderHasChanged;
   bool m_inputDeviceHasChanged;
   bool m_usingWrapperCameraBinSrc;

   class ViewfinderProbe : public QGstreamerBufferProbe
   {
    public:
      ViewfinderProbe(CameraBinSession *s)
         : QGstreamerBufferProbe(QGstreamerBufferProbe::ProbeCaps)
         , session(s)
      {}

      void probeCaps(GstCaps *caps);

    private:
      CameraBinSession *const session;
   } m_viewfinderProbe;

   GstElement *m_audioSrc;
   GstElement *m_audioConvert;
   GstElement *m_capsFilter;
   GstElement *m_fileSink;
   GstElement *m_audioEncoder;
   GstElement *m_videoEncoder;
   GstElement *m_muxer;

 public:
   QString m_imageFileName;
   int m_requestId;
};

#endif
