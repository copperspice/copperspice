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

#include <qgstreamervideowidget_p.h>

#include <qcoreevent.h>
#include <qdebug.h>
#include <qpainter.h>

class QGstreamerVideoWidget : public QWidget
{
 public:
   QGstreamerVideoWidget(QWidget *parent = nullptr)
      : QWidget(parent)
   {
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      QPalette palette;
      palette.setColor(QPalette::Background, Qt::black);
      setPalette(palette);
   }

   virtual ~QGstreamerVideoWidget() {}

   QSize sizeHint() const override {
      return m_nativeSize;
   }

   void setNativeSize( const QSize &size) {
      if (size != m_nativeSize) {
         m_nativeSize = size;
         if (size.isEmpty()) {
            setMinimumSize(0, 0);
         } else {
            setMinimumSize(160, 120);
         }

         updateGeometry();
      }
   }

   void paint_helper() {
      QPainter painter(this);
      painter.fillRect(rect(), palette().background());
   }

 protected:
   void paintEvent(QPaintEvent *) override {
      paint_helper();
   }

   QSize m_nativeSize;
};

QGstreamerVideoWidgetControl::QGstreamerVideoWidgetControl(QObject *parent, const QByteArray &elementName)
   : QVideoWidgetControl(parent)
   , m_videoOverlay(this, !elementName.isEmpty() ? elementName : qgetenv("QT_GSTREAMER_WIDGET_VIDEOSINK"))
   , m_widget(nullptr)
   , m_stopped(false)
   , m_windowId(0)
   , m_fullScreen(false)
{
   connect(&m_videoOverlay, &QGstreamerVideoOverlay::activeChanged,
      this, &QGstreamerVideoWidgetControl::onOverlayActiveChanged);
   connect(&m_videoOverlay, &QGstreamerVideoOverlay::nativeVideoSizeChanged,
      this, &QGstreamerVideoWidgetControl::onNativeVideoSizeChanged);
   connect(&m_videoOverlay, &QGstreamerVideoOverlay::brightnessChanged,
      this, &QGstreamerVideoWidgetControl::brightnessChanged);
   connect(&m_videoOverlay, &QGstreamerVideoOverlay::contrastChanged,
      this, &QGstreamerVideoWidgetControl::contrastChanged);
   connect(&m_videoOverlay, &QGstreamerVideoOverlay::hueChanged,
      this, &QGstreamerVideoWidgetControl::hueChanged);
   connect(&m_videoOverlay, &QGstreamerVideoOverlay::saturationChanged,
      this, &QGstreamerVideoWidgetControl::saturationChanged);
}

QGstreamerVideoWidgetControl::~QGstreamerVideoWidgetControl()
{
   delete m_widget;
}

void QGstreamerVideoWidgetControl::createVideoWidget()
{
   if (m_widget) {
      return;
   }

   m_widget = new QGstreamerVideoWidget;

   m_widget->installEventFilter(this);
   m_videoOverlay.setWindowHandle(m_windowId = m_widget->winId());
}

GstElement *QGstreamerVideoWidgetControl::videoSink()
{
   return m_videoOverlay.videoSink();
}

void QGstreamerVideoWidgetControl::onOverlayActiveChanged()
{
   updateWidgetAttributes();
}

void QGstreamerVideoWidgetControl::stopRenderer()
{
   m_stopped = true;
   updateWidgetAttributes();
   m_widget->setNativeSize(QSize());
}

void QGstreamerVideoWidgetControl::onNativeVideoSizeChanged()
{
   const QSize &size = m_videoOverlay.nativeVideoSize();

   if (size.isValid()) {
      m_stopped = false;
   }

   if (m_widget) {
      m_widget->setNativeSize(size);
   }
}

bool QGstreamerVideoWidgetControl::eventFilter(QObject *object, QEvent *e)
{
   if (m_widget && object == m_widget) {
      if (e->type() == QEvent::ParentChange || e->type() == QEvent::Show || e->type() == QEvent::WinIdChange) {
         WId newWId = m_widget->winId();
         if (newWId != m_windowId) {
            m_videoOverlay.setWindowHandle(m_windowId = newWId);
         }
      }

      if (e->type() == QEvent::Paint) {
         if (m_videoOverlay.isActive()) {
            m_videoOverlay.expose();   // triggers a repaint of the last frame
         } else {
            m_widget->paint_helper();   // paints the black background
         }

         return true;
      }
   }

   return false;
}

void QGstreamerVideoWidgetControl::updateWidgetAttributes()
{
   // When frames are being rendered (sink is active), we need the WA_PaintOnScreen attribute to
   // be set in order to avoid flickering when the widget is repainted (for example when resized).
   // We need to clear that flag when the the sink is inactive to allow the widget to paint its
   // background, otherwise some garbage will be displayed.
   if (m_videoOverlay.isActive() && !m_stopped) {
      m_widget->setAttribute(Qt::WA_NoSystemBackground, true);
      m_widget->setAttribute(Qt::WA_PaintOnScreen, true);
   } else {
      m_widget->setAttribute(Qt::WA_NoSystemBackground, false);
      m_widget->setAttribute(Qt::WA_PaintOnScreen, false);
      m_widget->update();
   }
}

bool QGstreamerVideoWidgetControl::processSyncMessage(const QGstreamerMessage &message)
{
   return m_videoOverlay.processSyncMessage(message);
}

bool QGstreamerVideoWidgetControl::processBusMessage(const QGstreamerMessage &message)
{
   return m_videoOverlay.processBusMessage(message);
}

QWidget *QGstreamerVideoWidgetControl::videoWidget()
{
   createVideoWidget();
   return m_widget;
}

Qt::AspectRatioMode QGstreamerVideoWidgetControl::aspectRatioMode() const
{
   return m_videoOverlay.aspectRatioMode();
}

void QGstreamerVideoWidgetControl::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   m_videoOverlay.setAspectRatioMode(mode);
}

bool QGstreamerVideoWidgetControl::isFullScreen() const
{
   return m_fullScreen;
}

void QGstreamerVideoWidgetControl::setFullScreen(bool fullScreen)
{
   emit fullScreenChanged(m_fullScreen =  fullScreen);
}

int QGstreamerVideoWidgetControl::brightness() const
{
   return m_videoOverlay.brightness();
}

void QGstreamerVideoWidgetControl::setBrightness(int brightness)
{
   m_videoOverlay.setBrightness(brightness);
}

int QGstreamerVideoWidgetControl::contrast() const
{
   return m_videoOverlay.contrast();
}

void QGstreamerVideoWidgetControl::setContrast(int contrast)
{
   m_videoOverlay.setContrast(contrast);
}

int QGstreamerVideoWidgetControl::hue() const
{
   return m_videoOverlay.hue();
}

void QGstreamerVideoWidgetControl::setHue(int hue)
{
   m_videoOverlay.setHue(hue);
}

int QGstreamerVideoWidgetControl::saturation() const
{
   return m_videoOverlay.saturation();
}

void QGstreamerVideoWidgetControl::setSaturation(int saturation)
{
   m_videoOverlay.setSaturation(saturation);
}


