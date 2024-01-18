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

#include <avfvideowidget.h>

#include <QDebug>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>

#include <AVFoundation/AVFoundation.h>

#if defined(Q_OS_DARWIN)
#import <AppKit/AppKit.h>
#else
#import <UIKit/UIKit.h>
#endif

AVFVideoWidget::AVFVideoWidget(QWidget *parent)
   : QWidget(parent), m_aspectRatioMode(Qt::KeepAspectRatio), m_playerLayer(nullptr), m_nativeView(nullptr)
{
   setAutoFillBackground(false);
}

AVFVideoWidget::~AVFVideoWidget()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif

   if (m_playerLayer) {
      [m_playerLayer removeFromSuperlayer];
      [m_playerLayer release];
   }
}

QSize AVFVideoWidget::sizeHint() const
{
   return m_nativeSize;
}

Qt::AspectRatioMode AVFVideoWidget::aspectRatioMode() const
{
   return m_aspectRatioMode;
}

void AVFVideoWidget::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   if (m_aspectRatioMode != mode) {
      m_aspectRatioMode = mode;

      updateAspectRatio();
   }
}

void AVFVideoWidget::setPlayerLayer(AVPlayerLayer *layer)
{
   if (m_playerLayer == layer) {
      return;
   }

   if (!m_nativeView) {
      //make video widget a native window
#if defined(Q_OS_DARWIN)
      m_nativeView = (NSView *)this->winId();
      [m_nativeView setWantsLayer: YES];
#else
      m_nativeView = (UIView *)this->winId();
#endif
   }

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
      updatePlayerLayerBounds(this->size());
   }
#ifdef QT_DEBUG_AVF
   NSArray *sublayers = [nativeLayer sublayers];
   qDebug() << "playerlayer: " << "at z:" << [m_playerLayer zPosition]
      << " frame: " << m_playerLayer.frame.size.width << "x"  << m_playerLayer.frame.size.height;
   qDebug() << "superlayer: " << "at z:" << [nativeLayer zPosition]
      << " frame: " << nativeLayer.frame.size.width << "x"  << nativeLayer.frame.size.height;
   int i = 0;
   for (CALayer * layer in sublayers) {
      qDebug() << "layer " << i << ": at z:" << [layer zPosition]
         << " frame: " << layer.frame.size.width << "x"  << layer.frame.size.height;
      i++;
   }
#endif

}

void AVFVideoWidget::resizeEvent(QResizeEvent *event)
{
   updatePlayerLayerBounds(event->size());
   QWidget::resizeEvent(event);
}

void AVFVideoWidget::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);
   painter.fillRect(rect(), Qt::black);

   QWidget::paintEvent(event);
}

void AVFVideoWidget::updateAspectRatio()
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

void AVFVideoWidget::updatePlayerLayerBounds(const QSize &size)
{
   m_playerLayer.bounds = CGRectMake(0.0f, 0.0f, (float)size.width(), (float)size.height());
}
