/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <camera_imagecapture.h>

#include <camera_capturebufferformat.h>
#include <camera_capturedestination.h>
#include <camera_control.h>
#include <camera_resourcepolicy.h>
#include <camera_session.h>
#include <qbuffer.h>
#include <qdebug.h>
#include <qimagereader.h>
#include <qmediametadata.h>

#include <qgstutils_p.h>
#include <qgstvideobuffer_p.h>
#include <qgstvideorenderersink_p.h>

#include <gst/gst.h>

using QVideoSurfaceGstSink = QGstVideoRendererSink;

#define IMAGE_DONE_SIGNAL "image-done"

CameraBinImageCapture::CameraBinImageCapture(CameraBinSession *session)
   : QCameraImageCaptureControl(session), m_encoderProbe(this), m_muxerProbe(this), m_session(session),
     m_jpegEncoderElement(nullptr), m_metadataMuxerElement(nullptr), m_requestId(0), m_ready(false)
{
   connect(m_session, &CameraBinSession::statusChanged, this, &CameraBinImageCapture::updateState);
   connect(m_session, &CameraBinSession::imageExposed,  this, &CameraBinImageCapture::imageExposed);
   connect(m_session, &CameraBinSession::imageCaptured, this, &CameraBinImageCapture::imageCaptured);

   connect(m_session->cameraControl()->resourcePolicy(), &CamerabinResourcePolicy::canCaptureChanged,
                  this, &CameraBinImageCapture::updateState);

   m_session->bus()->installMessageFilter(this);
}

CameraBinImageCapture::~CameraBinImageCapture()
{
}

bool CameraBinImageCapture::isReadyForCapture() const
{
   return m_ready;
}

int CameraBinImageCapture::capture(const QString &fileName)
{
   ++m_requestId;

   if (! m_ready) {
      emit error(m_requestId, QCameraImageCapture::NotReadyError, tr("Camera not ready"));
      return m_requestId;
   }

   m_session->captureImage(m_requestId, fileName);

   return m_requestId;
}

void CameraBinImageCapture::cancelCapture()
{
}

void CameraBinImageCapture::updateState()
{
   bool ready = m_session->status() == QCamera::ActiveStatus
                && m_session->cameraControl()->resourcePolicy()->canCapture();

   if (m_ready != ready) {
      emit readyForCaptureChanged(m_ready = ready);
   }
}

GstPadProbeReturn CameraBinImageCapture::encoderEventProbe(GstPad *, GstPadProbeInfo *info, gpointer user_data)
{
   GstEvent *const event = gst_pad_probe_info_get_event(info);

   CameraBinImageCapture  *const self = static_cast<CameraBinImageCapture *>(user_data);

   if (event && GST_EVENT_TYPE(event) == GST_EVENT_TAG) {
      GstTagList *gstTags;
      gst_event_parse_tag(event, &gstTags);
      QMap<QByteArray, QVariant> extendedTags = QGstUtils::gstTagListToMap(gstTags);

      QVariantMap tags;
      tags[QMediaMetaData::ISOSpeedRatings]  = extendedTags.value("capturing-iso-speed");
      tags[QMediaMetaData::DigitalZoomRatio] = extendedTags.value("capturing-digital-zoom-ratio");
      tags[QMediaMetaData::ExposureTime]     = extendedTags.value("capturing-shutter-speed");
      tags[QMediaMetaData::WhiteBalance]     = extendedTags.value("capturing-white-balance");
      tags[QMediaMetaData::Flash]            = extendedTags.value("capturing-flash-fired");
      tags[QMediaMetaData::FocalLengthIn35mmFilm] = extendedTags.value("capturing-focal-length");
      tags[QMediaMetaData::MeteringMode]     = extendedTags.value("capturing-metering-mode");
      tags[QMediaMetaData::ExposureMode]     = extendedTags.value("capturing-exposure-mode");
      tags[QMediaMetaData::FNumber]          = extendedTags.value("capturing-focal-ratio");
      tags[QMediaMetaData::ExposureMode]     = extendedTags.value("capturing-exposure-mode");

      QMapIterator<QString, QVariant> i(tags);

      while (i.hasNext()) {
         i.next();

         if (i.value().isValid()) {
            QMetaObject::invokeMethod(self, "imageMetadataAvailable", Qt::QueuedConnection,
                  Q_ARG(int, self->m_requestId), Q_ARG(QString, i.key()), Q_ARG(QVariant, i.value()));
         }
      }
   }

   return GST_PAD_PROBE_OK;
}

void CameraBinImageCapture::EncoderProbe::probeCaps(GstCaps *caps)
{
   m_capture->m_bufferFormat = QGstUtils::formatForCaps(caps, &m_capture->m_videoInfo);
}

bool CameraBinImageCapture::EncoderProbe::probeBuffer(GstBuffer *buffer)
{
   CameraBinSession *const session = m_capture->m_session;

   QCameraImageCapture::CaptureDestinations destination =
      session->captureDestinationControl()->captureDestination();

   QVideoFrame::PixelFormat format = session->captureBufferFormatControl()->bufferFormat();

   if (destination & QCameraImageCapture::CaptureToBuffer) {
      if (format != QVideoFrame::Format_Jpeg) {
         QGstVideoBuffer *videoBuffer = new QGstVideoBuffer(buffer, m_capture->m_videoInfo);

         QVideoFrame frame(videoBuffer, m_capture->m_bufferFormat.frameSize(), m_capture->m_bufferFormat.pixelFormat());

         QMetaObject::invokeMethod(m_capture, "imageAvailable", Qt::QueuedConnection,
                  Q_ARG(int, m_capture->m_requestId), Q_ARG(QVideoFrame, frame));
      }
   }

   //keep the buffer if capture to file or jpeg buffer capture was reuqsted
   bool keepBuffer = (destination & QCameraImageCapture::CaptureToFile) ||
                     ((destination & QCameraImageCapture::CaptureToBuffer) &&
                      format == QVideoFrame::Format_Jpeg);

   return keepBuffer;
}

