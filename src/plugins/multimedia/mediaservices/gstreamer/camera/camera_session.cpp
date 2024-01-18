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

#include <camera_session.h>
#include <camera_control.h>
#include <camera_recorder.h>
#include <camera_container.h>
#include <camera_audioencoder.h>
#include <camera_videoencoder.h>
#include <camera_imageencoder.h>

#ifdef HAVE_GST_PHOTOGRAPHY
#include <camera_exposure.h>
#include <camera_flash.h>
#include <camera_focus.h>
#include <camera_locks.h>
#endif

#include <camera_zoom.h>
#include <camera_imageprocessing.h>
#include <camera_viewfindersettings.h>
#include <camera_capturedestination.h>
#include <camera_capturebufferformat.h>

#include <qdebug.h>
#include <qcoreapplication.h>
#include <qmetaobject.h>
#include <qdesktopservices.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qmediarecorder.h>
#include <qvideosurfaceformat.h>

#include <qgstreamerbushelper_p.h>
#include <qgstreamervideorendererinterface_p.h>
#include <qgstutils_p.h>

#ifdef HAVE_GST_PHOTOGRAPHY
#include <gst/interfaces/photography.h>
#endif

#include <gst/gsttagsetter.h>
#include <gst/gstversion.h>

// #define CAMERABIN_DEBUG 1
// #define CAMERABIN_DEBUG_DUMP_BIN 1

#define ENUM_NAME(c,e,v) (c::staticMetaObject.enumerator(c::staticMetaObject.indexOfEnumerator(e)).valueToKey((v)))

#define FILENAME_PROPERTY "location"
#define MODE_PROPERTY "mode"
#define MUTE_PROPERTY "mute"
#define IMAGE_PP_PROPERTY "image-post-processing"
#define IMAGE_ENCODER_PROPERTY "image-encoder"
#define VIDEO_PP_PROPERTY "video-post-processing"
#define VIEWFINDER_SINK_PROPERTY "viewfinder-sink"
#define CAMERA_SOURCE_PROPERTY "camera-source"
#define AUDIO_SOURCE_PROPERTY "audio-source"
#define SUPPORTED_IMAGE_CAPTURE_CAPS_PROPERTY "image-capture-supported-caps"
#define SUPPORTED_VIDEO_CAPTURE_CAPS_PROPERTY "video-capture-supported-caps"
#define SUPPORTED_VIEWFINDER_CAPS_PROPERTY "viewfinder-supported-caps"
#define AUDIO_CAPTURE_CAPS_PROPERTY "audio-capture-caps"
#define IMAGE_CAPTURE_CAPS_PROPERTY "image-capture-caps"
#define VIDEO_CAPTURE_CAPS_PROPERTY "video-capture-caps"
#define VIEWFINDER_CAPS_PROPERTY "viewfinder-caps"
#define PREVIEW_CAPS_PROPERTY "preview-caps"
#define POST_PREVIEWS_PROPERTY "post-previews"


#define CAPTURE_START "start-capture"
#define CAPTURE_STOP "stop-capture"

#define FILESINK_BIN_NAME "videobin-filesink"

#define CAMERABIN_IMAGE_MODE 1
#define CAMERABIN_VIDEO_MODE 2

#define PREVIEW_CAPS_4_3 \
    "video/x-raw-rgb, width = (int) 640, height = (int) 480"

CameraBinSession::CameraBinSession(GstElementFactory *sourceFactory, QObject *parent)
   : QObject(parent),
     m_recordingActive(false),
     m_status(QCamera::UnloadedStatus),
     m_pendingState(QCamera::UnloadedState),
     m_muted(false),
     m_busy(false),
     m_captureMode(QCamera::CaptureStillImage),
     m_audioInputFactory(nullptr),
     m_videoInputFactory(nullptr),
     m_viewfinder(nullptr),
     m_viewfinderInterface(nullptr),
#ifdef HAVE_GST_PHOTOGRAPHY
     m_cameraExposureControl(nullptr),
     m_cameraFlashControl(nullptr),
     m_cameraFocusControl(nullptr),
     m_cameraLocksControl(nullptr),
#endif
     m_cameraSrc(nullptr),
     m_videoSrc(nullptr),
     m_viewfinderElement(nullptr),
     m_sourceFactory(sourceFactory),
     m_viewfinderHasChanged(true),
     m_inputDeviceHasChanged(true),
     m_usingWrapperCameraBinSrc(false),
     m_viewfinderProbe(this),
     m_audioSrc(nullptr),
     m_audioConvert(nullptr),
     m_capsFilter(nullptr),
     m_fileSink(nullptr),
     m_audioEncoder(nullptr),
     m_videoEncoder(nullptr),
     m_muxer(nullptr)
{
   if (m_sourceFactory) {
      gst_object_ref(GST_OBJECT(m_sourceFactory));
   }
   m_camerabin = gst_element_factory_make(QT_GSTREAMER_CAMERABIN_ELEMENT_NAME, "camerabin");

   g_signal_connect(G_OBJECT(m_camerabin), "notify::idle", G_CALLBACK(updateBusyStatus), this);
   g_signal_connect(G_OBJECT(m_camerabin), "element-added",  G_CALLBACK(elementAdded), this);
   g_signal_connect(G_OBJECT(m_camerabin), "element-removed",  G_CALLBACK(elementRemoved), this);
   qt_gst_object_ref_sink(m_camerabin);

   m_bus = gst_element_get_bus(m_camerabin);

   m_busHelper = new QGstreamerBusHelper(m_bus, this);
   m_busHelper->installMessageFilter(this);

   m_cameraControl = new CameraBinControl(this);
   m_audioEncodeControl = new CameraBinAudioEncoder(this);
   m_videoEncodeControl = new CameraBinVideoEncoder(this);
   m_imageEncodeControl = new CameraBinImageEncoder(this);
   m_recorderControl = new CameraBinRecorder(this);
   m_mediaContainerControl = new CameraBinContainer(this);
   m_cameraZoomControl = new CameraBinZoom(this);
   m_imageProcessingControl = new CameraBinImageProcessing(this);
   m_captureDestinationControl = new CameraBinCaptureDestination(this);
   m_captureBufferFormatControl = new CameraBinCaptureBufferFormat(this);

   QByteArray envFlags = qgetenv("QT_GSTREAMER_CAMERABIN_FLAGS");
   if (!envFlags.isEmpty()) {
      g_object_set(G_OBJECT(m_camerabin), "flags", envFlags.toInt(), nullptr);
   }

   //post image preview in RGB format
   g_object_set(G_OBJECT(m_camerabin), POST_PREVIEWS_PROPERTY, TRUE, nullptr);

#if GST_CHECK_VERSION(1,0,0)
   GstCaps *previewCaps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "RGBx", nullptr);
