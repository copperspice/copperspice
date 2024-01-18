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

#include <gstvideoconnector_p.h>
#include <unistd.h>

/* signals */
enum
{
  SIGNAL_RESEND_NEW_SEGMENT,
  SIGNAL_CONNECTION_FAILED,
  LAST_SIGNAL
};
static guint gst_video_connector_signals[LAST_SIGNAL] = { 0 };

GST_DEBUG_CATEGORY_STATIC (video_connector_debug);
#define GST_CAT_DEFAULT video_connector_debug

static GstStaticPadTemplate gst_video_connector_sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
                         GST_PAD_SINK,
                         GST_PAD_ALWAYS,
                         GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate gst_video_connector_src_factory =
GST_STATIC_PAD_TEMPLATE ("src",
                         GST_PAD_SRC,
                         GST_PAD_ALWAYS,
                         GST_STATIC_CAPS_ANY);

#define _do_init(bla) \
    GST_DEBUG_CATEGORY_INIT (video_connector_debug, \
    "video-connector", 0, "An identity like element for reconnecting video stream");

GST_BOILERPLATE_FULL (GstVideoConnector, gst_video_connector, GstElement, GST_TYPE_ELEMENT, _do_init);

static void gst_video_connector_dispose (GObject * object);
static GstFlowReturn gst_video_connector_chain (GstPad * pad, GstBuffer * buf);

static GstFlowReturn gst_video_connector_buffer_alloc (GstPad * pad,
                  guint64 offset, guint size, GstCaps * caps, GstBuffer ** buf);

static GstStateChangeReturn gst_video_connector_change_state (GstElement *element, GstStateChange transition);

static gboolean gst_video_connector_handle_sink_event (GstPad * pad, GstEvent * event);
static gboolean gst_video_connector_new_buffer_probe(GstObject *pad, GstBuffer *buffer, guint * object);
static void gst_video_connector_resend_new_segment(GstElement * element, gboolean emitFailedSignal);
static gboolean gst_video_connector_setcaps (GstPad  *pad, GstCaps *caps);
static GstCaps *gst_video_connector_getcaps (GstPad * pad);
static gboolean gst_video_connector_acceptcaps (GstPad * pad, GstCaps * caps);

static void gst_video_connector_base_init (gpointer g_class)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

    gst_element_class_set_details_simple (element_class, "Video Connector", "Generic",
                  "An identity like element used for reconnecting video stream",
                  "Dmytro Poplavskiy <dmytro.poplavskiy@nokia.com>");

    gst_element_class_add_pad_template (element_class,
                  gst_static_pad_template_get (&gst_video_connector_sink_factory));

    gst_element_class_add_pad_template (element_class,
                  gst_static_pad_template_get (&gst_video_connector_src_factory));
}

