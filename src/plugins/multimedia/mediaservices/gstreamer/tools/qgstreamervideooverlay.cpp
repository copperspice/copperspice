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

#include <qgstreamervideooverlay_p.h>

#include <qguiapplication.h>
#include <qgstutils_p.h>

#if ! GST_CHECK_VERSION(1,0,0)
#include <gst/interfaces/xoverlay.h>
#else
#include <gst/video/videooverlay.h>
#endif

struct ElementMap {
   const char *qtPlatform;
   const char *gstreamerElement;
};

// Ordered by descending priority
static const ElementMap elementMap[] = {
   { "xcb", "vaapisink" },
   { "xcb", "xvimagesink" },
   { "xcb", "ximagesink" }
};

QGstreamerVideoOverlay::QGstreamerVideoOverlay(QObject *parent, const QByteArray &elementName)
   : QObject(parent)
   , QGstreamerBufferProbe(QGstreamerBufferProbe::ProbeCaps)
   , m_videoSink(nullptr)
   , m_isActive(false)
   , m_hasForceAspectRatio(false)
   , m_hasBrightness(false)
   , m_hasContrast(false)
   , m_hasHue(false)
   , m_hasSaturation(false)
   , m_hasShowPrerollFrame(false)
   , m_windowId(0)
   , m_aspectRatioMode(Qt::KeepAspectRatio)
   , m_brightness(0)
   , m_contrast(0)
   , m_hue(0)
   , m_saturation(0)
{
   if (!elementName.isEmpty()) {
      m_videoSink = gst_element_factory_make(elementName.constData(), nullptr);
   } else {
      m_videoSink = findBestVideoSink();
   }

   if (m_videoSink) {
      qt_gst_object_ref_sink(GST_OBJECT(m_videoSink)); // Take ownership

      GstPad *pad = gst_element_get_static_pad(m_videoSink, "sink");
      addProbeToPad(pad);
      gst_object_unref(GST_OBJECT(pad));

      m_hasForceAspectRatio = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "force-aspect-ratio");
      m_hasBrightness       = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "brightness");
      m_hasContrast         = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "contrast");
      m_hasHue              = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "hue");
      m_hasSaturation       = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "saturation");
      m_hasShowPrerollFrame = g_object_class_find_property(G_OBJECT_GET_CLASS(m_videoSink), "show-preroll-frame");

      if (m_hasShowPrerollFrame) {
         g_signal_connect(m_videoSink, "notify::show-preroll-frame", G_CALLBACK(showPrerollFrameChanged), this);
      }
   }
}

QGstreamerVideoOverlay::~QGstreamerVideoOverlay()
{
   if (m_videoSink) {
      GstPad *pad = gst_element_get_static_pad(m_videoSink, "sink");
      removeProbeFromPad(pad);
      gst_object_unref(GST_OBJECT(pad));
      gst_object_unref(GST_OBJECT(m_videoSink));
   }
}

static bool qt_gst_element_is_functioning(GstElement *element)
{
   GstStateChangeReturn ret = gst_element_set_state(element, GST_STATE_READY);
   if (ret == GST_STATE_CHANGE_SUCCESS) {
      gst_element_set_state(element, GST_STATE_NULL);
      return true;
   }

   return false;
}

GstElement *QGstreamerVideoOverlay::findBestVideoSink() const
{
   GstElement *choice = nullptr;
   QString platform = QGuiApplication::platformName();

   // We need a native window ID to use the GstVideoOverlay interface.
   // Bail out if the platform plugin in use cannot provide a sensible WId.
   if (platform != "xcb") {
      return nullptr;
   }

   // First, try some known video sinks, depending on the Qt platform plugin in use.
   for (quint32 i = 0; i < (sizeof(elementMap) / sizeof(ElementMap)); ++i) {

      if (platform == QString::fromLatin1(elementMap[i].qtPlatform) &&
         (choice = gst_element_factory_make(elementMap[i].gstreamerElement, nullptr))) {

         if (qt_gst_element_is_functioning(choice)) {
            return choice;
         }

         gst_object_unref(choice);
         choice = nullptr;
      }
   }

   // If none of the known video sinks are available, try to find one that implements the
   // GstVideoOverlay interface and has autoplugging rank.
   GList *list = qt_gst_video_sinks();
   for (GList *item = list; item != nullptr; item = item->next) {
      GstElementFactory *f = GST_ELEMENT_FACTORY(item->data);

      if (! gst_element_factory_has_interface(f, QT_GSTREAMER_VIDEOOVERLAY_INTERFACE_NAME)) {
         continue;
      }

      if (GstElement *el = gst_element_factory_create(f, nullptr)) {
         if (qt_gst_element_is_functioning(el)) {
            choice = el;
            break;
         }

         gst_object_unref(el);
      }
   }

   gst_plugin_feature_list_free(list);

   return choice;
}

