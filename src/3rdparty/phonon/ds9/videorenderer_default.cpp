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

#include "videorenderer_default.h"

#ifndef QT_NO_PHONON_VIDEO

#include <QtGui/QWidget>
#include <QtGui/QPainter>

#include <uuids.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        VideoRendererDefault::~VideoRendererDefault()
        {
        }

        bool VideoRendererDefault::isNative() const
        {
            return true;
        }


        VideoRendererDefault::VideoRendererDefault(QWidget *target) : m_target(target)
        {
            m_target->setAttribute(Qt::WA_PaintOnScreen, true);
            m_filter = Filter(CLSID_VideoRenderer, IID_IBaseFilter);
        }

        QSize VideoRendererDefault::videoSize() const
        {
            LONG w = 0,
                h = 0;
            ComPointer<IBasicVideo> basic(m_filter, IID_IBasicVideo);
            if (basic) {
                basic->GetVideoSize( &w, &h);
            }
            return QSize(w, h);
        }

        void VideoRendererDefault::repaintCurrentFrame(QWidget * /*target*/, const QRect & /*rect*/)
        {
            //nothing to do here: the renderer paints everything
        }

        void VideoRendererDefault::notifyResize(const QSize &size, Phonon::VideoWidget::AspectRatio aspectRatio,
            Phonon::VideoWidget::ScaleMode scaleMode)
        {
            if (!isActive()) {
                ComPointer<IBasicVideo> basic(m_filter, IID_IBasicVideo);
                if (basic) {
                    basic->SetDestinationPosition(0, 0, 0, 0);
                }
                return;
            }

            ComPointer<IVideoWindow> video(m_filter, IID_IVideoWindow);

            OAHWND owner;
            HRESULT hr = video->get_Owner(&owner);
            if (FAILED(hr)) {
                return;
            }

            const OAHWND newOwner = reinterpret_cast<OAHWND>(m_target->winId());
            if (owner != newOwner) {
                video->put_Owner(newOwner);
                video->put_MessageDrain(newOwner);
                video->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
            }

            //make sure the widget takes the whole size of the parent
            video->SetWindowPosition(0, 0, size.width(), size.height());

            const QSize vsize = videoSize();
            internalNotifyResize(size, vsize, aspectRatio, scaleMode);

            ComPointer<IBasicVideo> basic(m_filter, IID_IBasicVideo);
            if (basic) {
                basic->SetDestinationPosition(m_dstX, m_dstY, m_dstWidth, m_dstHeight);
            }
        }

        void VideoRendererDefault::applyMixerSettings(qreal /*brightness*/, qreal /*contrast*/, qreal /*m_hue*/, qreal /*saturation*/)
        {
            //this can't be supported for the default renderer
        }

        QImage VideoRendererDefault::snapshot() const
        {
            ComPointer<IBasicVideo> basic(m_filter, IID_IBasicVideo);
            if (basic) {
                LONG bufferSize = 0;
                //1st we get the buffer size
                basic->GetCurrentImage(&bufferSize, 0);

                QByteArray buffer;
                buffer.resize(bufferSize);
                HRESULT hr = basic->GetCurrentImage(&bufferSize, reinterpret_cast<long*>(buffer.data()));

                if (SUCCEEDED(hr)) {

                    const BITMAPINFOHEADER  *bmi = reinterpret_cast<const BITMAPINFOHEADER*>(buffer.constData());

                    const int w = qAbs(bmi->biWidth),
                        h = qAbs(bmi->biHeight);

                    // Create image and copy data into image.
                    QImage ret(w, h, QImage::Format_RGB32);

                    if (!ret.isNull()) {
                        const char *data = buffer.constData() + bmi->biSize;
                        const int bytes_per_line = w * sizeof(QRgb);
                        for (int y = h - 1; y >= 0; --y) {
                            qMemCopy(ret.scanLine(y), //destination
                                data,     //source
                                bytes_per_line);
                            data += bytes_per_line;
                        }
                    }
                    return ret;
                }
            }
            return QImage();
        }

    }
}

QT_END_NAMESPACE

#endif //QT_NO_PHONON_VIDEO
