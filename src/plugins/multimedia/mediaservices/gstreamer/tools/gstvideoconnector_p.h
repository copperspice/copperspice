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

#ifndef QGSTVIDEOCONNECTOR_P_H
#define QGSTVIDEOCONNECTOR_P_H

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_VIDEO_CONNECTOR \
  (gst_video_connector_get_type())
#define GST_VIDEO_CONNECTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_VIDEO_CONNECTOR, GstVideoConnector))
#define GST_VIDEO_CONNECTOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_VIDEO_CONNECTOR, GstVideoConnectorClass))
#define GST_IS_VIDEO_CONNECTOR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_VIDEO_CONNECTOR))
#define GST_IS_VIDEO_CONNECTOR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_VIDEO_CONNECTOR))

typedef struct _GstVideoConnector GstVideoConnector;
typedef struct _GstVideoConnectorClass GstVideoConnectorClass;

struct _GstVideoConnector {
  GstElement element;

  GstPad *srcpad;
  GstPad *sinkpad;

  gboolean relinked;
  gboolean failedSignalEmited;
  GstSegment segment;
  GstBuffer *latest_buffer;
};

struct _GstVideoConnectorClass {
  GstElementClass parent_class;

  /* action signal to resend new segment */
  void (*resend_new_segment) (GstElement * element, gboolean emitFailedSignal);
};

GType gst_video_connector_get_type (void);

G_END_DECLS

#endif

