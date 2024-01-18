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

#ifndef QGSTUTILS_P_H
#define QGSTUTILS_P_H

#include <qabstractvideobuffer.h>
#include <qaudioformat.h>
#include <qcamera.h>
#include <qdebug.h>
#include <qmap.h>
#include <qset.h>
#include <qvector.h>
#include <qvideoframe.h>

#include <gst/gst.h>
#include <gst/video/video.h>

#if GST_CHECK_VERSION(1,0,0)
# define QT_GSTREAMER_PLAYBIN_ELEMENT_NAME "playbin"
# define QT_GSTREAMER_CAMERABIN_ELEMENT_NAME "camerabin"
# define QT_GSTREAMER_COLORCONVERSION_ELEMENT_NAME "videoconvert"
# define QT_GSTREAMER_RAW_AUDIO_MIME "audio/x-raw"
# define QT_GSTREAMER_VIDEOOVERLAY_INTERFACE_NAME "GstVideoOverlay"
#else
# define QT_GSTREAMER_PLAYBIN_ELEMENT_NAME "playbin2"
# define QT_GSTREAMER_CAMERABIN_ELEMENT_NAME "camerabin2"
# define QT_GSTREAMER_COLORCONVERSION_ELEMENT_NAME "ffmpegcolorspace"
# define QT_GSTREAMER_RAW_AUDIO_MIME "audio/x-raw-int"
# define QT_GSTREAMER_VIDEOOVERLAY_INTERFACE_NAME "GstXOverlay"
#endif

class QSize;
class QVariant;
class QByteArray;
class QImage;
class QVideoSurfaceFormat;

namespace QGstUtils {

struct CameraInfo
{
  QString name;
  QString description;
  int orientation;
  QCamera::Position position;
  QByteArray driver;
};

QMap<QByteArray, QVariant> gstTagListToMap(const GstTagList *list);

QSize capsResolution(const GstCaps *caps);
QSize capsCorrectedResolution(const GstCaps *caps);
QAudioFormat audioFormatForCaps(const GstCaps *caps);

#if GST_CHECK_VERSION(1,0,0)
   QAudioFormat audioFormatForSample(GstSample *sample);
#else
   QAudioFormat audioFormatForBuffer(GstBuffer *buffer);
#endif

GstCaps *capsForAudioFormat(const QAudioFormat &format);
void initializeGst();

QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList &codecs,
                  const QSet<QString> &supportedMimeTypeSet);

QVector<CameraInfo> enumerateCameras(GstElementFactory *factory = nullptr);
QList<QString> cameraDevices(GstElementFactory *factory = nullptr);
QString cameraDescription(const QString &device, GstElementFactory *factory = nullptr);
QCamera::Position cameraPosition(const QString &device, GstElementFactory *factory = nullptr);
int cameraOrientation(const QString &device, GstElementFactory *factory = nullptr);
QByteArray cameraDriver(const QString &device, GstElementFactory *factory = nullptr);

QSet<QString> supportedMimeTypes(bool (*isValidFactory)(GstElementFactory *factory));

#if GST_CHECK_VERSION(1,0,0)

QImage bufferToImage(GstBuffer *buffer, const GstVideoInfo &info);
QVideoSurfaceFormat formatForCaps(GstCaps *caps, GstVideoInfo *info = nullptr,
   QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle);

#else

QImage bufferToImage(GstBuffer *buffer);
QVideoSurfaceFormat formatForCaps(GstCaps *caps, int *bytesPerLine = nullptr,
   QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle);

#endif

GstCaps *capsForFormats(const QList<QVideoFrame::PixelFormat> &formats);
void setFrameTimeStamps(QVideoFrame *frame, GstBuffer *buffer);

void setMetaData(GstElement *element, const QMap<QByteArray, QVariant> &data);
void setMetaData(GstBin *bin, const QMap<QByteArray, QVariant> &data);

GstCaps *videoFilterCaps();

QSize structureResolution(const GstStructure *s);
QVideoFrame::PixelFormat structurePixelFormat(const GstStructure *s, int *bpp = nullptr);
QSize structurePixelAspectRatio(const GstStructure *s);
QPair<qreal, qreal> structureFrameRateRange(const GstStructure *s);

}

void qt_gst_object_ref_sink(gpointer object);
GstCaps *qt_gst_pad_get_current_caps(GstPad *pad);
GstCaps *qt_gst_pad_get_caps(GstPad *pad);
GstStructure *qt_gst_structure_new_empty(const char *name);
gboolean qt_gst_element_query_position(GstElement *element, GstFormat format, gint64 *cur);
gboolean qt_gst_element_query_duration(GstElement *element, GstFormat format, gint64 *cur);
GstCaps *qt_gst_caps_normalize(GstCaps *caps);
const gchar *qt_gst_element_get_factory_name(GstElement *element);
gboolean qt_gst_caps_can_intersect(const GstCaps *caps1, const GstCaps *caps2);
GList *qt_gst_video_sinks();
void qt_gst_util_double_to_fraction(gdouble src, gint *dest_n, gint *dest_d);

QDebug operator <<(QDebug debug, GstCaps *caps);

#endif
