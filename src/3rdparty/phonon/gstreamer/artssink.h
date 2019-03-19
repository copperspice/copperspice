/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef GSTREAMER_ARTSSINK_H
#define GSTREAMER_ARTSSINK_H

#include <QtCore>
#include <sys/types.h>
#include <gst/gst.h>
#include <gst/audio/gstaudiosink.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{

G_BEGIN_DECLS

extern "C" {

#define GST_TYPE_ARTS_SINK           (arts_sink_get_type())
#define GST_ARTS_SINK(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ARTS_SINK,ArtsSink))
#define GST_ARTS_SINK_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ARTS_SINK,ArtsSinkClass))
#define GST_IS_ARTS_SINK(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ARTS_SINK))
#define GST_IS_ARTS_SINK_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ARTS_SINK))

typedef struct _ArtsSink ArtsSink;
typedef struct _ArtsSinkClass ArtsSinkClass;

enum arts_parameter_t_enum {
    ARTS_P_BUFFER_SIZE = 1,
    ARTS_P_BUFFER_TIME = 2,
    ARTS_P_BUFFER_SPACE = 3,
    ARTS_P_SERVER_LATENCY = 4,
    ARTS_P_TOTAL_LATENCY = 5,
    ARTS_P_BLOCKING = 6,
    ARTS_P_PACKET_SIZE = 7,
    ARTS_P_PACKET_COUNT = 8,
    ARTS_P_PACKET_SETTINGS = 9
};

typedef void *arts_stream_t;

struct _ArtsSink {
    GstAudioSink sink;
    arts_stream_t stream;
    int samplerate;
    int samplebits;
    int channels;
    int bytes_per_sample;
};

struct GConfClient;
struct GError;
typedef void (*Ptr_g_type_init)();
typedef GConfClient* (*Ptr_gconf_client_get_default)();
typedef char* (*Ptr_gconf_client_get_string)(GConfClient*, const char*, GError **);
typedef void (*Ptr_g_object_unref)(void *);
typedef void (*Ptr_g_error_free)(GError *);
typedef void (*Ptr_g_free)(void*);

struct _ArtsSinkClass {
    GstAudioSinkClass parent_class;
};

GType arts_sink_get_type (void);
}
G_END_DECLS

}
} //namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif // __ARTS_SINK_H__