#else
   GstCaps *previewCaps = gst_caps_from_string("video/x-raw-rgb");
#endif

   g_object_set(G_OBJECT(m_camerabin), PREVIEW_CAPS_PROPERTY, previewCaps, nullptr);
   gst_caps_unref(previewCaps);
}

CameraBinSession::~CameraBinSession()
{
   if (m_camerabin) {
      if (m_viewfinderInterface) {
         m_viewfinderInterface->stopRenderer();
      }

      gst_element_set_state(m_camerabin, GST_STATE_NULL);
      gst_element_get_state(m_camerabin, nullptr, nullptr, GST_CLOCK_TIME_NONE);
      gst_object_unref(GST_OBJECT(m_bus));
      gst_object_unref(GST_OBJECT(m_camerabin));
   }
   if (m_viewfinderElement) {
      gst_object_unref(GST_OBJECT(m_viewfinderElement));
   }

   if (m_sourceFactory) {
      gst_object_unref(GST_OBJECT(m_sourceFactory));
   }
}

#ifdef HAVE_GST_PHOTOGRAPHY
GstPhotography *CameraBinSession::photography()
{
   if (GST_IS_PHOTOGRAPHY(m_camerabin)) {
      return GST_PHOTOGRAPHY(m_camerabin);
   }

   GstElement *const source = buildCameraSource();

   if (source && GST_IS_PHOTOGRAPHY(source)) {
      return GST_PHOTOGRAPHY(source);
   }

   return 0;
}

CameraBinExposure *CameraBinSession::cameraExposureControl()
{
   if (!m_cameraExposureControl && photography()) {
      m_cameraExposureControl = new CameraBinExposure(this);
   }
   return m_cameraExposureControl;
}

CameraBinFlash *CameraBinSession::cameraFlashControl()
{
   if (!m_cameraFlashControl && photography()) {
      m_cameraFlashControl = new CameraBinFlash(this);
   }
   return m_cameraFlashControl;
}

CameraBinFocus *CameraBinSession::cameraFocusControl()
{
   if (!m_cameraFocusControl && photography()) {
      m_cameraFocusControl = new CameraBinFocus(this);
   }
   return m_cameraFocusControl;
}

CameraBinLocks *CameraBinSession::cameraLocksControl()
{
   if (!m_cameraLocksControl && photography()) {
      m_cameraLocksControl = new CameraBinLocks(this);
   }
   return m_cameraLocksControl;
}
#endif

bool CameraBinSession::setupCameraBin()
{
   if (!buildCameraSource()) {
      return false;
   }

   if (m_viewfinderHasChanged) {
      if (m_viewfinderElement) {
         GstPad *pad = gst_element_get_static_pad(m_viewfinderElement, "sink");
         m_viewfinderProbe.removeProbeFromPad(pad);
         gst_object_unref(GST_OBJECT(pad));
         gst_object_unref(GST_OBJECT(m_viewfinderElement));
      }

      m_viewfinderElement = m_viewfinderInterface ? m_viewfinderInterface->videoSink() : nullptr;
#if CAMERABIN_DEBUG
      qDebug() << Q_FUNC_INFO << "Viewfinder changed, reconfigure.";
#endif
      m_viewfinderHasChanged = false;
      if (!m_viewfinderElement) {
         if (m_pendingState == QCamera::ActiveState) {
            qWarning() << "Starting camera without viewfinder available";
         }
         m_viewfinderElement = gst_element_factory_make("fakesink", nullptr);
      }

      GstPad *pad = gst_element_get_static_pad(m_viewfinderElement, "sink");
      m_viewfinderProbe.addProbeToPad(pad);
      gst_object_unref(GST_OBJECT(pad));

      g_object_set(G_OBJECT(m_viewfinderElement), "sync", FALSE, nullptr);
      qt_gst_object_ref_sink(GST_OBJECT(m_viewfinderElement));
      gst_element_set_state(m_camerabin, GST_STATE_NULL);
      g_object_set(G_OBJECT(m_camerabin), VIEWFINDER_SINK_PROPERTY, m_viewfinderElement, nullptr);
   }

   return true;
}

static GstCaps *resolutionToCaps(const QSize &resolution,
                                 qreal frameRate = 0.0,
                                 QVideoFrame::PixelFormat pixelFormat = QVideoFrame::Format_Invalid)
{
   GstCaps *caps = nullptr;
   if (pixelFormat == QVideoFrame::Format_Invalid) {
      caps = QGstUtils::videoFilterCaps();
   } else {
      caps = QGstUtils::capsForFormats(QList<QVideoFrame::PixelFormat>() << pixelFormat);
   }

   if (!resolution.isEmpty()) {
      gst_caps_set_simple(
         caps,
         "width", G_TYPE_INT, resolution.width(),
         "height", G_TYPE_INT, resolution.height(),
         nullptr);
   }

   if (frameRate > 0.0) {
      gint numerator;
      gint denominator;
      qt_gst_util_double_to_fraction(frameRate, &numerator, &denominator);

      gst_caps_set_simple(
         caps,
         "framerate", GST_TYPE_FRACTION, numerator, denominator,
         nullptr);
   }

   return caps;
}

