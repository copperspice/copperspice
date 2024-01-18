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

#include "avfvideowindowcontrol.h"

#include <AVFoundation/AVFoundation.h>

#if defined(Q_OS_DARWIN)
#import <AppKit/AppKit.h>
#endif

AVFVideoWindowControl::AVFVideoWindowControl(QObject *parent)
   : QVideoWindowControl(parent)
   , m_winId(0)
   , m_fullscreen(false)
   , m_brightness(0)
   , m_contrast(0)
   , m_hue(0)
   , m_saturation(0)
   , m_aspectRatioMode(Qt::IgnoreAspectRatio)
   , m_playerLayer(nullptr)
   , m_nativeView(nullptr)
{ }

AVFVideoWindowControl::~AVFVideoWindowControl()
{
   if (m_playerLayer) {
      [m_playerLayer removeFromSuperlayer];
      [m_playerLayer release];
   }
}

WId AVFVideoWindowControl::winId() const
{
   return m_winId;
}

void AVFVideoWindowControl::setWinId(WId id)
{
   m_winId = id;
   m_nativeView = (NativeView *)m_winId;
}

QRect AVFVideoWindowControl::displayRect() const
{
   return m_displayRect;
}

void AVFVideoWindowControl::setDisplayRect(const QRect &rect)
{
   if (m_displayRect != rect) {
      m_displayRect = rect;
      updatePlayerLayerBounds();
   }
}

bool AVFVideoWindowControl::isFullScreen() const
{
   return m_fullscreen;
}

void AVFVideoWindowControl::setFullScreen(bool fullScreen)
{
   if (m_fullscreen != fullScreen) {
      m_fullscreen = fullScreen;
      Q_EMIT QVideoWindowControl::fullScreenChanged(fullScreen);
   }
}

void AVFVideoWindowControl::repaint()
{
   if (m_playerLayer) {
      [m_playerLayer setNeedsDisplay];
   }
}

QSize AVFVideoWindowControl::nativeSize() const
{
   return m_nativeSize;
}

Qt::AspectRatioMode AVFVideoWindowControl::aspectRatioMode() const
{
   return m_aspectRatioMode;
}

void AVFVideoWindowControl::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   if (m_aspectRatioMode != mode) {
      m_aspectRatioMode = mode;
      updateAspectRatio();
   }
}

int AVFVideoWindowControl::brightness() const
{
   return m_brightness;
}

void AVFVideoWindowControl::setBrightness(int brightness)
{
   if (m_brightness != brightness) {
      m_brightness = brightness;
      Q_EMIT QVideoWindowControl::brightnessChanged(brightness);
   }
}

int AVFVideoWindowControl::contrast() const
{
   return m_contrast;
}

void AVFVideoWindowControl::setContrast(int contrast)
{
   if (m_contrast != contrast) {
      m_contrast = contrast;
      Q_EMIT QVideoWindowControl::contrastChanged(contrast);
   }
}

int AVFVideoWindowControl::hue() const
{
   return m_hue;
}

void AVFVideoWindowControl::setHue(int hue)
{
   if (m_hue != hue) {
      m_hue = hue;
      Q_EMIT QVideoWindowControl::hueChanged(hue);
   }
}

int AVFVideoWindowControl::saturation() const
{
   return m_saturation;
}

void AVFVideoWindowControl::setSaturation(int saturation)
{
   if (m_saturation != saturation) {
      m_saturation = saturation;
      Q_EMIT QVideoWindowControl::saturationChanged(saturation);
   }
}

void AVFVideoWindowControl::setLayer(void *playerLayer)
{
   AVPlayerLayer *layer = (AVPlayerLayer *)playerLayer;
   if (m_playerLayer == layer) {
      return;
   }

   if (!m_winId) {
      qDebug("AVFVideoWindowControl: No video window");
      return;
   }

#if defined(Q_OS_DARWIN)
   [m_nativeView setWantsLayer: YES];
#endif

   if (m_playerLayer) {
      [m_playerLayer removeFromSuperlayer];
      [m_playerLayer release];
   }

   m_playerLayer = layer;

   CALayer *nativeLayer = [m_nativeView layer];

   if (layer) {
      [layer retain];

      m_nativeSize = QSize(m_playerLayer.bounds.size.width,
            m_playerLayer.bounds.size.height);

      updateAspectRatio();
      [nativeLayer addSublayer: m_playerLayer];
      updatePlayerLayerBounds();
   }
}

void AVFVideoWindowControl::updateAspectRatio()
{
   if (m_playerLayer) {
      switch (m_aspectRatioMode) {
         case Qt::IgnoreAspectRatio:
            [m_playerLayer setVideoGravity: AVLayerVideoGravityResize];
            break;
         case Qt::KeepAspectRatio:
            [m_playerLayer setVideoGravity: AVLayerVideoGravityResizeAspect];
            break;
         case Qt::KeepAspectRatioByExpanding:
            [m_playerLayer setVideoGravity: AVLayerVideoGravityResizeAspectFill];
            break;
         default:
            break;
      }
   }
}

void AVFVideoWindowControl::updatePlayerLayerBounds()
{
   if (m_playerLayer) {
      CGRect newBounds = CGRectMake(0, 0,
            m_displayRect.width(), m_displayRect.height());
      m_playerLayer.bounds = newBounds;
      m_playerLayer.position = CGPointMake(m_displayRect.x(), m_displayRect.y());
   }
}
