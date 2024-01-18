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

#ifndef QVIDEOWIDGET_P_H
#define QVIDEOWIDGET_P_H

#include <qpointer.h>
#include <qvideowidget.h>

#ifndef QT_NO_OPENGL
#include <qglwidget.h>
#endif

#include <qpaintervideosurface_p.h>

class QMediaService;
class QVideoWidgetControl;
class QVideoRendererControl;
class QVideoWindowControl;
class QMediaService;
class QVideoOutputControl;

class QVideoWidgetControlInterface
{
 public:
   virtual ~QVideoWidgetControlInterface()
   { }

   virtual void setBrightness(int brightness) = 0;
   virtual void setContrast(int contrast) = 0;
   virtual void setHue(int hue) = 0;
   virtual void setSaturation(int saturation) = 0;

   virtual void setFullScreen(bool fullScreen) = 0;

   virtual Qt::AspectRatioMode aspectRatioMode() const = 0;
   virtual void setAspectRatioMode(Qt::AspectRatioMode mode) = 0;
};

class QVideoWidgetBackend : public QObject, public QVideoWidgetControlInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QVideoWidgetBackend, QObject)

 public:
   virtual QSize sizeHint() const = 0;

   virtual void showEvent() = 0;
   virtual void hideEvent(QHideEvent *event) = 0;
   virtual void resizeEvent(QResizeEvent *event) = 0;
   virtual void moveEvent(QMoveEvent *event) = 0;
   virtual void paintEvent(QPaintEvent *event) = 0;
};

class QVideoWidgetControlBackend : public QObject, public QVideoWidgetControlInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QVideoWidgetControlBackend, QObject)

 public:
   QVideoWidgetControlBackend(QMediaService *service, QVideoWidgetControl *control, QVideoWidget *widget);

   void releaseControl();

   void setBrightness(int brightness) override;
   void setContrast(int contrast) override;
   void setHue(int hue) override;
   void setSaturation(int saturation) override;

   void setFullScreen(bool fullScreen) override;

   Qt::AspectRatioMode aspectRatioMode() const override;
   void setAspectRatioMode(Qt::AspectRatioMode mode) override;

 private:
   QMediaService *m_service;
   QVideoWidgetControl *m_widgetControl;
};

class QRendererVideoWidgetBackend : public QVideoWidgetBackend
{
   MULTI_CS_OBJECT(QRendererVideoWidgetBackend)

 public:
   QRendererVideoWidgetBackend(QMediaService *service, QVideoRendererControl *control, QVideoWidget *widget);
   ~QRendererVideoWidgetBackend();

   void releaseControl();
   void clearSurface();

   void setBrightness(int brightness) override;
   void setContrast(int contrast) override;
   void setHue(int hue) override;
   void setSaturation(int saturation) override;

   void setFullScreen(bool fullScreen) override;

   Qt::AspectRatioMode aspectRatioMode() const override;
   void setAspectRatioMode(Qt::AspectRatioMode mode) override;

   QSize sizeHint() const override;

   void showEvent() override;
   void hideEvent(QHideEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void moveEvent(QMoveEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

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

 private:
   MULTI_CS_SLOT_1(Private, void formatChanged(const QVideoSurfaceFormat &format))
   MULTI_CS_SLOT_2(formatChanged)

   MULTI_CS_SLOT_1(Private, void frameChanged())
   MULTI_CS_SLOT_2(frameChanged)

   void updateRects();

   QMediaService *m_service;
   QVideoRendererControl *m_rendererControl;
   QVideoWidget *m_widget;
   QPainterVideoSurface *m_surface;
   Qt::AspectRatioMode m_aspectRatioMode;
   QRect m_boundingRect;
   QRectF m_sourceRect;
   QSize m_nativeSize;
   bool m_updatePaintDevice;
};

class QWindowVideoWidgetBackend : public QVideoWidgetBackend
{
   MULTI_CS_OBJECT(QWindowVideoWidgetBackend)

 public:
   QWindowVideoWidgetBackend(QMediaService *service, QVideoWindowControl *control, QVideoWidget *widget);
   ~QWindowVideoWidgetBackend();

   void releaseControl();

   void setBrightness(int brightness) override;
   void setContrast(int contrast) override;
   void setHue(int hue) override;
   void setSaturation(int saturation) override;

   void setFullScreen(bool fullScreen) override;

   Qt::AspectRatioMode aspectRatioMode() const override;
   void setAspectRatioMode(Qt::AspectRatioMode mode) override;

   QSize sizeHint() const override;

   void showEvent() override;
   void hideEvent(QHideEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void moveEvent(QMoveEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

 private:
   QMediaService *m_service;
   QVideoWindowControl *m_windowControl;
   QVideoWidget *m_widget;
   QSize m_pixelAspectRatio;
};

class QVideoWidgetPrivate
{
   Q_DECLARE_PUBLIC(QVideoWidget)

 public:
   QVideoWidgetPrivate()
      : q_ptr(nullptr), mediaObject(nullptr), service(nullptr), widgetBackend(nullptr),
        windowBackend(nullptr), rendererBackend(nullptr), currentControl(nullptr), currentBackend(nullptr),
        brightness(0), contrast(0), hue(0), saturation(0),
        aspectRatioMode(Qt::KeepAspectRatio), nonFullScreenFlags(Qt::EmptyFlag), wasFullScreen(false) {
   }

   QVideoWidget *q_ptr;
   QPointer<QMediaObject> mediaObject;
   QMediaService *service;
   QVideoWidgetControlBackend *widgetBackend;
   QWindowVideoWidgetBackend *windowBackend;
   QRendererVideoWidgetBackend *rendererBackend;
   QVideoWidgetControlInterface *currentControl;
   QVideoWidgetBackend *currentBackend;
   int brightness;
   int contrast;
   int hue;
   int saturation;
   Qt::AspectRatioMode aspectRatioMode;
   Qt::WindowFlags nonFullScreenFlags;
   bool wasFullScreen;

   bool createWidgetBackend();
   bool createWindowBackend();
   bool createRendererBackend();

   void setCurrentControl(QVideoWidgetControlInterface *control);
   void clearService();

   void _q_serviceDestroyed();
   void _q_brightnessChanged(int brightness);
   void _q_contrastChanged(int contrast);
   void _q_hueChanged(int hue);
   void _q_saturationChanged(int saturation);
   void _q_fullScreenChanged(bool fullScreen);
   void _q_dimensionsChanged();
};

#endif
