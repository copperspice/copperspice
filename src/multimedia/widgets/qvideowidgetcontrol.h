/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QVIDEOWIDGETCONTROL_H
#define QVIDEOWIDGETCONTROL_H

#include <qvideowidget.h>
#include <qmediacontrol.h>
#include <qwidget.h>

class QVideoWidgetControlPrivate;

class Q_MULTIMEDIA_EXPORT QVideoWidgetControl : public QMediaControl
{
   MULTI_CS_OBJECT(QVideoWidgetControl)

 public:
   virtual ~QVideoWidgetControl();

   virtual QWidget *videoWidget() = 0;

   virtual Qt::AspectRatioMode aspectRatioMode() const = 0;
   virtual void setAspectRatioMode(Qt::AspectRatioMode mode) = 0;

   virtual bool isFullScreen() const = 0;
   virtual void setFullScreen(bool fullScreen) = 0;

   virtual int brightness() const = 0;
   virtual void setBrightness(int brightness) = 0;

   virtual int contrast() const = 0;
   virtual void setContrast(int contrast) = 0;

   virtual int hue() const = 0;
   virtual void setHue(int hue) = 0;

   virtual int saturation() const = 0;
   virtual void setSaturation(int saturation) = 0;

   MULTI_CS_SIGNAL_1(Public, void fullScreenChanged(bool fullScreen))
   MULTI_CS_SIGNAL_2(fullScreenChanged, fullScreen)
   MULTI_CS_SIGNAL_1(Public, void brightnessChanged(int brightness))
   MULTI_CS_SIGNAL_2(brightnessChanged, brightness)
   MULTI_CS_SIGNAL_1(Public, void contrastChanged(int contrast))
   MULTI_CS_SIGNAL_2(contrastChanged, contrast)
   MULTI_CS_SIGNAL_1(Public, void hueChanged(int hue))
   MULTI_CS_SIGNAL_2(hueChanged, hue)
   MULTI_CS_SIGNAL_1(Public, void saturationChanged(int saturation))
   MULTI_CS_SIGNAL_2(saturationChanged, saturation)

 protected:
   explicit QVideoWidgetControl(QObject *parent = nullptr);
};

#define QVideoWidgetControl_iid "com.copperpsice.CS.videoWidgetControl/1.0"
CS_DECLARE_INTERFACE(QVideoWidgetControl, QVideoWidgetControl_iid)

#endif