void CameraBinSession::setupCaptureResolution()
{
   QSize viewfinderResolution = m_viewfinderSettings.resolution();
   qreal viewfinderFrameRate = m_viewfinderSettings.maximumFrameRate();
   QVideoFrame::PixelFormat viewfinderPixelFormat = m_viewfinderSettings.pixelFormat();
   const QSize imageResolution = m_imageEncodeControl->imageSettings().resolution();
   const QSize videoResolution = m_videoEncodeControl->actualVideoSettings().resolution();

   // WrapperCameraBinSrc cannot have different caps on its imgsrc, vidsrc and vfsrc pads.
   // If capture resolution is specified, use it also for the viewfinder to avoid caps negotiation
   // to fail.
   if (m_usingWrapperCameraBinSrc) {
      if (m_captureMode == QCamera::CaptureStillImage && !imageResolution.isEmpty()) {
         viewfinderResolution = imageResolution;
      } else if (m_captureMode == QCamera::CaptureVideo && !videoResolution.isEmpty()) {
         viewfinderResolution = videoResolution;
      }

      // Make sure we don't use incompatible frame rate and pixel format with the new resolution
      if (viewfinderResolution != m_viewfinderSettings.resolution() &&
            (!qFuzzyIsNull(viewfinderFrameRate) || viewfinderPixelFormat != QVideoFrame::Format_Invalid)) {

         enum {
            Nothing = 0x0,
            OnlyFrameRate = 0x1,
            OnlyPixelFormat = 0x2,
            Both = 0x4
         };
         quint8 found = Nothing;

         for (int i = 0; i < m_supportedViewfinderSettings.count() && !(found & Both); ++i) {
            const QCameraViewfinderSettings &s = m_supportedViewfinderSettings.at(i);
            if (s.resolution() == viewfinderResolution) {
               if ((qFuzzyIsNull(viewfinderFrameRate) || s.maximumFrameRate() == viewfinderFrameRate)
                     && (viewfinderPixelFormat == QVideoFrame::Format_Invalid || s.pixelFormat() == viewfinderPixelFormat)) {
                  found |= Both;
               } else if (s.maximumFrameRate() == viewfinderFrameRate) {
                  found |= OnlyFrameRate;
               } else if (s.pixelFormat() == viewfinderPixelFormat) {
                  found |= OnlyPixelFormat;
               }
            }
         }

         if (found & Both) {
            // no-op
         } else if (found & OnlyPixelFormat) {
            viewfinderFrameRate = qreal(0);
         } else if (found & OnlyFrameRate) {
            viewfinderPixelFormat = QVideoFrame::Format_Invalid;
         } else {
            viewfinderPixelFormat = QVideoFrame::Format_Invalid;
            viewfinderFrameRate = qreal(0);
         }
      }
   }

   GstCaps *caps = resolutionToCaps(imageResolution);
   g_object_set(m_camerabin, IMAGE_CAPTURE_CAPS_PROPERTY, caps, nullptr);
   gst_caps_unref(caps);

   qreal framerate = m_videoEncodeControl->videoSettings().frameRate();
   caps = resolutionToCaps(videoResolution, framerate);
   g_object_set(m_camerabin, VIDEO_CAPTURE_CAPS_PROPERTY, caps, nullptr);
   gst_caps_unref(caps);

   caps = resolutionToCaps(viewfinderResolution, viewfinderFrameRate, viewfinderPixelFormat);
   g_object_set(m_camerabin, VIEWFINDER_CAPS_PROPERTY, caps, nullptr);
   gst_caps_unref(caps);

   // Special case when using mfw_v4lsrc
   if (m_videoSrc && qstrcmp(qt_gst_element_get_factory_name(m_videoSrc), "mfw_v4lsrc") == 0) {
      int capMode = 0;
      if (viewfinderResolution == QSize(320, 240)) {
         capMode = 1;
      } else if (viewfinderResolution == QSize(720, 480)) {
         capMode = 2;
      } else if (viewfinderResolution == QSize(720, 576)) {
         capMode = 3;
      } else if (viewfinderResolution == QSize(1280, 720)) {
         capMode = 4;
      } else if (viewfinderResolution == QSize(1920, 1080)) {
         capMode = 5;
      }
      g_object_set(G_OBJECT(m_videoSrc), "capture-mode", capMode, nullptr);

      if (!qFuzzyIsNull(viewfinderFrameRate)) {
         int n, d;
         qt_gst_util_double_to_fraction(viewfinderFrameRate, &n, &d);
         g_object_set(G_OBJECT(m_videoSrc), "fps-n", n, nullptr);
         g_object_set(G_OBJECT(m_videoSrc), "fps-d", d, nullptr);
      }
   }

   if (m_videoEncoder) {
      m_videoEncodeControl->applySettings(m_videoEncoder);
   }
}

void CameraBinSession::setAudioCaptureCaps()
{
   QAudioEncoderSettings settings = m_audioEncodeControl->audioSettings();
   const int sampleRate = settings.sampleRate();
   const int channelCount = settings.channelCount();

   if (sampleRate <= 0 && channelCount <= 0) {
      return;
   }

#if GST_CHECK_VERSION(1,0,0)
   GstStructure *structure = gst_structure_new_empty(QT_GSTREAMER_RAW_AUDIO_MIME);
#else
   GstStructure *structure = gst_structure_new(
                                QT_GSTREAMER_RAW_AUDIO_MIME,
                                "endianness", G_TYPE_INT, 1234,
                                "signed", G_TYPE_BOOLEAN, TRUE,
                                "width", G_TYPE_INT, 16,
                                "depth", G_TYPE_INT, 16,
                                nullptr);
#endif
   if (sampleRate > 0) {
      gst_structure_set(structure, "rate", G_TYPE_INT, sampleRate, nullptr);
   }
   if (channelCount > 0) {
      gst_structure_set(structure, "channels", G_TYPE_INT, channelCount, nullptr);
   }

   GstCaps *caps = gst_caps_new_full(structure, nullptr);
   g_object_set(G_OBJECT(m_camerabin), AUDIO_CAPTURE_CAPS_PROPERTY, caps, nullptr);
   gst_caps_unref(caps);

   if (m_audioEncoder) {
      m_audioEncodeControl->applySettings(m_audioEncoder);
   }
}

GstElement *CameraBinSession::buildCameraSource()
{
#if CAMERABIN_DEBUG
   qDebug() << Q_FUNC_INFO;
#endif
   if (!m_inputDeviceHasChanged) {
      return m_cameraSrc;
   }

   m_inputDeviceHasChanged = false;
   m_usingWrapperCameraBinSrc = false;

   GstElement *camSrc = nullptr;
   g_object_get(G_OBJECT(m_camerabin), CAMERA_SOURCE_PROPERTY, &camSrc, nullptr);

   if (!m_cameraSrc && m_sourceFactory) {
      m_cameraSrc = gst_element_factory_create(m_sourceFactory, "camera_source");
   }

   // If gstreamer has set a default source use it.
   if (!m_cameraSrc) {
      m_cameraSrc = camSrc;
   }

   if (m_cameraSrc && !m_inputDevice.isEmpty()) {
#if CAMERABIN_DEBUG
      qDebug() << "set camera device" << m_inputDevice;
#endif
      m_usingWrapperCameraBinSrc = qstrcmp(qt_gst_element_get_factory_name(m_cameraSrc), "wrappercamerabinsrc") == 0;

      if (g_object_class_find_property(G_OBJECT_GET_CLASS(m_cameraSrc), "video-source")) {
         if (!m_videoSrc) {
            /* QT_GSTREAMER_CAMERABIN_VIDEOSRC can be used to set the video source element.

               --- Usage

                 QT_GSTREAMER_CAMERABIN_VIDEOSRC=[drivername=elementname[,drivername2=elementname2 ...],][elementname]

               --- Examples

                 Always use 'somevideosrc':
                 QT_GSTREAMER_CAMERABIN_VIDEOSRC="somevideosrc"

                 Use 'somevideosrc' when the device driver is 'somedriver', otherwise use default:
                 QT_GSTREAMER_CAMERABIN_VIDEOSRC="somedriver=somevideosrc"

                 Use 'somevideosrc' when the device driver is 'somedriver', otherwise use 'somevideosrc2'
                 QT_GSTREAMER_CAMERABIN_VIDEOSRC="somedriver=somevideosrc,somevideosrc2"
            */
            const QByteArray envVideoSource = qgetenv("QT_GSTREAMER_CAMERABIN_VIDEOSRC");

            if (!envVideoSource.isEmpty()) {
               QList<QByteArray> sources = envVideoSource.split(',');

               for (const QByteArray &source : sources) {
                  QList<QByteArray> keyValue = source.split('=');

                  if (keyValue.count() == 1) {
                     m_videoSrc = gst_element_factory_make(keyValue[0].constData(), "camera_source");
                     break;

                  } else if (keyValue.at(0) == QGstUtils::cameraDriver(m_inputDevice, m_sourceFactory)) {
                     m_videoSrc = gst_element_factory_make(keyValue[1].constData(), "camera_source");
                     break;
                  }
               }
            } else if (m_videoInputFactory) {
               m_videoSrc = m_videoInputFactory->buildElement();
            }

            if (! m_videoSrc) {
               m_videoSrc = gst_element_factory_make("v4l2src", "camera_source");
            }

            if (m_videoSrc) {
               g_object_set(G_OBJECT(m_cameraSrc), "video-source", m_videoSrc, nullptr);
            }
         }

         if (m_videoSrc) {
            g_object_set(G_OBJECT(m_videoSrc), "device", m_inputDevice.toUtf8().constData(), nullptr);
         }

      } else if (g_object_class_find_property(G_OBJECT_GET_CLASS(m_cameraSrc), "camera-device")) {
         if (m_inputDevice == "secondary") {
            g_object_set(G_OBJECT(m_cameraSrc), "camera-device", 1, nullptr);
         } else {
            g_object_set(G_OBJECT(m_cameraSrc), "camera-device", 0, nullptr);
         }
      }
   }

   if (m_cameraSrc != camSrc) {
      g_object_set(G_OBJECT(m_camerabin), CAMERA_SOURCE_PROPERTY, m_cameraSrc, nullptr);
   }

   if (camSrc) {
      gst_object_unref(GST_OBJECT(camSrc));
   }

   return m_cameraSrc;
}

