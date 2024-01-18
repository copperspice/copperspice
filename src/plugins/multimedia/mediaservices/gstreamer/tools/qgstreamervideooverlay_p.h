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

#ifndef QGSTREAMERVIDEOOVERLAY_P_H
#define QGSTREAMERVIDEOOVERLAY_P_H

#include <qgstreamerbushelper_p.h>
#include <qgstreamerbufferprobe_p.h>
#include <qwindowdefs.h>
#include <qsize.h>

class QGstreamerVideoOverlay
   : public QObject, public QGstreamerSyncMessageFilter
   , public QGstreamerBusMessageFilter, private QGstreamerBufferProbe
{
   CS_OBJECT_MULTIPLE(QGstreamerVideoOverlay, QObject)

   CS_INTERFACES(QGstreamerSyncMessageFilter, QGstreamerBusMessageFilter)

 public:
   explicit QGstreamerVideoOverlay(QObject *parent = nullptr, const QByteArray &elementName = QByteArray());
   virtual ~QGstreamerVideoOverlay();

   GstElement *videoSink() const;
   QSize nativeVideoSize() const;

   void setWindowHandle(WId id);
   void expose();
   void setRenderRectangle(const QRect &rect);

   bool isActive() const;

   Qt::AspectRatioMode aspectRatioMode() const;
   void setAspectRatioMode(Qt::AspectRatioMode mode);

   int brightness() const;
   void setBrightness(int brightness);

   int contrast() const;
   void setContrast(int contrast);

   int hue() const;
   void setHue(int hue);

   int saturation() const;
   void setSaturation(int saturation);

   bool processSyncMessage(const QGstreamerMessage &message) override;
   bool processBusMessage(const QGstreamerMessage &message) override;

   CS_SIGNAL_1(Public, void nativeVideoSizeChanged())
   CS_SIGNAL_2(nativeVideoSizeChanged)

   CS_SIGNAL_1(Public, void activeChanged())
   CS_SIGNAL_2(activeChanged)

   CS_SIGNAL_1(Public, void brightnessChanged(int brightness))
   CS_SIGNAL_2(brightnessChanged, brightness)

   CS_SIGNAL_1(Public, void contrastChanged(int contrast))
   CS_SIGNAL_2(contrastChanged, contrast)

   CS_SIGNAL_1(Public, void hueChanged(int hue))
   CS_SIGNAL_2(hueChanged, hue)

   CS_SIGNAL_1(Public, void saturationChanged(int saturation))
   CS_SIGNAL_2(saturationChanged, saturation)

 private:
   GstElement *findBestVideoSink() const;
   void setWindowHandle_helper(WId id);
   void updateIsActive();
   void probeCaps(GstCaps *caps) override;
   static void showPrerollFrameChanged(GObject *, GParamSpec *, QGstreamerVideoOverlay *);

   GstElement *m_videoSink;
   QSize m_nativeVideoSize;
   bool m_isActive;

   bool m_hasForceAspectRatio;
   bool m_hasBrightness;
   bool m_hasContrast;
   bool m_hasHue;
   bool m_hasSaturation;
   bool m_hasShowPrerollFrame;

   WId m_windowId;
   Qt::AspectRatioMode m_aspectRatioMode;
   int m_brightness;
   int m_contrast;
   int m_hue;
   int m_saturation;
};

#endif

