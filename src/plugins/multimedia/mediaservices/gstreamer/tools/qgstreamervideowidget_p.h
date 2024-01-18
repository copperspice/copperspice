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

#ifndef QGSTREAMERVIDEOWIDGET_H
#define QGSTREAMERVIDEOWIDGET_H

#include <qvideowidgetcontrol.h>

#include <qgstreamervideorendererinterface_p.h>
#include <qgstreamerbushelper_p.h>
#include <qgstreamervideooverlay_p.h>

class QGstreamerVideoWidget;

class QGstreamerVideoWidgetControl
   : public QVideoWidgetControl, public QGstreamerVideoRendererInterface
   , public QGstreamerSyncMessageFilter, public QGstreamerBusMessageFilter
{
   CS_OBJECT_MULTIPLE(QGstreamerVideoWidgetControl, QVideoWidgetControl)

   CS_INTERFACES(QGstreamerVideoRendererInterface, QGstreamerSyncMessageFilter, QGstreamerBusMessageFilter)

 public:
   explicit QGstreamerVideoWidgetControl(QObject *parent = nullptr, const QByteArray &elementName = QByteArray());
   virtual ~QGstreamerVideoWidgetControl();

   GstElement *videoSink() override;

   QWidget *videoWidget() override;

   void stopRenderer() override;

   Qt::AspectRatioMode aspectRatioMode() const override;
   void setAspectRatioMode(Qt::AspectRatioMode mode) override;

   bool isFullScreen() const override;
   void setFullScreen(bool fullScreen) override;

   int brightness() const override;
   void setBrightness(int brightness) override;

   int contrast() const override;
   void setContrast(int contrast) override;

   int hue() const override;
   void setHue(int hue) override;

   int saturation() const override;
   void setSaturation(int saturation) override;

   bool eventFilter(QObject *object, QEvent *event) override;

   CS_SIGNAL_1(Public, void sinkChanged())
   CS_SIGNAL_2(sinkChanged)

   CS_SIGNAL_1(Public, void readyChanged(bool isReady))
   CS_SIGNAL_2(readyChanged, isReady)

 private:
   void createVideoWidget();
   void updateWidgetAttributes();

   bool processSyncMessage(const QGstreamerMessage &message) override;
   bool processBusMessage(const QGstreamerMessage &message) override;

   QGstreamerVideoOverlay m_videoOverlay;
   QGstreamerVideoWidget *m_widget;
   bool m_stopped;
   WId m_windowId;
   bool m_fullScreen;

   CS_SLOT_1(Private, void onOverlayActiveChanged())
   CS_SLOT_2(onOverlayActiveChanged)

   CS_SLOT_1(Private, void onNativeVideoSizeChanged())
   CS_SLOT_2(onNativeVideoSizeChanged)
};

#endif
