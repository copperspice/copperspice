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

#include <avfvideowidgetcontrol.h>
#include <avfvideowidget.h>

#ifdef QT_DEBUG_AVF
#include <QDebug>
#endif

#import <AVFoundation/AVFoundation.h>

AVFVideoWidgetControl::AVFVideoWidgetControl(QObject *parent)
   : QVideoWidgetControl(parent)
   , m_fullscreen(false)
   , m_brightness(0)
   , m_contrast(0)
   , m_hue(0)
   , m_saturation(0)
{
   m_videoWidget = new AVFVideoWidget(nullptr);
}

AVFVideoWidgetControl::~AVFVideoWidgetControl()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif
   delete m_videoWidget;
}

void AVFVideoWidgetControl::setLayer(void *playerLayer)
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO << playerLayer;
#endif

   m_videoWidget->setPlayerLayer((AVPlayerLayer *)playerLayer);

}

QWidget *AVFVideoWidgetControl::videoWidget()
{
   return m_videoWidget;
}

bool AVFVideoWidgetControl::isFullScreen() const
{
   return m_fullscreen;
}

void AVFVideoWidgetControl::setFullScreen(bool fullScreen)
{
   m_fullscreen = fullScreen;
}

Qt::AspectRatioMode AVFVideoWidgetControl::aspectRatioMode() const
{
   return m_videoWidget->aspectRatioMode();
}

void AVFVideoWidgetControl::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   m_videoWidget->setAspectRatioMode(mode);
}

int AVFVideoWidgetControl::brightness() const
{
   return m_brightness;
}

void AVFVideoWidgetControl::setBrightness(int brightness)
{
   m_brightness = brightness;
}

int AVFVideoWidgetControl::contrast() const
{
   return m_contrast;
}

void AVFVideoWidgetControl::setContrast(int contrast)
{
   m_contrast = contrast;
}

int AVFVideoWidgetControl::hue() const
{
   return m_hue;
}

void AVFVideoWidgetControl::setHue(int hue)
{
   m_hue = hue;
}

int AVFVideoWidgetControl::saturation() const
{
   return m_saturation;
}

void AVFVideoWidgetControl::setSaturation(int saturation)
{
   m_saturation = saturation;
}