void CameraBinSession::captureImage(int requestId, const QString &fileName)
{
   const QString actualFileName = m_mediaStorageLocation.generateFileName(fileName,
                  QMediaStorageLocation::Pictures, "IMG_", "jpg");

   m_requestId = requestId;

#if CAMERABIN_DEBUG
   qDebug() << Q_FUNC_INFO << m_requestId << fileName << "actual file name:" << actualFileName;
#endif

   g_object_set(G_OBJECT(m_camerabin), FILENAME_PROPERTY, actualFileName.toUtf8().constData(), nullptr);
   g_signal_emit_by_name(G_OBJECT(m_camerabin), CAPTURE_START, nullptr);

   m_imageFileName = actualFileName;
}

void CameraBinSession::setCaptureMode(QCamera::CaptureModes mode)
{
   m_captureMode = mode;

   switch (m_captureMode) {
      case QCamera::CaptureStillImage:
         g_object_set(m_camerabin, MODE_PROPERTY, CAMERABIN_IMAGE_MODE, nullptr);
         break;

      case QCamera::CaptureVideo:
         g_object_set(m_camerabin, MODE_PROPERTY, CAMERABIN_VIDEO_MODE, nullptr);
         break;
   }

   m_recorderControl->updateStatus();
}

QUrl CameraBinSession::outputLocation() const
{
   // return the location service wrote data to, not one set by user, it can be empty.
   return m_actualSink;
}

bool CameraBinSession::setOutputLocation(const QUrl &sink)
{
   if (!sink.isRelative() && !sink.isLocalFile()) {
      qWarning("Output location must be a local file");
      return false;
   }

   m_sink = m_actualSink = sink;
   return true;
}

void CameraBinSession::setDevice(const QString &device)
{
   if (m_inputDevice != device) {
      m_inputDevice = device;
      m_inputDeviceHasChanged = true;
   }
}

void CameraBinSession::setAudioInput(QGstreamerElementFactory *audioInput)
{
   m_audioInputFactory = audioInput;
}

void CameraBinSession::setVideoInput(QGstreamerElementFactory *videoInput)
{
   m_videoInputFactory = videoInput;
   m_inputDeviceHasChanged = true;
}

bool CameraBinSession::isReady() const
{
   // it is possible to use QCamera without any viewfinder attached
   return !m_viewfinderInterface || m_viewfinderInterface->isReady();
}

void CameraBinSession::setViewfinder(QObject *viewfinder)
{
   if (m_viewfinderInterface) {
      m_viewfinderInterface->stopRenderer();
   }

   m_viewfinderInterface = dynamic_cast<QGstreamerVideoRendererInterface *>(viewfinder);
   if (!m_viewfinderInterface) {
      viewfinder = nullptr;
   }

   if (m_viewfinder != viewfinder) {
      bool oldReady = isReady();

      if (m_viewfinder) {
         disconnect(m_viewfinder, SIGNAL(sinkChanged()), this, SLOT(handleViewfinderChange()));
         disconnect(m_viewfinder, SIGNAL(readyChanged(bool)), this, SLOT(readyChanged(bool)));

         m_busHelper->removeMessageFilter(m_viewfinder);
      }

      m_viewfinder = viewfinder;
      m_viewfinderHasChanged = true;

      if (m_viewfinder) {
         connect(m_viewfinder, SIGNAL(sinkChanged()), this, SLOT(handleViewfinderChange()));
         connect(m_viewfinder, SIGNAL(readyChanged(bool)), this, SLOT(readyChanged(bool)));

         m_busHelper->installMessageFilter(m_viewfinder);
      }

      emit viewfinderChanged();
      if (oldReady != isReady()) {
         emit readyChanged(isReady());
      }
   }
}

QList<QCameraViewfinderSettings> CameraBinSession::supportedViewfinderSettings() const
{
   return m_supportedViewfinderSettings;
}

QCameraViewfinderSettings CameraBinSession::viewfinderSettings() const
{
   return m_status == QCamera::ActiveStatus ? m_actualViewfinderSettings : m_viewfinderSettings;
}

void CameraBinSession::ViewfinderProbe::probeCaps(GstCaps *caps)
{
   // Update actual viewfinder settings on viewfinder caps change
   const GstStructure *s = gst_caps_get_structure(caps, 0);
   const QPair<qreal, qreal> frameRate = QGstUtils::structureFrameRateRange(s);
   session->m_actualViewfinderSettings.setResolution(QGstUtils::structureResolution(s));
   session->m_actualViewfinderSettings.setMinimumFrameRate(frameRate.first);
   session->m_actualViewfinderSettings.setMaximumFrameRate(frameRate.second);
   session->m_actualViewfinderSettings.setPixelFormat(QGstUtils::structurePixelFormat(s));
   session->m_actualViewfinderSettings.setPixelAspectRatio(QGstUtils::structurePixelAspectRatio(s));
}

