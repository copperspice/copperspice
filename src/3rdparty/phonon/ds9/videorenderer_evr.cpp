/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/********************************************************
**  This file is part of the KDE project.
********************************************************/

#include "videorenderer_evr.h"
#include "qevr9.h"

#ifndef QT_NO_PHONON_VIDEO

#include <QtGui/QWidget>
#include <QtGui/QPainter>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        //we have to define them here because not all compilers/sdk have them
        static const GUID MR_VIDEO_RENDER_SERVICE =     {0x1092a86c, 0xab1a, 0x459a, {0xa3, 0x36, 0x83, 0x1f, 0xbc, 0x4d, 0x11, 0xff} };
        static const GUID MR_VIDEO_MIXER_SERVICE =      { 0x73cd2fc, 0x6cf4, 0x40b7, {0x88, 0x59, 0xe8, 0x95, 0x52, 0xc8, 0x41, 0xf8} };
        static const IID IID_IMFVideoDisplayControl =   {0xa490b1e4, 0xab84, 0x4d31, {0xa1, 0xb2, 0x18, 0x1e, 0x03, 0xb1, 0x07, 0x7a} };
        static const IID IID_IMFVideoMixerControl =     {0xA5C6C53F, 0xC202, 0x4aa5, {0x96, 0x95, 0x17, 0x5B, 0xA8, 0xC5, 0x08, 0xA5} };
        static const IID IID_IMFVideoProcessor =        {0x6AB0000C, 0xFECE, 0x4d1f, {0xA2, 0xAC, 0xA9, 0x57, 0x35, 0x30, 0x65, 0x6E} };
        static const IID IID_IMFGetService =            {0xFA993888, 0x4383, 0x415A, {0xA9, 0x30, 0xDD, 0x47, 0x2A, 0x8C, 0xF6, 0xF7} };
        static const GUID CLSID_EnhancedVideoRenderer = {0xfa10746c, 0x9b63, 0x4b6c, {0xbc, 0x49, 0xfc, 0x30,  0xe, 0xa5, 0xf2, 0x56} };

        template <typename T> ComPointer<T> getService(const Filter &filter, REFGUID guidService, REFIID riid)
        {
            //normally we should use IID_IMFGetService but this introduces another dependency
            //so here we simply define our own IId with the same value
            ComPointer<T> ret;
            ComPointer<IMFGetService> getService(filter, IID_IMFGetService);
            if (getService) {
                getService->GetService(guidService, riid, reinterpret_cast<void**>(ret.pparam()));
            }
            return ret;
        }

        VideoRendererEVR::~VideoRendererEVR()
        {
        }

        bool VideoRendererEVR::isNative() const
        {
            return true;
        }

        VideoRendererEVR::VideoRendererEVR(QWidget *target) : m_target(target)
        {
            if (QSysInfo::WindowsVersion < QSysInfo::WV_VISTA)
                return;
            m_filter = Filter(CLSID_EnhancedVideoRenderer, IID_IBaseFilter);
            if (!m_filter) {
                return;
            }

            ComPointer<IMFVideoDisplayControl> filterControl = getService<IMFVideoDisplayControl>(m_filter, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl);
            if (!filterControl ||
                FAILED(filterControl->SetVideoWindow(reinterpret_cast<HWND>(target->winId()))) ||
                FAILED(filterControl->SetAspectRatioMode(MFVideoARMode_None)) ||        // We're in control of the size
                !getService<IMFVideoMixerControl>(m_filter, MR_VIDEO_MIXER_SERVICE, IID_IMFVideoMixerControl) ||
                !getService<IMFVideoProcessor>(m_filter, MR_VIDEO_MIXER_SERVICE, IID_IMFVideoProcessor))  {
                m_filter = Filter(); //will release the interface
            }
        }

        QImage VideoRendererEVR::snapshot() const
        {
            // This will always capture black areas where no video is drawn, if any are present.
            // Due to the hack in notifyResize()
            ComPointer<IMFVideoDisplayControl> filterControl = getService<IMFVideoDisplayControl>(m_filter, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl);
            if (filterControl) {
                BITMAPINFOHEADER bmi;
                BYTE *buffer = 0;
                DWORD bufferSize;
                LONGLONG timeStamp;

                bmi.biSize = sizeof(BITMAPINFOHEADER);

                HRESULT hr = filterControl->GetCurrentImage(&bmi, &buffer, &bufferSize, &timeStamp);
                if (SUCCEEDED(hr)) {

                    const int w = qAbs(bmi.biWidth),
                        h = qAbs(bmi.biHeight);

                    // Create image and copy data into image.
                    QImage ret(w, h, QImage::Format_RGB32);

                    if (!ret.isNull()) {
                        uchar *data = buffer;
                        const int bytes_per_line = w * sizeof(QRgb);
                        for (int y = h - 1; y >= 0; --y) {
                            memcpy(ret.scanLine(y), //destination
                                data,     //source
                                bytes_per_line);
                            data += bytes_per_line;
                        }
                    }
                    ::CoTaskMemFree(buffer);
                    return ret;
                }
            }
            return QImage();
        }

        QSize VideoRendererEVR::videoSize() const
        {
            SIZE nativeSize;
            SIZE aspectRatioSize;

            ComPointer<IMFVideoDisplayControl> filterControl = getService<IMFVideoDisplayControl>(m_filter, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl);

            filterControl->GetNativeVideoSize(&nativeSize, &aspectRatioSize);

            return QSize(nativeSize.cx, nativeSize.cy);
        }
       
        void VideoRendererEVR::repaintCurrentFrame(QWidget *target, const QRect &rect)
        {
            // repaint the video
            ComPointer<IMFVideoDisplayControl> filterControl = getService<IMFVideoDisplayControl>(m_filter, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl);
            // All failed results can be safely ignored
            filterControl->RepaintVideo();
        }

        void VideoRendererEVR::notifyResize(const QSize &size, Phonon::VideoWidget::AspectRatio aspectRatio,
            Phonon::VideoWidget::ScaleMode scaleMode)
        {
            if (!isActive()) {
                RECT dummyRect = { 0, 0, 0, 0};
                ComPointer<IMFVideoDisplayControl> filterControl = getService<IMFVideoDisplayControl>(m_filter, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl);
                filterControl->SetVideoPosition(0, &dummyRect);
                return;
            }

            const QSize vsize = videoSize();
            internalNotifyResize(size, vsize, aspectRatio, scaleMode);

            RECT dstRectWin = { 0, 0, size.width(), size.height()};

            // Resize the Stream output rect instead of the destination rect.
            // Hacky workaround for flicker in the areas outside of the destination rect
            // This way these areas don't exist
            MFVideoNormalizedRect streamOutputRect = { float(m_dstX) / float(size.width()), float(m_dstY) / float(size.height()),
                                                       float(m_dstWidth + m_dstX) / float(size.width()), float(m_dstHeight + m_dstY) / float(size.height())};

            ComPointer<IMFVideoMixerControl> filterMixer = getService<IMFVideoMixerControl>(m_filter, MR_VIDEO_MIXER_SERVICE, IID_IMFVideoMixerControl);
            ComPointer<IMFVideoDisplayControl> filterControl = getService<IMFVideoDisplayControl>(m_filter, MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl);

            filterMixer->SetStreamOutputRect(0, &streamOutputRect);
            filterControl->SetVideoPosition(0, &dstRectWin);
        }

        void VideoRendererEVR::applyMixerSettings(qreal brightness, qreal contrast, qreal hue, qreal saturation)
        {
            InputPin sink = BackendNode::pins(m_filter, PINDIR_INPUT).first();
            OutputPin source;
            if (FAILED(sink->ConnectedTo(source.pparam()))) {
                return; //it must be connected to work
            }

            // Get the "Video Processor" (used for brightness/contrast/saturation/hue)
            ComPointer<IMFVideoProcessor> processor = getService<IMFVideoProcessor>(m_filter, MR_VIDEO_MIXER_SERVICE, IID_IMFVideoProcessor);
            Q_ASSERT(processor);

            DXVA2_ValueRange contrastRange;
            DXVA2_ValueRange brightnessRange;
            DXVA2_ValueRange saturationRange;
            DXVA2_ValueRange hueRange;

            if (FAILED(processor->GetProcAmpRange(DXVA2_ProcAmp_Contrast, &contrastRange)))
                return;
            if (FAILED(processor->GetProcAmpRange(DXVA2_ProcAmp_Brightness, &brightnessRange)))
                return;
            if (FAILED(processor->GetProcAmpRange(DXVA2_ProcAmp_Saturation, &saturationRange)))
                return;
            if (FAILED(processor->GetProcAmpRange(DXVA2_ProcAmp_Hue, &hueRange)))
                return;

            DXVA2_ProcAmpValues values;

            values.Contrast = DXVA2FloatToFixed(((contrast < 0
                                ? DXVA2FixedToFloat(contrastRange.MinValue) : DXVA2FixedToFloat(contrastRange.MaxValue))
                               - DXVA2FixedToFloat(contrastRange.DefaultValue)) * qAbs(contrast) + DXVA2FixedToFloat(contrastRange.DefaultValue));
            values.Brightness = DXVA2FloatToFixed(((brightness < 0
                                ? DXVA2FixedToFloat(brightnessRange.MinValue) : DXVA2FixedToFloat(brightnessRange.MaxValue))
                               - DXVA2FixedToFloat(brightnessRange.DefaultValue)) * qAbs(brightness) + DXVA2FixedToFloat(brightnessRange.DefaultValue));
            values.Saturation = DXVA2FloatToFixed(((saturation < 0
                                ? DXVA2FixedToFloat(saturationRange.MinValue) : DXVA2FixedToFloat(saturationRange.MaxValue))
                               - DXVA2FixedToFloat(saturationRange.DefaultValue)) * qAbs(saturation) + DXVA2FixedToFloat(saturationRange.DefaultValue));
            values.Hue = DXVA2FloatToFixed(((hue < 0
                                ? DXVA2FixedToFloat(hueRange.MinValue) : DXVA2FixedToFloat(hueRange.MaxValue))
                               - DXVA2FixedToFloat(hueRange.DefaultValue)) * qAbs(hue) + DXVA2FixedToFloat(hueRange.DefaultValue));

            //finally set the settings
            processor->SetProcAmpValues(DXVA2_ProcAmp_Contrast | DXVA2_ProcAmp_Brightness | DXVA2_ProcAmp_Saturation | DXVA2_ProcAmp_Hue, &values);

        }
    }
}

QT_END_NAMESPACE

#endif //QT_NO_PHONON_VIDEO
