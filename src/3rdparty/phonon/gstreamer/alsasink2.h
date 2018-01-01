/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/*****************************************************
** Copyright (C)  2005 Wim Taymans <wim@fluendo.com>
*****************************************************/

#ifndef GSTREAMER_ALSASINK2_H
#define GSTREAMER_ALSASINK2_H

#include <gst/gst.h>
#include <gst/audio/gstaudiosink.h>
#include <alsa/asoundlib.h>

G_BEGIN_DECLS

#define GST_TYPE_ALSA_SINK2            (gst_alsasink2_get_type())
#define GST_ALSA_SINK2(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ALSA_SINK2,_k_GstAlsaSink))
#define GST_ALSA_SINK2_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ALSA_SINK2,_k_GstAlsaSinkClass))
#define GST_IS_ALSA_SINK2(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ALSA_SINK2))
#define GST_IS_ALSA_SINK2_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ALSA_SINK2))
#define GST_ALSA_SINK2_CAST(obj)       ((_k_GstAlsaSink *) (obj))

typedef struct _k_GstAlsaSink _k_GstAlsaSink;
typedef struct _k_GstAlsaSinkClass _k_GstAlsaSinkClass;

#define GST_ALSA_SINK2_GET_LOCK(obj)	(GST_ALSA_SINK2_CAST (obj)->alsa_lock)
#define GST_ALSA_SINK2_LOCK(obj)	        (g_mutex_lock (GST_ALSA_SINK2_GET_LOCK (obj)))
#define GST_ALSA_SINK2_UNLOCK(obj)	(g_mutex_unlock (GST_ALSA_SINK2_GET_LOCK (obj)))

/*
 * _k_GstAlsaSink:
 *
 * Opaque data structure
 */
struct _k_GstAlsaSink {
  GstAudioSink    sink;

  gchar                 *device;

  snd_pcm_t             *handle;
  snd_pcm_hw_params_t   *hwparams;
  snd_pcm_sw_params_t   *swparams;

  snd_pcm_access_t access;
  snd_pcm_format_t format;
  guint rate;
  guint channels;
  gint bytes_per_sample;
  gboolean iec958;
  gboolean need_swap;

  guint buffer_time;
  guint period_time;
  snd_pcm_uframes_t buffer_size;
  snd_pcm_uframes_t period_size;

  GstCaps *cached_caps;

  GMutex *alsa_lock;
};

struct _k_GstAlsaSinkClass {
  GstAudioSinkClass parent_class;
};

GType gst_alsasink2_get_type(void);

G_END_DECLS

#endif /* ALSASINK2_H */
