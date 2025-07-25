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

#include <qgstreamerbufferprobe_p.h>
#include <qgstutils_p.h>

QGstreamerBufferProbe::QGstreamerBufferProbe(Flags flags)
   : m_capsProbeId(-1), m_bufferProbeId(-1), m_flags(flags)
{
}

QGstreamerBufferProbe::~QGstreamerBufferProbe()
{
}

void QGstreamerBufferProbe::addProbeToPad(GstPad *pad, bool downstream)
{
   if (GstCaps *caps = qt_gst_pad_get_current_caps(pad)) {
      probeCaps(caps);
      gst_caps_unref(caps);
   }

   if (m_flags & ProbeCaps) {
      m_capsProbeId = gst_pad_add_probe(
            pad,
            downstream
            ? GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM
            : GST_PAD_PROBE_TYPE_EVENT_UPSTREAM,
            capsProbe,
            this,
            nullptr);
   }

   if (m_flags & ProbeBuffers) {
      m_bufferProbeId = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, bufferProbe, this, nullptr);
   }
}

void QGstreamerBufferProbe::removeProbeFromPad(GstPad *pad)
{
   if (m_capsProbeId != -1) {
      gst_pad_remove_probe(pad, m_capsProbeId);
      m_capsProbeId = -1;
   }

   if (m_bufferProbeId != -1) {
      gst_pad_remove_probe(pad, m_bufferProbeId);
      m_bufferProbeId = -1;
   }
}

void QGstreamerBufferProbe::probeCaps(GstCaps *)
{
}

bool QGstreamerBufferProbe::probeBuffer(GstBuffer *)
{
   return true;
}

GstPadProbeReturn QGstreamerBufferProbe::capsProbe(GstPad *, GstPadProbeInfo *info, gpointer user_data)
{
   QGstreamerBufferProbe *const control = static_cast<QGstreamerBufferProbe *>(user_data);

   if (GstEvent *const event = gst_pad_probe_info_get_event(info)) {
      if (GST_EVENT_TYPE(event) == GST_EVENT_CAPS) {
         GstCaps *caps;
         gst_event_parse_caps(event, &caps);

         control->probeCaps(caps);
      }
   }

   return GST_PAD_PROBE_OK;
}

GstPadProbeReturn QGstreamerBufferProbe::bufferProbe(GstPad *, GstPadProbeInfo *info, gpointer user_data)
{
   QGstreamerBufferProbe *const control = static_cast<QGstreamerBufferProbe *>(user_data);

   if (GstBuffer *const buffer = gst_pad_probe_info_get_buffer(info)) {
      return control->probeBuffer(buffer) ? GST_PAD_PROBE_OK : GST_PAD_PROBE_DROP;
   }

   return GST_PAD_PROBE_OK;
}