void CameraBinSession::handleViewfinderChange()
{
   //the viewfinder will be reloaded
   //shortly when the pipeline is started
   m_viewfinderHasChanged = true;
   emit viewfinderChanged();
}

void CameraBinSession::setStatus(QCamera::Status status)
{
   if (m_status == status) {
      return;
   }

   m_status = status;
   emit statusChanged(m_status);

   setStateHelper(m_pendingState);
}

QCamera::Status CameraBinSession::status() const
{
   return m_status;
}

QCamera::State CameraBinSession::pendingState() const
{
   return m_pendingState;
}

void CameraBinSession::setState(QCamera::State newState)
{
   if (newState == m_pendingState) {
      return;
   }

   m_pendingState = newState;
   emit pendingStateChanged(m_pendingState);

#if CAMERABIN_DEBUG
   qDebug() << Q_FUNC_INFO << newState;
#endif

   setStateHelper(newState);
}

void CameraBinSession::setStateHelper(QCamera::State state)
{
   switch (state) {
      case QCamera::UnloadedState:
         unload();
         break;

      case QCamera::LoadedState:
         if (m_status == QCamera::ActiveStatus) {
            stop();
         } else if (m_status == QCamera::UnloadedStatus) {
            load();
         }
         break;

      case QCamera::ActiveState:
         // If the viewfinder changed while in the loaded state, we need to reload the pipeline
         if (m_status == QCamera::LoadedStatus && !m_viewfinderHasChanged) {
            start();
         } else if (m_status == QCamera::UnloadedStatus || m_viewfinderHasChanged) {
            load();
         }
   }
}

void CameraBinSession::setError(int err, const QString &errorString)
{
   m_pendingState = QCamera::UnloadedState;
   emit error(err, errorString);
   setStatus(QCamera::UnloadedStatus);
}

void CameraBinSession::load()
{
   if (m_status != QCamera::UnloadedStatus && !m_viewfinderHasChanged) {
      return;
   }

   setStatus(QCamera::LoadingStatus);

   gst_element_set_state(m_camerabin, GST_STATE_NULL);

   if (!setupCameraBin()) {
      setError(QCamera::CameraError, "No camera source available");
      return;
   }

   gst_element_set_state(m_camerabin, GST_STATE_READY);
}

void CameraBinSession::unload()
{
   if (m_status == QCamera::UnloadedStatus || m_status == QCamera::UnloadingStatus) {
      return;
   }

   setStatus(QCamera::UnloadingStatus);

   if (m_recordingActive) {
      stopVideoRecording();
   }

   if (m_viewfinderInterface) {
      m_viewfinderInterface->stopRenderer();
   }

   gst_element_set_state(m_camerabin, GST_STATE_NULL);

   if (m_busy) {
      emit busyChanged(m_busy = false);
   }

   m_supportedViewfinderSettings.clear();

   setStatus(QCamera::UnloadedStatus);
}

void CameraBinSession::start()
{
   if (m_status != QCamera::LoadedStatus) {
      return;
   }

   setStatus(QCamera::StartingStatus);

   m_recorderControl->applySettings();

#ifdef HAVE_GST_ENCODING_PROFILES
   GstEncodingContainerProfile *profile = m_recorderControl->videoProfile();
   g_object_set (G_OBJECT(m_camerabin), "video-profile", profile, nullptr);
   gst_encoding_profile_unref(profile);
#endif

   setAudioCaptureCaps();
   setupCaptureResolution();

   gst_element_set_state(m_camerabin, GST_STATE_PLAYING);
}

void CameraBinSession::stop()
{
   if (m_status != QCamera::ActiveStatus) {
      return;
   }

   setStatus(QCamera::StoppingStatus);

   if (m_recordingActive) {
      stopVideoRecording();
   }

   if (m_viewfinderInterface) {
      m_viewfinderInterface->stopRenderer();
   }

   gst_element_set_state(m_camerabin, GST_STATE_READY);
}

bool CameraBinSession::isBusy() const
{
   return m_busy;
}

void CameraBinSession::updateBusyStatus(GObject *o, GParamSpec *p, gpointer d)
{
   (void) p;
   CameraBinSession *session = reinterpret_cast<CameraBinSession *>(d);

   gboolean idle = false;
   g_object_get(o, "idle", &idle, nullptr);
   bool busy = !idle;

   if (session->m_busy != busy) {
      session->m_busy = busy;
      QMetaObject::invokeMethod(session, "busyChanged",
                                Qt::QueuedConnection,
                                Q_ARG(bool, busy));
   }
}

qint64 CameraBinSession::duration() const
{
   if (m_camerabin) {
      GstElement *fileSink = gst_bin_get_by_name(GST_BIN(m_camerabin), FILESINK_BIN_NAME);
      if (fileSink) {
         GstFormat format = GST_FORMAT_TIME;
         gint64 duration = 0;
         bool ret = qt_gst_element_query_position(fileSink, format, &duration);
         gst_object_unref(GST_OBJECT(fileSink));
         if (ret) {
            return duration / 1000000;
         }
      }
   }

   return 0;
}

bool CameraBinSession::isMuted() const
{
   return m_muted;
}

void CameraBinSession::setMuted(bool muted)
{
   if (m_muted != muted) {
      m_muted = muted;

      if (m_camerabin) {
         g_object_set(G_OBJECT(m_camerabin), MUTE_PROPERTY, m_muted, nullptr);
      }
      emit mutedChanged(m_muted);
   }
}

void CameraBinSession::setCaptureDevice(const QString &deviceName)
{
   m_captureDevice = deviceName;
}

void CameraBinSession::setMetaData(const QMap<QByteArray, QVariant> &data)
{
   m_metaData = data;

   if (m_camerabin) {
      QGstUtils::setMetaData(m_camerabin, data);
   }
}