void CameraBinImageCapture::MuxerProbe::probeCaps(GstCaps *caps)
{
   m_capture->m_jpegResolution = QGstUtils::capsCorrectedResolution(caps);
}

bool CameraBinImageCapture::MuxerProbe::probeBuffer(GstBuffer *buffer)
{
   CameraBinSession *const session = m_capture->m_session;

   QCameraImageCapture::CaptureDestinations destination =
      session->captureDestinationControl()->captureDestination();

   if ((destination & QCameraImageCapture::CaptureToBuffer) &&
         session->captureBufferFormatControl()->bufferFormat() == QVideoFrame::Format_Jpeg) {

      QSize resolution = m_capture->m_jpegResolution;

      // if resolution is not presented in caps, try to find it from encoded jpeg data:
      GstMapInfo mapInfo;

      if (resolution.isEmpty() && gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
         QBuffer data;
         data.setData(reinterpret_cast<const char *>(mapInfo.data), mapInfo.size);

         QImageReader reader(&data, "JPEG");
         resolution = reader.size();

         gst_buffer_unmap(buffer, &mapInfo);
      }

      GstVideoInfo info;
      gst_video_info_set_format(&info, GST_VIDEO_FORMAT_ENCODED, resolution.width(), resolution.height());
      QGstVideoBuffer *videoBuffer = new QGstVideoBuffer(buffer, info);

      QVideoFrame frame(videoBuffer, resolution, QVideoFrame::Format_Jpeg);
      QMetaObject::invokeMethod(m_capture, "imageAvailable", Qt::QueuedConnection,
                  Q_ARG(int, m_capture->m_requestId), Q_ARG(QVideoFrame, frame));
   }

   // Theoretically we could drop the buffer here when don't want to capture to file but that
   // prevents camerabin from recognizing that capture has been completed and returning to its idle state.

   return true;
}

bool CameraBinImageCapture::processBusMessage(const QGstreamerMessage &message)
{
   // install metadata event and buffer probes

   // image capture pipeline is built dynamically,
   // it is necessary to wait until jpeg encoder is added

   GstMessage *gm = message.rawMessage();

   if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_STATE_CHANGED) {
      GstState    oldState;
      GstState    newState;
      GstState    pending;
      gst_message_parse_state_changed(gm, &oldState, &newState, &pending);

      if (newState == GST_STATE_READY) {
         GstElement *element = GST_ELEMENT(GST_MESSAGE_SRC(gm));
         if (! element) {
            return false;
         }

         QString elementName = QString::fromLatin1(gst_element_get_name(element));

         if (elementName.contains("jpegenc") && element != m_jpegEncoderElement) {
            m_jpegEncoderElement = element;
            GstPad *sinkpad = gst_element_get_static_pad(element, "sink");

            //metadata event probe is installed before jpeg encoder
            //to emit metadata available signal as soon as possible.

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
            qDebug("CameraBinImageCapture::processBusMessage() Install metadata probe");
#endif

            gst_pad_add_probe(sinkpad, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, encoderEventProbe, this, nullptr);

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
            qDebug("CameraBinImageCapture::processBusMessage() Install uncompressed buffer probe");
#endif
            m_encoderProbe.addProbeToPad(sinkpad, true);

            gst_object_unref(sinkpad);

         } else if ((elementName.contains("jifmux")
               || elementName.startsWith("metadatamux")) && element != m_metadataMuxerElement) {

            //Jpeg encoded buffer probe is added after jifmux/metadatamux
            //element to ensure the resulting jpeg buffer contains capture metadata

            m_metadataMuxerElement = element;

            GstPad *srcpad = gst_element_get_static_pad(element, "src");

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
            qDebug("CameraBinImageCapture::processBusMessage() Install jpeg buffer probe");
#endif

            m_muxerProbe.addProbeToPad(srcpad);

            gst_object_unref(srcpad);
         }
      }

   } else if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ELEMENT) {
      if (GST_MESSAGE_SRC(gm) == (GstObject *)m_session->cameraBin()) {
         const GstStructure *structure = gst_message_get_structure(gm);

         if (gst_structure_has_name (structure, "image-done")) {
            const gchar *fileName = gst_structure_get_string (structure, "filename");

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
            qDebug() << "CameraBinImageCapture::processBusMessage() Image saved" << fileName;
#endif

            if (m_session->captureDestinationControl()->captureDestination() & QCameraImageCapture::CaptureToFile) {
               emit imageSaved(m_requestId, QString::fromUtf8(fileName));

            } else {

#if defined(CS_SHOW_DEBUG_PLUGINS_GSTREAMER)
               qDebug() << "CameraBinImageCapture::processBusMessage() Dropped saving file" << fileName;
#endif
               // camerabin creates an empty file when captured buffer is dropped, remove it
               QFileInfo info(QString::fromUtf8(fileName));

               if (info.exists() && info.isFile() && info.size() == 0) {
                  QFile(info.absoluteFilePath()).remove();
               }
            }
         }
      }
   }

   return false;
}
