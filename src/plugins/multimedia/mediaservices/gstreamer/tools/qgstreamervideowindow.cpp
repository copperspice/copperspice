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

#include <qgstreamervideowindow_p.h>

#include <qdebug.h>
#include <qgstutils_p.h>

QGstreamerVideoWindow::QGstreamerVideoWindow(QObject *parent, const QByteArray &elementName)
   : QVideoWindowControl(parent)
   , m_videoOverlay(this, !elementName.isEmpty() ? elementName : qgetenv("QT_GSTREAMER_WINDOW_VIDEOSINK"))
   , m_windowId(0), m_fullScreen(false), m_colorKey(QColor::Invalid)
{
   connect(&m_videoOverlay, &QGstreamerVideoOverlay::nativeVideoSizeChanged,
      this, &QGstreamerVideoWindow::nativeSizeChanged);

   connect(&m_videoOverlay, &QGstreamerVideoOverlay::brightnessChanged,
      this, &QGstreamerVideoWindow::brightnessChanged);

   connect(&m_videoOverlay, &QGstreamerVideoOverlay::contrastChanged,
      this, &QGstreamerVideoWindow::contrastChanged);

   connect(&m_videoOverlay, &QGstreamerVideoOverlay::hueChanged,
      this, &QGstreamerVideoWindow::hueChanged);

   connect(&m_videoOverlay, &QGstreamerVideoOverlay::saturationChanged,
      this, &QGstreamerVideoWindow::saturationChanged);
}

QGstreamerVideoWindow::~QGstreamerVideoWindow()
{
}

GstElement *QGstreamerVideoWindow::videoSink()
{
   return m_videoOverlay.videoSink();
}

WId QGstreamerVideoWindow::winId() const
{
   return m_windowId;
}

void QGstreamerVideoWindow::setWinId(WId id)
{
   if (m_windowId == id) {
      return;
   }

   WId oldId = m_windowId;
   m_videoOverlay.setWindowHandle(m_windowId = id);

   if (!oldId) {
      emit readyChanged(true);
   }

   if (!id) {
      emit readyChanged(false);
   }
}

bool QGstreamerVideoWindow::processSyncMessage(const QGstreamerMessage &message)
{
   return m_videoOverlay.processSyncMessage(message);
}

bool QGstreamerVideoWindow::processBusMessage(const QGstreamerMessage &message)
{
   return m_videoOverlay.processBusMessage(message);
}

QRect QGstreamerVideoWindow::displayRect() const
{
   return m_displayRect;
}

void QGstreamerVideoWindow::setDisplayRect(const QRect &rect)
{
   m_videoOverlay.setRenderRectangle(m_displayRect = rect);
   repaint();
}

Qt::AspectRatioMode QGstreamerVideoWindow::aspectRatioMode() const
{
   return m_videoOverlay.aspectRatioMode();
}

void QGstreamerVideoWindow::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   m_videoOverlay.setAspectRatioMode(mode);
}

void QGstreamerVideoWindow::repaint()
{
   m_videoOverlay.expose();
}

int QGstreamerVideoWindow::brightness() const
{
   return m_videoOverlay.brightness();
}

void QGstreamerVideoWindow::setBrightness(int brightness)
{
   m_videoOverlay.setBrightness(brightness);
}

int QGstreamerVideoWindow::contrast() const
{
   return m_videoOverlay.contrast();
}

void QGstreamerVideoWindow::setContrast(int contrast)
{
   m_videoOverlay.setContrast(contrast);
}

int QGstreamerVideoWindow::hue() const
{
   return m_videoOverlay.hue();
}

void QGstreamerVideoWindow::setHue(int hue)
{
   m_videoOverlay.setHue(hue);
}

int QGstreamerVideoWindow::saturation() const
{
   return m_videoOverlay.saturation();
}

void QGstreamerVideoWindow::setSaturation(int saturation)
{
   m_videoOverlay.setSaturation(saturation);
}

bool QGstreamerVideoWindow::isFullScreen() const
{
   return m_fullScreen;
}

void QGstreamerVideoWindow::setFullScreen(bool fullScreen)
{
   emit fullScreenChanged(m_fullScreen = fullScreen);
}

QSize QGstreamerVideoWindow::nativeSize() const
{
   return m_videoOverlay.nativeVideoSize();
}