bool CameraBinSession::processSyncMessage(const QGstreamerMessage &message)
{
   GstMessage *gm = message.rawMessage();

   if (gm && GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ELEMENT) {
      const GstStructure *st = gst_message_get_structure(gm);
      const GValue *sampleValue = nullptr;
      if (m_captureMode == QCamera::CaptureStillImage
            && gst_structure_has_name(st, "preview-image")
#if GST_CHECK_VERSION(1,0,0)
            && gst_structure_has_field_typed(st, "sample", GST_TYPE_SAMPLE)
            && (sampleValue = gst_structure_get_value(st, "sample"))) {
         GstSample *const sample = gst_value_get_sample(sampleValue);
         GstCaps *const previewCaps = gst_sample_get_caps(sample);
         GstBuffer *const buffer = gst_sample_get_buffer(sample);
#else
            &&gst_structure_has_field_typed(st, "buffer", GST_TYPE_BUFFER)
            &&(sampleValue = gst_structure_get_value(st, "buffer"))) {
         GstBuffer *const buffer = gst_value_get_buffer(sampleValue);
#endif

         QImage image;
#if GST_CHECK_VERSION(1,0,0)
         GstVideoInfo previewInfo;
         if (gst_video_info_from_caps(&previewInfo, previewCaps)) {
            image = QGstUtils::bufferToImage(buffer, previewInfo);
         }
         gst_sample_unref(sample);
#else
         image = QGstUtils::bufferToImage(buffer);
         gst_buffer_unref(buffer);
#endif
         if (!image.isNull()) {
            static QMetaMethod exposedSignal = QMetaMethod::fromSignal(&CameraBinSession::imageExposed);
            exposedSignal.invoke(this,
                                 Qt::QueuedConnection,
                                 Q_ARG(int, m_requestId));

            static QMetaMethod capturedSignal = QMetaMethod::fromSignal(&CameraBinSession::imageCaptured);
            capturedSignal.invoke(this,
                                  Qt::QueuedConnection,
                                  Q_ARG(int, m_requestId),
                                  Q_ARG(QImage, image));
         }
         return true;
      }
#ifdef HAVE_GST_PHOTOGRAPHY
      if (gst_structure_has_name(st, GST_PHOTOGRAPHY_AUTOFOCUS_DONE)) {
         m_cameraFocusControl->handleFocusMessage(gm);
      }
#endif
   }

   return false;
}

bool CameraBinSession::processBusMessage(const QGstreamerMessage &message)
{
   GstMessage *gm = message.rawMessage();

   if (gm) {
      if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ERROR) {
         GError *err;
         gchar *debug;
         gst_message_parse_error (gm, &err, &debug);

         QString message;

         if (err && err->message) {
            message = QString::fromUtf8(err->message);
            qWarning() << "CameraBin error:" << message;
         }

         //only report error messager from camerabin
         if (GST_MESSAGE_SRC(gm) == GST_OBJECT_CAST(m_camerabin)) {
            if (message.isEmpty()) {
               message = tr("Camera error");
            }

            setError(int(QMediaRecorder::ResourceError), message);
         }

#ifdef CAMERABIN_DEBUG_DUMP_BIN
         _gst_debug_bin_to_dot_file_with_ts(GST_BIN(m_camerabin),
                                            GstDebugGraphDetails(GST_DEBUG_GRAPH_SHOW_ALL /* GST_DEBUG_GRAPH_SHOW_MEDIA_TYPE | GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS | GST_DEBUG_GRAPH_SHOW_STATES*/),
                                            "camerabin_error");
#endif


         if (err) {
            g_error_free (err);
         }

         if (debug) {
            g_free (debug);
         }
      }

      if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_WARNING) {
         GError *err;
         gchar *debug;
         gst_message_parse_warning (gm, &err, &debug);

         if (err && err->message) {
            qWarning() << "CameraBin warning:" << QString::fromUtf8(err->message);
         }

         if (err) {
            g_error_free (err);
         }
         if (debug) {
            g_free (debug);
         }
      }

      if (GST_MESSAGE_SRC(gm) == GST_OBJECT_CAST(m_camerabin)) {
         switch (GST_MESSAGE_TYPE(gm))  {
            case GST_MESSAGE_DURATION:
               break;

            case GST_MESSAGE_STATE_CHANGED: {

               GstState    oldState;
               GstState    newState;
               GstState    pending;

               gst_message_parse_state_changed(gm, &oldState, &newState, &pending);


#if CAMERABIN_DEBUG
               QStringList states;
               states << "GST_STATE_VOID_PENDING" <<  "GST_STATE_NULL" << "GST_STATE_READY" << "GST_STATE_PAUSED" << "GST_STATE_PLAYING";


               qDebug() << QString("state changed: old: %1  new: %2  pending: %3") \
                        .formatArg(states[oldState]) \
                        .formatArg(states[newState]) \
                        .formatArg(states[pending]);
#endif

#ifdef CAMERABIN_DEBUG_DUMP_BIN
               _gst_debug_bin_to_dot_file_with_ts(GST_BIN(m_camerabin),
                                                  GstDebugGraphDetails(GST_DEBUG_GRAPH_SHOW_ALL /*GST_DEBUG_GRAPH_SHOW_MEDIA_TYPE | GST_DEBUG_GRAPH_SHOW_NON_DEFAULT_PARAMS | GST_DEBUG_GRAPH_SHOW_STATES*/),
                                                  "camerabin");
#endif

               switch (newState) {
                  case GST_STATE_VOID_PENDING:
                  case GST_STATE_NULL:
                     setStatus(QCamera::UnloadedStatus);
                     break;

                  case GST_STATE_READY:
                     if (oldState == GST_STATE_NULL) {
                        updateSupportedViewfinderSettings();
                     }

                     setMetaData(m_metaData);
                     setStatus(QCamera::LoadedStatus);
                     break;

                  case GST_STATE_PLAYING:
                     setStatus(QCamera::ActiveStatus);
                     break;

                  case GST_STATE_PAUSED:
                  default:
                     break;
               }
            }
            break;
            default:
               break;
         }
      }
   }

   return false;
}

QString CameraBinSession::currentContainerFormat() const
{
   if (!m_muxer) {
      return QString();
   }

   QString format;

   if (GstPad *srcPad = gst_element_get_static_pad(m_muxer, "src")) {
      if (GstCaps *caps = qt_gst_pad_get_caps(srcPad)) {
         gchar *capsString = gst_caps_to_string(caps);
         format = QString::fromLatin1(capsString);
         if (capsString) {
            g_free(capsString);
         }
         gst_caps_unref(caps);
      }
      gst_object_unref(GST_OBJECT(srcPad));
   }

   return format;
}

void CameraBinSession::recordVideo()
{
   QString format = currentContainerFormat();
   if (format.isEmpty()) {
      format = m_mediaContainerControl->actualContainerFormat();
   }

   const QString actualFileName = m_mediaStorageLocation.generateFileName(m_sink.isLocalFile() ? m_sink.toLocalFile()
                                  : m_sink.toString(),
                                  QMediaStorageLocation::Movies,
                                  QLatin1String("clip_"),
                                  m_mediaContainerControl->suggestedFileExtension(format));

   m_recordingActive = true;
   m_actualSink = QUrl::fromLocalFile(actualFileName);

   g_object_set(G_OBJECT(m_camerabin), FILENAME_PROPERTY, QFile::encodeName(actualFileName).constData(), nullptr);

   g_signal_emit_by_name(G_OBJECT(m_camerabin), CAPTURE_START, nullptr);
}

void CameraBinSession::stopVideoRecording()
{
   m_recordingActive = false;
   g_signal_emit_by_name(G_OBJECT(m_camerabin), CAPTURE_STOP, nullptr);
}