GstElement *QGstreamerVideoOverlay::videoSink() const
{
   return m_videoSink;
}

QSize QGstreamerVideoOverlay::nativeVideoSize() const
{
   return m_nativeVideoSize;
}

void QGstreamerVideoOverlay::setWindowHandle(WId id)
{
   m_windowId = id;

   if (isActive()) {
      setWindowHandle_helper(id);
   }
}

void QGstreamerVideoOverlay::setWindowHandle_helper(WId id)
{
#if GST_CHECK_VERSION(1,0,0)
   if (m_videoSink && GST_IS_VIDEO_OVERLAY(m_videoSink)) {
      gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(m_videoSink), id);
#else
   if (m_videoSink && GST_IS_X_OVERLAY(m_videoSink)) {
# if GST_CHECK_VERSION(0,10,31)
      gst_x_overlay_set_window_handle(GST_X_OVERLAY(m_videoSink), id);
# else
      gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_videoSink), id);
# endif
#endif

      // Properties need to be reset when changing the winId.
      setAspectRatioMode(m_aspectRatioMode);
      setBrightness(m_brightness);
      setContrast(m_contrast);
      setHue(m_hue);
      setSaturation(m_saturation);
   }
}

void QGstreamerVideoOverlay::expose()
{
   if (!isActive()) {
      return;
   }

#if !GST_CHECK_VERSION(1,0,0)
   if (m_videoSink && GST_IS_X_OVERLAY(m_videoSink)) {
      gst_x_overlay_expose(GST_X_OVERLAY(m_videoSink));
   }
#else
   if (m_videoSink && GST_IS_VIDEO_OVERLAY(m_videoSink)) {
      gst_video_overlay_expose(GST_VIDEO_OVERLAY(m_videoSink));
   }
#endif
}

void QGstreamerVideoOverlay::setRenderRectangle(const QRect &rect)
{
   int x = -1;
   int y = -1;
   int w = -1;
   int h = -1;

   if (!rect.isEmpty()) {
      x = rect.x();
      y = rect.y();
      w = rect.width();
      h = rect.height();
   }

#if GST_CHECK_VERSION(1,0,0)
   if (m_videoSink && GST_IS_VIDEO_OVERLAY(m_videoSink)) {
      gst_video_overlay_set_render_rectangle(GST_VIDEO_OVERLAY(m_videoSink), x, y, w, h);
   }
#elif GST_CHECK_VERSION(0, 10, 29)
   if (m_videoSink && GST_IS_X_OVERLAY(m_videoSink)) {
      gst_x_overlay_set_render_rectangle(GST_X_OVERLAY(m_videoSink), x, y, w, h);
   }
#else
   (void) x;
   (void) y;
   (void) w;
   (void) h;
#endif
}

