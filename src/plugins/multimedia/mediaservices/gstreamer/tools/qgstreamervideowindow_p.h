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

#ifndef QGSTREAMERVIDEOWINDOW_H
#define QGSTREAMERVIDEOWINDOW_H

#include <qcolor.h>
#include <qvideowindowcontrol.h>

#include <qgstreamervideorendererinterface_p.h>
#include <qgstreamerbushelper_p.h>
#include <qgstreamervideooverlay_p.h>

class QAbstractVideoSurface;

class QGstreamerVideoWindow : public QVideoWindowControl, public QGstreamerVideoRendererInterface,
   public QGstreamerSyncMessageFilter, public QGstreamerBusMessageFilter
{
   CS_OBJECT_MULTIPLE(QGstreamerVideoWindow, QVideoWindowControl)

   CS_INTERFACES(QGstreamerVideoRendererInterface, QGstreamerSyncMessageFilter, QGstreamerBusMessageFilter)

 public:
   explicit QGstreamerVideoWindow(QObject *parent = nullptr, const QByteArray &elementName = QByteArray());
   ~QGstreamerVideoWindow();

   WId winId() const override;
   void setWinId(WId id) override;

   QRect displayRect() const override;
   void setDisplayRect(const QRect &rect) override;

   bool isFullScreen() const override;
   void setFullScreen(bool fullScreen) override;

   QSize nativeSize() const override;

   Qt::AspectRatioMode aspectRatioMode() const override;
   void setAspectRatioMode(Qt::AspectRatioMode mode) override;

   void repaint() override;

   int brightness() const override;
   void setBrightness(int brightness) override;

   int contrast() const override;
   void setContrast(int contrast) override;

   int hue() const override;
   void setHue(int hue) override;

   int saturation() const override;
   void setSaturation(int saturation) override;

   QAbstractVideoSurface *surface() const;

   GstElement *videoSink() override;

   bool processSyncMessage(const QGstreamerMessage &message) override;
   bool processBusMessage(const QGstreamerMessage &message) override;

   bool isReady() const  override {
      return m_windowId != 0;
   }

   CS_SIGNAL_1(Public, void sinkChanged())
   CS_SIGNAL_2(sinkChanged)

   CS_SIGNAL_1(Public, void readyChanged(bool isReady))
   CS_SIGNAL_2(readyChanged, isReady)

 private:
   QGstreamerVideoOverlay m_videoOverlay;
   WId m_windowId;
   QRect m_displayRect;
   bool m_fullScreen;
   mutable QColor m_colorKey;
};

#endif