//internal, only used by CameraBinSession::supportedFrameRates.
//recursively fills the list of framerates res from value data.
static void readValue(const GValue *value, QList< QPair<int, int> > *res, bool *continuous)
{
   if (GST_VALUE_HOLDS_FRACTION(value)) {
      int num = gst_value_get_fraction_numerator(value);
      int denum = gst_value_get_fraction_denominator(value);

      *res << QPair<int, int>(num, denum);
   } else if (GST_VALUE_HOLDS_FRACTION_RANGE(value)) {
      const GValue *rateValueMin = gst_value_get_fraction_range_min(value);
      const GValue *rateValueMax = gst_value_get_fraction_range_max(value);

      if (continuous) {
         *continuous = true;
      }

      readValue(rateValueMin, res, continuous);
      readValue(rateValueMax, res, continuous);
   } else if (GST_VALUE_HOLDS_LIST(value)) {
      for (uint i = 0; i < gst_value_list_get_size(value); i++) {
         readValue(gst_value_list_get_value(value, i), res, continuous);
      }
   }
}

static bool rateLessThan(const QPair<int, int> &r1, const QPair<int, int> &r2)
{
   return r1.first * r2.second < r2.first * r1.second;
}

GstCaps *CameraBinSession::supportedCaps(QCamera::CaptureModes mode) const
{
   GstCaps *supportedCaps = nullptr;

   // When using wrappercamerabinsrc, get the supported caps directly from the video source element.
   // This makes sure we only get the caps actually supported by the video source element.
   if (m_videoSrc) {
      GstPad *pad = gst_element_get_static_pad(m_videoSrc, "src");
      if (pad) {
         supportedCaps = qt_gst_pad_get_caps(pad);
         gst_object_unref(GST_OBJECT(pad));
      }
   }

   // Otherwise, let the camerabin handle this.
   if (!supportedCaps) {
      const gchar *prop;
      switch (mode) {
         case QCamera::CaptureStillImage:
            prop = SUPPORTED_IMAGE_CAPTURE_CAPS_PROPERTY;
            break;
         case QCamera::CaptureVideo:
            prop = SUPPORTED_VIDEO_CAPTURE_CAPS_PROPERTY;
            break;
         case QCamera::CaptureViewfinder:
         default:
            prop = SUPPORTED_VIEWFINDER_CAPS_PROPERTY;
            break;
      }

      g_object_get(G_OBJECT(m_camerabin), prop, &supportedCaps, nullptr);
   }

   return supportedCaps;
}

QList< QPair<int, int> > CameraBinSession::supportedFrameRates(const QSize &frameSize, bool *continuous) const
{
   QList< QPair<int, int> > res;

   GstCaps *supportedCaps = this->supportedCaps(QCamera::CaptureVideo);

   if (!supportedCaps) {
      return res;
   }

   GstCaps *caps = nullptr;

   if (frameSize.isEmpty()) {
      caps = gst_caps_copy(supportedCaps);
   } else {
      GstCaps *filter = QGstUtils::videoFilterCaps();
      gst_caps_set_simple(
         filter,
         "width", G_TYPE_INT, frameSize.width(),
         "height", G_TYPE_INT, frameSize.height(),
         nullptr);

      caps = gst_caps_intersect(supportedCaps, filter);
      gst_caps_unref(filter);
   }
   gst_caps_unref(supportedCaps);

   //simplify to the list of rates only:
   caps = gst_caps_make_writable(caps);
   for (uint i = 0; i < gst_caps_get_size(caps); i++) {
      GstStructure *structure = gst_caps_get_structure(caps, i);
      gst_structure_set_name(structure, "video/x-raw");
      const GValue *oldRate = gst_structure_get_value(structure, "framerate");
      GValue rate;
      memset(&rate, 0, sizeof(rate));
      g_value_init(&rate, G_VALUE_TYPE(oldRate));
      g_value_copy(oldRate, &rate);
      gst_structure_remove_all_fields(structure);
      gst_structure_set_value(structure, "framerate", &rate);
   }
#if GST_CHECK_VERSION(1,0,0)
   caps = gst_caps_simplify(caps);
#else
   gst_caps_do_simplify(caps);
#endif

   for (uint i = 0; i < gst_caps_get_size(caps); i++) {
      GstStructure *structure = gst_caps_get_structure(caps, i);
      const GValue *rateValue = gst_structure_get_value(structure, "framerate");
      readValue(rateValue, &res, continuous);
   }

   std::sort(res.begin(), res.end(), rateLessThan);

#if CAMERABIN_DEBUG
   qDebug() << "Supported rates:" << caps;
   qDebug() << res;
#endif

   gst_caps_unref(caps);

   return res;
}

//internal, only used by CameraBinSession::supportedResolutions
//recursively find the supported resolutions range.
static QPair<int, int> valueRange(const GValue *value, bool *continuous)
{
   int minValue = 0;
   int maxValue = 0;

   if (g_value_type_compatible(G_VALUE_TYPE(value), G_TYPE_INT)) {
      minValue = maxValue = g_value_get_int(value);
   } else if (GST_VALUE_HOLDS_INT_RANGE(value)) {
      minValue = gst_value_get_int_range_min(value);
      maxValue = gst_value_get_int_range_max(value);
      *continuous = true;

   } else if (GST_VALUE_HOLDS_LIST(value)) {
      for (uint i = 0; i < gst_value_list_get_size(value); i++) {
         QPair<int, int> res = valueRange(gst_value_list_get_value(value, i), continuous);

         if (res.first > 0 && minValue > 0) {
            minValue = qMin(minValue, res.first);
         } else { //select non 0 valid value
            minValue = qMax(minValue, res.first);
         }

         maxValue = qMax(maxValue, res.second);
      }
   }

   return QPair<int, int>(minValue, maxValue);
}

static bool resolutionLessThan(const QSize &r1, const QSize &r2)
{
   return qint64(r1.width()) * r1.height() < qint64(r2.width()) * r2.height();
}


