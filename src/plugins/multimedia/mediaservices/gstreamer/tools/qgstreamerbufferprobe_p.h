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

#ifndef QGSTREAMERBUFFERPROBE_H
#define QGSTREAMERBUFFERPROBE_H

#include <qglobal.h>

#include <gst/gst.h>

class QGstreamerBufferProbe
{
 public:
   enum Flags {
      ProbeCaps     = 0x01,
      ProbeBuffers  = 0x02,
      ProbeAll      = ProbeCaps | ProbeBuffers
   };

   explicit QGstreamerBufferProbe(Flags flags = ProbeAll);
   virtual ~QGstreamerBufferProbe();

   void addProbeToPad(GstPad *pad, bool downstream = true);
   void removeProbeFromPad(GstPad *pad);

 protected:
   virtual void probeCaps(GstCaps *caps);
   virtual bool probeBuffer(GstBuffer *buffer);

 private:

#if GST_CHECK_VERSION(1,0,0)
   static GstPadProbeReturn capsProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
   static GstPadProbeReturn bufferProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
   int m_capsProbeId;
#else
   static gboolean bufferProbe(GstElement *element, GstBuffer *buffer, gpointer user_data);
   GstCaps *m_caps;
#endif

   int m_bufferProbeId;
   const Flags m_flags;
};

#endif