static void gst_video_connector_class_init (GstVideoConnectorClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);

    parent_class = reinterpret_cast<GstElementClass *>(g_type_class_peek_parent(klass));

    gobject_class->dispose         = gst_video_connector_dispose;
    gstelement_class->change_state = gst_video_connector_change_state;
    klass->resend_new_segment      = gst_video_connector_resend_new_segment;

    gst_video_connector_signals[SIGNAL_RESEND_NEW_SEGMENT] =
            g_signal_new ("resend-new-segment", G_TYPE_FROM_CLASS (klass),
                          GSignalFlags(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                          G_STRUCT_OFFSET (GstVideoConnectorClass, resend_new_segment), nullptr, nullptr,
                          g_cclosure_marshal_VOID__BOOLEAN, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

    gst_video_connector_signals[SIGNAL_CONNECTION_FAILED] =
            g_signal_new ("connection-failed", G_TYPE_FROM_CLASS (klass),
                          G_SIGNAL_RUN_LAST, 0, nullptr, nullptr,
                          g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void gst_video_connector_init (GstVideoConnector *element, GstVideoConnectorClass *g_class)
{
    (void) g_class;
    element->sinkpad = gst_pad_new_from_static_template (&gst_video_connector_sink_factory, "sink");
    gst_pad_set_chain_function(element->sinkpad, GST_DEBUG_FUNCPTR (gst_video_connector_chain));
    gst_pad_set_event_function(element->sinkpad, GST_DEBUG_FUNCPTR (gst_video_connector_handle_sink_event));
    gst_pad_set_bufferalloc_function(element->sinkpad, GST_DEBUG_FUNCPTR (gst_video_connector_buffer_alloc));
    gst_pad_set_setcaps_function(element->sinkpad, GST_DEBUG_FUNCPTR (gst_video_connector_setcaps));
    gst_pad_set_getcaps_function(element->sinkpad, GST_DEBUG_FUNCPTR(gst_video_connector_getcaps));
    gst_pad_set_acceptcaps_function(element->sinkpad, GST_DEBUG_FUNCPTR(gst_video_connector_acceptcaps));

    gst_element_add_pad (GST_ELEMENT (element), element->sinkpad);

    element->srcpad = gst_pad_new_from_static_template (&gst_video_connector_src_factory, "src");
    gst_pad_add_buffer_probe(element->srcpad, G_CALLBACK(gst_video_connector_new_buffer_probe), element);
    gst_element_add_pad (GST_ELEMENT (element), element->srcpad);

    element->relinked = FALSE;
    element->failedSignalEmited = FALSE;
    gst_segment_init (&element->segment, GST_FORMAT_TIME);
    element->latest_buffer = nullptr;
}

static void gst_video_connector_reset (GstVideoConnector * element)
{
    element->relinked = FALSE;
    element->failedSignalEmited = FALSE;

    if (element->latest_buffer != nullptr) {
        gst_buffer_unref (element->latest_buffer);
        element->latest_buffer = nullptr;
    }

    gst_segment_init (&element->segment, GST_FORMAT_UNDEFINED);
}

static void gst_video_connector_dispose (GObject * object)
{
    GstVideoConnector *element = GST_VIDEO_CONNECTOR (object);

    gst_video_connector_reset (element);

    G_OBJECT_CLASS (parent_class)->dispose (object);
}

// "When this function returns anything else than GST_FLOW_OK,
// the buffer allocation failed and buf does not contain valid data."
static GstFlowReturn gst_video_connector_buffer_alloc (GstPad * pad, guint64 offset, guint size,
                                  GstCaps * caps, GstBuffer ** buf)
{
    GstVideoConnector *element;
    GstFlowReturn res = GST_FLOW_OK;
    element = GST_VIDEO_CONNECTOR (GST_PAD_PARENT (pad));

    if (!buf) {
        return GST_FLOW_ERROR;
    }
    *buf = nullptr;

    gboolean isFailed = FALSE;
    while (1) {
        GST_OBJECT_LOCK (element);
        gst_object_ref(element->srcpad);
        GST_OBJECT_UNLOCK (element);

        // Check if downstream element is in NULL state
        // and wait for up to 1 second for it to switch.
        GstPad *peerPad = gst_pad_get_peer(element->srcpad);

        if (peerPad) {
            GstElement *parent = gst_pad_get_parent_element(peerPad);
            gst_object_unref (peerPad);

            if (parent) {
                GstState state;
                GstState pending;
                int totalTimeout = 0;

                // This seems to sleep for about 10ms usually.
                while (totalTimeout < 1000000) {
                    gst_element_get_state(parent, &state, &pending, 0);
                    if (state != GST_STATE_NULL) {
                        break;
                    }

                    usleep(5000);
                    totalTimeout += 5000;
                }

                gst_object_unref (parent);
                if (state == GST_STATE_NULL) {
                    GST_DEBUG_OBJECT (element, "Downstream element is in NULL state");
                    // Downstream filter seems to be in the wrong state
                    return GST_FLOW_UNEXPECTED;
                }
            }
        }

        res = gst_pad_alloc_buffer(element->srcpad, offset, size, caps, buf);
        gst_object_unref (element->srcpad);

        GST_DEBUG_OBJECT (element, "buffer alloc finished: %s", gst_flow_get_name (res));

        if (res == GST_FLOW_WRONG_STATE) {
            // Just in case downstream filter is still somehow in the wrong state.
            // Pipeline stalls if we report GST_FLOW_WRONG_STATE.
            return GST_FLOW_UNEXPECTED;
        }

        if (res >= GST_FLOW_OK || isFailed == TRUE)
            break;

        //if gst_pad_alloc_buffer failed, emit "connection-failed" signal
        //so colorspace transformation element can be inserted
        GST_INFO_OBJECT(element, "gst_video_connector_buffer_alloc failed, emit connection-failed signal");
        g_signal_emit(G_OBJECT(element), gst_video_connector_signals[SIGNAL_CONNECTION_FAILED], 0);
        isFailed = TRUE;
    }

    return res;
}

static gboolean gst_video_connector_setcaps (GstPad  *pad, GstCaps *caps)
{
    GstVideoConnector *element;
    element = GST_VIDEO_CONNECTOR (GST_PAD_PARENT (pad));

    /* forward-negotiate */
    gboolean res = gst_pad_set_caps(element->srcpad, caps);

    gchar * debugmsg = nullptr;
    GST_DEBUG_OBJECT(element, "gst_video_connector_setcaps %s %i", debugmsg = gst_caps_to_string(caps), res);
    if (debugmsg)
        g_free(debugmsg);

    if (!res) {
        //if set_caps failed, emit "connection-failed" signal
        //so colorspace transformation element can be inserted
        GST_INFO_OBJECT(element, "gst_video_connector_setcaps failed, emit connection-failed signal");
        g_signal_emit(G_OBJECT(element), gst_video_connector_signals[SIGNAL_CONNECTION_FAILED], 0);

        return gst_pad_set_caps(element->srcpad, caps);
    }

    return TRUE;
}

static GstCaps *gst_video_connector_getcaps (GstPad * pad)
{
    GstVideoConnector *element;
    element = GST_VIDEO_CONNECTOR (GST_PAD_PARENT (pad));

#if (GST_VERSION_MICRO > 25)
    GstCaps *caps = gst_pad_peer_get_caps_reffed(element->srcpad);
#else
    GstCaps *caps = gst_pad_peer_get_caps(element->srcpad);
#endif

    if (! caps) {
        caps = gst_caps_new_any();
    }

    return caps;
}

static gboolean gst_video_connector_acceptcaps (GstPad * pad, GstCaps * caps)
{
    GstVideoConnector *element;
    element = GST_VIDEO_CONNECTOR (GST_PAD_PARENT (pad));

    return gst_pad_peer_accept_caps(element->srcpad, caps);
}

static void gst_video_connector_resend_new_segment(GstElement * element, gboolean emitFailedSignal)
{
    GST_INFO_OBJECT(element, "New segment requested, failed signal enabled: %i", emitFailedSignal);
    GstVideoConnector *connector = GST_VIDEO_CONNECTOR(element);
    connector->relinked = TRUE;

    if (emitFailedSignal) {
        connector->failedSignalEmited = FALSE;
    }
}

static gboolean gst_video_connector_new_buffer_probe(GstObject *pad, GstBuffer *buffer, guint * object)
{
    (void) pad;
    (void) buffer;

    GstVideoConnector *element = GST_VIDEO_CONNECTOR (object);

    /*
      If relinking is requested, the current buffer should be rejected and
      the new segment + previous buffer should be pushed first
    */

    if (element->relinked)
        GST_LOG_OBJECT(element, "rejected buffer because of new segment request");

    return !element->relinked;
}

static GstFlowReturn gst_video_connector_chain (GstPad * pad, GstBuffer * buf)
{
    GstFlowReturn res;
    GstVideoConnector *element;

    element = GST_VIDEO_CONNECTOR (gst_pad_get_parent (pad));

    do {
        /*
          Resend the segment message and last buffer to preroll the new sink.
          Sinks can be changed multiple times while paused,
          while loop allows to send the segment message and preroll
          all of them with the same buffer.
        */
        while (element->relinked) {
            element->relinked = FALSE;

            gint64 pos = element->segment.last_stop;

            if (element->latest_buffer && GST_BUFFER_TIMESTAMP_IS_VALID(element->latest_buffer)) {
                pos = GST_BUFFER_TIMESTAMP (element->latest_buffer);
            }

            //push a new segment and last buffer
            GstEvent *ev = gst_event_new_new_segment (TRUE,
                                                      element->segment.rate,
                                                      element->segment.format,
                                                      pos, //start
                                                      element->segment.stop,
                                                      pos);

            GST_DEBUG_OBJECT (element, "Pushing new segment event");
            if (!gst_pad_push_event (element->srcpad, ev)) {
                GST_WARNING_OBJECT (element, "New segment handling failed in %" GST_PTR_FORMAT, element->srcpad);
            }

            if (element->latest_buffer) {
                GST_DEBUG_OBJECT (element, "Pushing latest buffer...");
                gst_buffer_ref(element->latest_buffer);
                gst_pad_push(element->srcpad, element->latest_buffer);
            }
        }

        gst_buffer_ref(buf);

        //it's possible video sink is changed during gst_pad_push blocked by
        //pad lock, in this case ( element->relinked == TRUE )
        //the buffer should be rejected by the buffer probe and
        //the new segment + prev buffer should be sent before

        GST_LOG_OBJECT (element, "Pushing buffer...");
        res = gst_pad_push (element->srcpad, buf);
        GST_LOG_OBJECT (element, "Pushed buffer: %s", gst_flow_get_name (res));

        //if gst_pad_push failed give the service another chance,
        //it may still work with the colorspace element added
        if (!element->failedSignalEmited && res == GST_FLOW_NOT_NEGOTIATED) {
            element->failedSignalEmited = TRUE;
            GST_INFO_OBJECT(element, "gst_pad_push failed, emit connection-failed signal");
            g_signal_emit(G_OBJECT(element), gst_video_connector_signals[SIGNAL_CONNECTION_FAILED], 0);
        }

    } while (element->relinked);


    if (element->latest_buffer) {
        gst_buffer_unref (element->latest_buffer);
        element->latest_buffer = nullptr;
    }

    //don't save the last video buffer on maemo6 because of buffers shortage with omapxvsink
    element->latest_buffer = gst_buffer_ref(buf);

    gst_buffer_unref(buf);
    gst_object_unref (element);

    return res;
}

static GstStateChangeReturn gst_video_connector_change_state (GstElement * element,
                  GstStateChange transition)
{
    GstVideoConnector *connector;
    GstStateChangeReturn result;

    connector = GST_VIDEO_CONNECTOR(element);
    result = GST_ELEMENT_CLASS (parent_class)->change_state(element, transition);

    switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
        gst_video_connector_reset (connector);
        break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
        connector->relinked = FALSE;
        break;
    default:
        break;
    }

    return result;
}

static gboolean gst_video_connector_handle_sink_event (GstPad * pad, GstEvent * event)
{
    if (GST_EVENT_TYPE (event) == GST_EVENT_NEWSEGMENT) {
        GstVideoConnector *element = GST_VIDEO_CONNECTOR (gst_pad_get_parent (pad));

        gboolean update;
        GstFormat format;
        gdouble rate, arate;
        gint64 start, stop, time;

        gst_event_parse_new_segment_full (event, &update, &rate, &arate, &format,
                                          &start, &stop, &time);

        GST_LOG_OBJECT (element,
                          "NEWSEGMENT update %d, rate %lf, applied rate %lf, "
                          "format %d, " "%" G_GINT64_FORMAT " -- %" G_GINT64_FORMAT ", time %"
                          G_GINT64_FORMAT, update, rate, arate, format, start, stop, time);

        gst_segment_set_newsegment_full (&element->segment, update,
                                         rate, arate, format, start, stop, time);

        gst_object_unref (element);
    }

    return gst_pad_event_default (pad, event);
}
