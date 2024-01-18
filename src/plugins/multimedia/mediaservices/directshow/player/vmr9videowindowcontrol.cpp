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

#include <vmr9videowindowcontrol.h>

#include <qpalette.h>
#include <qwidget.h>

#include <dsplayer_global.h>

Vmr9VideoWindowControl::Vmr9VideoWindowControl(QObject *parent)
   : QVideoWindowControl(parent)
   , m_filter(com_new<IBaseFilter>(CLSID_VideoMixingRenderer9))
   , m_windowId(0)
   , m_windowColor(RGB(0, 0, 0))
   , m_dirtyValues(0)
   , m_aspectRatioMode(Qt::KeepAspectRatio)
   , m_brightness(0)
   , m_contrast(0)
   , m_hue(0)
   , m_saturation(0)
   , m_fullScreen(false)
{
   if (IVMRFilterConfig9 *config = com_cast<IVMRFilterConfig9>(m_filter, IID_IVMRFilterConfig9)) {
      config->SetRenderingMode(VMR9Mode_Windowless);
      config->SetNumberOfStreams(1);
      config->Release();
   }
}

Vmr9VideoWindowControl::~Vmr9VideoWindowControl()
{
   if (m_filter) {
      m_filter->Release();
   }
}


WId Vmr9VideoWindowControl::winId() const
{
   return m_windowId;

}

void Vmr9VideoWindowControl::setWinId(WId id)
{
   m_windowId = id;

   if (QWidget *widget = QWidget::find(m_windowId)) {
      const QColor color = widget->palette().color(QPalette::Window);

      m_windowColor = RGB(color.red(), color.green(), color.blue());
   }

   if (IVMRWindowlessControl9 *control = com_cast<IVMRWindowlessControl9>(m_filter, IID_IVMRWindowlessControl9)) {
      control->SetVideoClippingWindow(reinterpret_cast<HWND>(m_windowId));
      control->SetBorderColor(m_windowColor);
      control->Release();
   }
}

QRect Vmr9VideoWindowControl::displayRect() const
{
   return m_displayRect;
}

void Vmr9VideoWindowControl::setDisplayRect(const QRect &rect)
{
   m_displayRect = rect;

   if (IVMRWindowlessControl9 *control = com_cast<IVMRWindowlessControl9>(
            m_filter, IID_IVMRWindowlessControl9)) {
      RECT sourceRect = { 0, 0, 0, 0 };
      RECT displayRect = { rect.left(), rect.top(), rect.right() + 1, rect.bottom() + 1 };

      control->GetNativeVideoSize(&sourceRect.right, &sourceRect.bottom, nullptr, nullptr);

      if (m_aspectRatioMode == Qt::KeepAspectRatioByExpanding) {
         QSize clippedSize = rect.size();
         clippedSize.scale(sourceRect.right, sourceRect.bottom, Qt::KeepAspectRatio);

         sourceRect.left = (sourceRect.right - clippedSize.width()) / 2;
         sourceRect.top = (sourceRect.bottom - clippedSize.height()) / 2;
         sourceRect.right = sourceRect.left + clippedSize.width();
         sourceRect.bottom = sourceRect.top + clippedSize.height();
      }

      control->SetVideoPosition(&sourceRect, &displayRect);
      control->Release();
   }
}

bool Vmr9VideoWindowControl::isFullScreen() const
{
   return m_fullScreen;
}

void Vmr9VideoWindowControl::setFullScreen(bool fullScreen)
{
   emit fullScreenChanged(m_fullScreen = fullScreen);
}

void Vmr9VideoWindowControl::repaint()
{
   PAINTSTRUCT paint;

   if (HDC dc = ::BeginPaint(reinterpret_cast<HWND>(m_windowId), &paint)) {
      HRESULT hr = E_FAIL;

      if (IVMRWindowlessControl9 *control = com_cast<IVMRWindowlessControl9>(
               m_filter, IID_IVMRWindowlessControl9)) {
         hr = control->RepaintVideo(reinterpret_cast<HWND>(m_windowId), dc);
         control->Release();
      }

      if (!SUCCEEDED(hr)) {
         HPEN pen = ::CreatePen(PS_SOLID, 1, m_windowColor);
         HBRUSH brush = ::CreateSolidBrush(m_windowColor);
         ::SelectObject(dc, pen);
         ::SelectObject(dc, brush);

         ::Rectangle(
            dc,
            m_displayRect.left(),
            m_displayRect.top(),
            m_displayRect.right() + 1,
            m_displayRect.bottom() + 1);

         ::DeleteObject(pen);
         ::DeleteObject(brush);
      }
      ::EndPaint(reinterpret_cast<HWND>(m_windowId), &paint);
   }
}

