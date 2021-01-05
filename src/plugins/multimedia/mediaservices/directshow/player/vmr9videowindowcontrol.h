/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

   WId winId() const;
   void setWinId(WId id);

   QRect displayRect() const;
   void setDisplayRect(const QRect &rect);

   bool isFullScreen() const;
   void setFullScreen(bool fullScreen);

   void repaint();

   QSize nativeSize() const;

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
