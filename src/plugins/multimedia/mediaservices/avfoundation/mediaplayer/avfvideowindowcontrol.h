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

#ifndef AVFVIDEOWINDOWCONTROL_H
#define AVFVIDEOWINDOWCONTROL_H

#include <QVideoWindowControl>

@class AVPlayerLayer;
#if defined(Q_OS_DARWIN)
@class NSView;
typedef NSView NativeView;
#else
@class UIView;
typedef UIView NativeView;
#endif

#include "avfvideooutput.h"

class AVFVideoWindowControl : public QVideoWindowControl, public AVFVideoOutput
{
   CS_OBJECT_MULTIPLE(AVFVideoWindowControl,  QVideoWindowControl)
   CS_INTERFACES(AVFVideoOutput)

 public:
   AVFVideoWindowControl(QObject *parent = nullptr);
   virtual ~AVFVideoWindowControl();

   // QVideoWindowControl interface

   WId winId() const override;
   void setWinId(WId id) override;

   QRect displayRect() const override;
   void setDisplayRect(const QRect &rect) override;

   bool isFullScreen() const override;
   void setFullScreen(bool fullScreen) override;

   void repaint() override;
   QSize nativeSize() const override;

   Qt::AspectRatioMode aspectRatioMode() const override;
   void setAspectRatioMode(Qt::AspectRatioMode mode) override;

   int brightness() const override;
   void setBrightness(int brightness) override;

   int contrast() const override;
   void setContrast(int contrast) override;

   int hue() const override;
   void setHue(int hue) override;

   int saturation() const override;
   void setSaturation(int saturation) override;

   // AVFVideoOutput interface
   void setLayer(void *playerLayer) override;

 private:
   void updateAspectRatio();
   void updatePlayerLayerBounds();

   WId m_winId;
   QRect m_displayRect;
   bool m_fullscreen;
   int m_brightness;
   int m_contrast;
   int m_hue;
   int m_saturation;
   Qt::AspectRatioMode m_aspectRatioMode;
   QSize m_nativeSize;

   AVPlayerLayer *m_playerLayer;
   NativeView *m_nativeView;
};

#endif