QSize Vmr9VideoWindowControl::nativeSize() const
{
   QSize size;

   if (IVMRWindowlessControl9 *control = com_cast<IVMRWindowlessControl9>(
            m_filter, IID_IVMRWindowlessControl9)) {
      LONG width;
      LONG height;

      if (control->GetNativeVideoSize(&width, &height, nullptr, nullptr) == S_OK) {
         size = QSize(width, height);
      }
      control->Release();
   }
   return size;
}

Qt::AspectRatioMode Vmr9VideoWindowControl::aspectRatioMode() const
{
   return m_aspectRatioMode;
}

void Vmr9VideoWindowControl::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   m_aspectRatioMode = mode;

   if (IVMRWindowlessControl9 *control = com_cast<IVMRWindowlessControl9>(
            m_filter, IID_IVMRWindowlessControl9)) {
      switch (mode) {
         case Qt::IgnoreAspectRatio:
            control->SetAspectRatioMode(VMR9ARMode_None);
            break;

         case Qt::KeepAspectRatio:
            control->SetAspectRatioMode(VMR9ARMode_LetterBox);
            break;

         case Qt::KeepAspectRatioByExpanding:
            control->SetAspectRatioMode(VMR9ARMode_LetterBox);
            setDisplayRect(m_displayRect);
            break;

         default:
            break;
      }
      control->Release();
   }
}

int Vmr9VideoWindowControl::brightness() const
{
   return m_brightness;
}

void Vmr9VideoWindowControl::setBrightness(int brightness)
{
   m_brightness = brightness;

   m_dirtyValues |= ProcAmpControl9_Brightness;

   setProcAmpValues();

   emit brightnessChanged(brightness);
}

int Vmr9VideoWindowControl::contrast() const
{
   return m_contrast;
}

void Vmr9VideoWindowControl::setContrast(int contrast)
{
   m_contrast = contrast;

   m_dirtyValues |= ProcAmpControl9_Contrast;

   setProcAmpValues();

   emit contrastChanged(contrast);
}

int Vmr9VideoWindowControl::hue() const
{
   return m_hue;
}

void Vmr9VideoWindowControl::setHue(int hue)
{
   m_hue = hue;

   m_dirtyValues |= ProcAmpControl9_Hue;

   setProcAmpValues();

   emit hueChanged(hue);
}

int Vmr9VideoWindowControl::saturation() const
{
   return m_saturation;
}

void Vmr9VideoWindowControl::setSaturation(int saturation)
{
   m_saturation = saturation;

   m_dirtyValues |= ProcAmpControl9_Saturation;

   setProcAmpValues();

   emit saturationChanged(saturation);
}

void Vmr9VideoWindowControl::setProcAmpValues()
{
   if (IVMRMixerControl9 *control = com_cast<IVMRMixerControl9>(m_filter, IID_IVMRMixerControl9)) {
      VMR9ProcAmpControl procAmp;
      procAmp.dwSize = sizeof(VMR9ProcAmpControl);
      procAmp.dwFlags = m_dirtyValues;

      if (m_dirtyValues & ProcAmpControl9_Brightness) {
         procAmp.Brightness = scaleProcAmpValue(
               control, ProcAmpControl9_Brightness, m_brightness);
      }

      if (m_dirtyValues & ProcAmpControl9_Contrast) {
         procAmp.Contrast = scaleProcAmpValue(
               control, ProcAmpControl9_Contrast, m_contrast);
      }

      if (m_dirtyValues & ProcAmpControl9_Hue) {
         procAmp.Hue = scaleProcAmpValue(
               control, ProcAmpControl9_Hue, m_hue);
      }

      if (m_dirtyValues & ProcAmpControl9_Saturation) {
         procAmp.Saturation = scaleProcAmpValue(
               control, ProcAmpControl9_Saturation, m_saturation);
      }

      if (SUCCEEDED(control->SetProcAmpControl(0, &procAmp))) {
         m_dirtyValues = 0;
      }

      control->Release();
   }
}

float Vmr9VideoWindowControl::scaleProcAmpValue(
   IVMRMixerControl9 *control, VMR9ProcAmpControlFlags property, int value) const
{
   float scaledValue = 0.0;

   VMR9ProcAmpControlRange range;
   range.dwSize = sizeof(VMR9ProcAmpControlRange);
   range.dwProperty = property;

   if (SUCCEEDED(control->GetProcAmpControlRange(0, &range))) {
      scaledValue = range.DefaultValue;
      if (value > 0) {
         scaledValue += float(value) * (range.MaxValue - range.DefaultValue) / 100;
      } else if (value < 0) {
         scaledValue -= float(value) * (range.MinValue - range.DefaultValue) / 100;
      }
   }

   return scaledValue;
}
