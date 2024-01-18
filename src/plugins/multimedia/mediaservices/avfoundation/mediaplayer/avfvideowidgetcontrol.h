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

#ifndef AVFVIDEOWIDGETCONTROL_H
#define AVFVIDEOWIDGETCONTROL_H

#include <qvideowidgetcontrol.h>
#include <avfvideooutput.h>

@class AVPlayerLayer;

class AVFVideoWidget;

class AVFVideoWidgetControl : public QVideoWidgetControl, public AVFVideoOutput
{
   CS_OBJECT_MULTIPLE(AVFVideoWidgetControl, QVideoWidgetControl)
   CS_INTERFACES(AVFVideoOutput)

 public:
   AVFVideoWidgetControl(QObject *parent = nullptr);
   virtual ~AVFVideoWidgetControl();

   void setLayer(void *playerLayer) override;

   QWidget *videoWidget() override;

   bool isFullScreen() const override;
   void setFullScreen(bool fullScreen) override;

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

 private:
   AVFVideoWidget *m_videoWidget;

   bool m_fullscreen;
   int m_brightness;
   int m_contrast;
   int m_hue;
   int m_saturation;
};

#endif
