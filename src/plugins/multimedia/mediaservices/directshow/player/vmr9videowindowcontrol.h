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

#ifndef VMR9VIDEOWINDOWCONTROL_H
#define VMR9VIDEOWINDOWCONTROL_H

#include "qvideowindowcontrol.h"

#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>

class Vmr9VideoWindowControl : public QVideoWindowControl
{
   CS_OBJECT(Vmr9VideoWindowControl)

 public:
   Vmr9VideoWindowControl(QObject *parent = nullptr);
   ~Vmr9VideoWindowControl();

   IBaseFilter *filter() const {
      return m_filter;
   }

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

 private:
   void setProcAmpValues();
   float scaleProcAmpValue(
      IVMRMixerControl9 *control, VMR9ProcAmpControlFlags property, int value) const;

   IBaseFilter *m_filter;
   WId m_windowId;
   COLORREF m_windowColor;
   DWORD m_dirtyValues;
   Qt::AspectRatioMode m_aspectRatioMode;
   QRect m_displayRect;
   int m_brightness;
   int m_contrast;
   int m_hue;
   int m_saturation;
   bool m_fullScreen;
};

#endif
