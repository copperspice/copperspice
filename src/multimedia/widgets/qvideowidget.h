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

#ifndef QVIDEOWIDGET_H
#define QVIDEOWIDGET_H

#include <qwidget.h>
#include <qmediabindableinterface.h>

class QMediaObject;
class QVideoWidgetPrivate;

class Q_MULTIMEDIA_EXPORT QVideoWidget : public QWidget, public QMediaBindableInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QVideoWidget, QWidget)

   CS_INTERFACES(QMediaBindableInterface)

   MULTI_CS_PROPERTY_READ(mediaObject, mediaObject)
   MULTI_CS_PROPERTY_WRITE(mediaObject, cs_setMediaObject)

   MULTI_CS_PROPERTY_READ(fullScreen, isFullScreen)
   MULTI_CS_PROPERTY_WRITE(fullScreen, setFullScreen)
   MULTI_CS_PROPERTY_NOTIFY(fullScreen, fullScreenChanged)

   MULTI_CS_PROPERTY_READ(aspectRatioMode, aspectRatioMode)
   MULTI_CS_PROPERTY_WRITE(aspectRatioMode, setAspectRatioMode)

   MULTI_CS_PROPERTY_READ(brightness, brightness)
   MULTI_CS_PROPERTY_WRITE(brightness, setBrightness)
   MULTI_CS_PROPERTY_NOTIFY(brightness, brightnessChanged)

   MULTI_CS_PROPERTY_READ(contrast, contrast)
   MULTI_CS_PROPERTY_WRITE(contrast, setContrast)
   MULTI_CS_PROPERTY_NOTIFY(contrast, contrastChanged)

   MULTI_CS_PROPERTY_READ(hue, hue)
   MULTI_CS_PROPERTY_WRITE(hue, setHue)
   MULTI_CS_PROPERTY_NOTIFY(hue, hueChanged)

   MULTI_CS_PROPERTY_READ(saturation, saturation)
   MULTI_CS_PROPERTY_WRITE(saturation, setSaturation)
   MULTI_CS_PROPERTY_NOTIFY(saturation, saturationChanged)

 public:
   explicit QVideoWidget(QWidget *parent = nullptr);
   ~QVideoWidget();

   QMediaObject *mediaObject() const override;

   Qt::AspectRatioMode aspectRatioMode() const;

   int brightness() const;
   int contrast() const;
   int hue() const;
   int saturation() const;

   QSize sizeHint() const override;

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

   MULTI_CS_SLOT_1(Public, void setFullScreen(bool fullScreen))
   MULTI_CS_SLOT_2(setFullScreen)

   MULTI_CS_SLOT_1(Public, void setAspectRatioMode(Qt::AspectRatioMode mode))
   MULTI_CS_SLOT_2(setAspectRatioMode)

   MULTI_CS_SLOT_1(Public, void setBrightness(int brightness))
   MULTI_CS_SLOT_2(setBrightness)

   MULTI_CS_SLOT_1(Public, void setContrast(int contrast))
   MULTI_CS_SLOT_2(setContrast)

   MULTI_CS_SLOT_1(Public, void setHue(int hue))
   MULTI_CS_SLOT_2(setHue)

   // these 6 slots were private
   MULTI_CS_SLOT_1(Public, void _q_brightnessChanged(int brightness))
   MULTI_CS_SLOT_2(_q_brightnessChanged)

   MULTI_CS_SLOT_1(Public, void _q_contrastChanged(int contrast))
   MULTI_CS_SLOT_2(_q_contrastChanged)

   MULTI_CS_SLOT_1(Public, void _q_hueChanged(int hue))
   MULTI_CS_SLOT_2(_q_hueChanged)

   MULTI_CS_SLOT_1(Public, void _q_saturationChanged(int saturation))
   MULTI_CS_SLOT_2(_q_saturationChanged)

   MULTI_CS_SLOT_1(Public, void setSaturation(int saturation))
   MULTI_CS_SLOT_2(setSaturation)

   MULTI_CS_SLOT_1(Public, void _q_fullScreenChanged(bool fullScreen))
   MULTI_CS_SLOT_2(_q_fullScreenChanged)

   MULTI_CS_SLOT_1(Public, void _q_dimensionsChanged())
   MULTI_CS_SLOT_2(_q_dimensionsChanged)

 protected:
   bool event(QEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void hideEvent(QHideEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void moveEvent(QMoveEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

   bool setMediaObject(QMediaObject *object) override;

   QVideoWidget(QVideoWidgetPrivate &dd, QWidget *parent);
   QVideoWidgetPrivate *d_ptr;

 private:
   Q_DECLARE_PRIVATE(QVideoWidget)

   // wrapper
   void cs_setMediaObject(QMediaObject *object) {
      setMediaObject(object);
   }

   MULTI_CS_SLOT_1(Private, void _q_serviceDestroyed())
   MULTI_CS_SLOT_2(_q_serviceDestroyed)
};

#endif