QList<QSize> CameraBinSession::supportedResolutions(QPair<int, int> rate,
      bool *continuous, QCamera::CaptureModes mode) const
{
   QList<QSize> res;

   if (continuous) {
      *continuous = false;
   }

   GstCaps *supportedCaps = this->supportedCaps(mode);

#if CAMERABIN_DEBUG
   qDebug() << "Source caps:" << supportedCaps;
#endif

   if (! supportedCaps) {
      return res;
   }

   GstCaps *caps = nullptr;
   bool isContinuous = false;

   if (rate.first <= 0 || rate.second <= 0) {
      caps = gst_caps_copy(supportedCaps);
   } else {
      GstCaps *filter = QGstUtils::videoFilterCaps();
      gst_caps_set_simple(
         filter,
         "framerate", GST_TYPE_FRACTION, rate.first, rate.second,
         nullptr);
      caps = gst_caps_intersect(supportedCaps, filter);
      gst_caps_unref(filter);
   }
   gst_caps_unref(supportedCaps);

   //simplify to the list of resolutions only:
   caps = gst_caps_make_writable(caps);
   for (uint i = 0; i < gst_caps_get_size(caps); i++) {
      GstStructure *structure = gst_caps_get_structure(caps, i);
      gst_structure_set_name(structure, "video/x-raw");
      const GValue *oldW = gst_structure_get_value(structure, "width");
      const GValue *oldH = gst_structure_get_value(structure, "height");
      GValue w;
      memset(&w, 0, sizeof(GValue));
      GValue h;
      memset(&h, 0, sizeof(GValue));
      g_value_init(&w, G_VALUE_TYPE(oldW));
      g_value_init(&h, G_VALUE_TYPE(oldH));
      g_value_copy(oldW, &w);
      g_value_copy(oldH, &h);
      gst_structure_remove_all_fields(structure);
      gst_structure_set_value(structure, "width", &w);
      gst_structure_set_value(structure, "height", &h);
   }

#if GST_CHECK_VERSION(1,0,0)
   caps = gst_caps_simplify(caps);
#else
   gst_caps_do_simplify(caps);
#endif

   for (uint i = 0; i < gst_caps_get_size(caps); i++) {
      GstStructure *structure = gst_caps_get_structure(caps, i);
      const GValue *wValue = gst_structure_get_value(structure, "width");
      const GValue *hValue = gst_structure_get_value(structure, "height");

      QPair<int, int> wRange = valueRange(wValue, &isContinuous);
      QPair<int, int> hRange = valueRange(hValue, &isContinuous);

      QSize minSize(wRange.first, hRange.first);
      QSize maxSize(wRange.second, hRange.second);

      if (!minSize.isEmpty()) {
         res << minSize;
      }

      if (minSize != maxSize && !maxSize.isEmpty()) {
         res << maxSize;
      }
   }


   std::sort(res.begin(), res.end(), resolutionLessThan);

   //if the range is continuos, populate is with the common rates
   if (isContinuous && res.size() >= 2) {
      //fill the ragne with common value
      static QList<QSize> commonSizes =
         QList<QSize>() << QSize(128, 96)
         << QSize(160, 120)
         << QSize(176, 144)
         << QSize(320, 240)
         << QSize(352, 288)
         << QSize(640, 480)
         << QSize(848, 480)
         << QSize(854, 480)
         << QSize(1024, 768)
         << QSize(1280, 720) // HD 720
         << QSize(1280, 1024)
         << QSize(1600, 1200)
         << QSize(1920, 1080) // HD
         << QSize(1920, 1200)
         << QSize(2048, 1536)
         << QSize(2560, 1600)
         << QSize(2580, 1936);

      QSize minSize = res.first();
      QSize maxSize = res.last();
      res.clear();

      for (const QSize &candidate : commonSizes) {
         int w = candidate.width();
         int h = candidate.height();

         if (w > maxSize.width() && h > maxSize.height()) {
            break;
         }

         if (w >= minSize.width() && h >= minSize.height() &&
               w <= maxSize.width() && h <= maxSize.height()) {
            res << candidate;
         }
      }

      if (res.isEmpty() || res.first() != minSize) {
         res.prepend(minSize);
      }

      if (res.last() != maxSize) {
         res.append(maxSize);
      }
   }

#if CAMERABIN_DEBUG
   qDebug() << "Supported resolutions:" << gst_caps_to_string(caps);
   qDebug() << res;
#endif

   gst_caps_unref(caps);

   if (continuous) {
      *continuous = isContinuous;
   }

   return res;
}

void CameraBinSession::updateSupportedViewfinderSettings()
{
   m_supportedViewfinderSettings.clear();

   GstCaps *supportedCaps = this->supportedCaps(QCamera::CaptureViewfinder);

   // Convert caps to QCameraViewfinderSettings
   if (supportedCaps) {
      supportedCaps = qt_gst_caps_normalize(supportedCaps);

      for (uint i = 0; i < gst_caps_get_size(supportedCaps); i++) {
         const GstStructure *structure = gst_caps_get_structure(supportedCaps, i);

         QCameraViewfinderSettings s;
         s.setResolution(QGstUtils::structureResolution(structure));
         s.setPixelFormat(QGstUtils::structurePixelFormat(structure));
         s.setPixelAspectRatio(QGstUtils::structurePixelAspectRatio(structure));

         QPair<qreal, qreal> frameRateRange = QGstUtils::structureFrameRateRange(structure);
         s.setMinimumFrameRate(frameRateRange.first);
         s.setMaximumFrameRate(frameRateRange.second);

         if (!s.resolution().isEmpty()
               && s.pixelFormat() != QVideoFrame::Format_Invalid
               && !m_supportedViewfinderSettings.contains(s)) {

            m_supportedViewfinderSettings.append(s);
         }
      }

      gst_caps_unref(supportedCaps);
   }
}

void CameraBinSession::elementAdded(GstBin *, GstElement *element, CameraBinSession *session)
{
   GstElementFactory *factory = gst_element_get_factory(element);

   if (GST_IS_BIN(element)) {
      g_signal_connect(G_OBJECT(element), "element-added",  G_CALLBACK(elementAdded), session);
      g_signal_connect(G_OBJECT(element), "element-removed",  G_CALLBACK(elementRemoved), session);
   } else if (!factory) {
      // no-op
#if GST_CHECK_VERSION(0,10,31)
   } else if (gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_AUDIO_ENCODER)) {
#else
   } else if (strstr(gst_element_factory_get_klass(factory), "Encoder/Audio") != nullptr) {
#endif
      session->m_audioEncoder = element;

      session->m_audioEncodeControl->applySettings(element);
#if GST_CHECK_VERSION(0,10,31)
   } else if (gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_VIDEO_ENCODER)) {
#else
   } else if (strstr(gst_element_factory_get_klass(factory), "Encoder/Video") != nullptr) {
#endif
      session->m_videoEncoder = element;

      session->m_videoEncodeControl->applySettings(element);
#if GST_CHECK_VERSION(0,10,31)
   } else if (gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_MUXER)) {
#else
   } else if (strstr(gst_element_factory_get_klass(factory), "Muxer") != nullptr) {
#endif
      session->m_muxer = element;
   }
}

void CameraBinSession::elementRemoved(GstBin *, GstElement *element, CameraBinSession *session)
{
   if (element == session->m_audioEncoder) {
      session->m_audioEncoder = nullptr;
   } else if (element == session->m_videoEncoder) {
      session->m_videoEncoder = nullptr;
   } else if (element == session->m_muxer) {
      session->m_muxer = nullptr;
   }
}