bool QGstreamerVideoOverlay::processSyncMessage(const QGstreamerMessage &message)
{
   GstMessage *gm = message.rawMessage();

#if !GST_CHECK_VERSION(1,0,0)
   if (gm && (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ELEMENT) &&
      gst_structure_has_name(gm->structure, "prepare-xwindow-id")) {
#else
   if (gm && (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_ELEMENT) &&
      gst_structure_has_name(gst_message_get_structure(gm), "prepare-window-handle")) {
#endif
      setWindowHandle_helper(m_windowId);
      return true;
   }

   return false;
}

bool QGstreamerVideoOverlay::processBusMessage(const QGstreamerMessage &message)
{
   GstMessage *gm = message.rawMessage();

   if (GST_MESSAGE_TYPE(gm) == GST_MESSAGE_STATE_CHANGED &&
      GST_MESSAGE_SRC(gm) == GST_OBJECT_CAST(m_videoSink)) {

      updateIsActive();
   }

   return false;
}

void QGstreamerVideoOverlay::probeCaps(GstCaps *caps)
{
   QSize size = QGstUtils::capsCorrectedResolution(caps);
   if (size != m_nativeVideoSize) {
      m_nativeVideoSize = size;
      emit nativeVideoSizeChanged();
   }
}

bool QGstreamerVideoOverlay::isActive() const
{
   return m_isActive;
}

void QGstreamerVideoOverlay::updateIsActive()
{
   if (!m_videoSink) {
      return;
   }

   GstState state = GST_STATE(m_videoSink);
   gboolean showPreroll = true;

   if (m_hasShowPrerollFrame) {
      g_object_get(G_OBJECT(m_videoSink), "show-preroll-frame", &showPreroll, nullptr);
   }

   bool newIsActive = (state == GST_STATE_PLAYING || (state == GST_STATE_PAUSED && showPreroll));

   if (newIsActive != m_isActive) {
      m_isActive = newIsActive;
      emit activeChanged();
   }
}

void QGstreamerVideoOverlay::showPrerollFrameChanged(GObject *, GParamSpec *, QGstreamerVideoOverlay *overlay)
{
   overlay->updateIsActive();
}

Qt::AspectRatioMode QGstreamerVideoOverlay::aspectRatioMode() const
{
   Qt::AspectRatioMode mode = Qt::KeepAspectRatio;

   if (m_hasForceAspectRatio) {
      gboolean forceAR = false;
      g_object_get(G_OBJECT(m_videoSink), "force-aspect-ratio", &forceAR, nullptr);
      if (!forceAR) {
         mode = Qt::IgnoreAspectRatio;
      }
   }

   return mode;
}

void QGstreamerVideoOverlay::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   if (m_hasForceAspectRatio) {
      g_object_set(G_OBJECT(m_videoSink), "force-aspect-ratio", (mode == Qt::KeepAspectRatio), nullptr);
   }

   m_aspectRatioMode = mode;
}

int QGstreamerVideoOverlay::brightness() const
{
   int brightness = 0;

   if (m_hasBrightness) {
      g_object_get(G_OBJECT(m_videoSink), "brightness", &brightness, nullptr);
   }

   return brightness / 10;
}

void QGstreamerVideoOverlay::setBrightness(int brightness)
{
   if (m_hasBrightness) {
      g_object_set(G_OBJECT(m_videoSink), "brightness", brightness * 10, nullptr);
      emit brightnessChanged(brightness);
   }

   m_brightness = brightness;
}

int QGstreamerVideoOverlay::contrast() const
{
   int contrast = 0;

   if (m_hasContrast) {
      g_object_get(G_OBJECT(m_videoSink), "contrast", &contrast, nullptr);
   }

   return contrast / 10;
}

void QGstreamerVideoOverlay::setContrast(int contrast)
{
   if (m_hasContrast) {
      g_object_set(G_OBJECT(m_videoSink), "contrast", contrast * 10, nullptr);
      emit contrastChanged(contrast);
   }

   m_contrast = contrast;
}

int QGstreamerVideoOverlay::hue() const
{
   int hue = 0;

   if (m_hasHue) {
      g_object_get(G_OBJECT(m_videoSink), "hue", &hue, nullptr);
   }

   return hue / 10;
}

void QGstreamerVideoOverlay::setHue(int hue)
{
   if (m_hasHue) {
      g_object_set(G_OBJECT(m_videoSink), "hue", hue * 10, nullptr);
      emit hueChanged(hue);
   }

   m_hue = hue;
}

int QGstreamerVideoOverlay::saturation() const
{
   int saturation = 0;

   if (m_hasSaturation) {
      g_object_get(G_OBJECT(m_videoSink), "saturation", &saturation, nullptr);
   }

   return saturation / 10;
}

void QGstreamerVideoOverlay::setSaturation(int saturation)
{
   if (m_hasSaturation) {
      g_object_set(G_OBJECT(m_videoSink), "saturation", saturation * 10, nullptr);
      emit saturationChanged(saturation);
   }

   m_saturation = saturation;
}

