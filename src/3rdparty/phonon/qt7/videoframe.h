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

#ifndef QT7_VIDEOFRAME_H
#define QT7_VIDEOFRAME_H

#import <QuartzCore/CVOpenGLTexture.h>
#import <AppKit/NSImage.h>
#undef check // avoid name clash;

#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QImage>

QT_BEGIN_NAMESPACE

class QGLContext;

namespace Phonon
{
namespace QT7
{
    class QuickTimeVideoPlayer;
	class QNSBitmapImage;

    class VideoFrame
    {
        public:
            VideoFrame();
            VideoFrame(QuickTimeVideoPlayer *videoPlayer);
            VideoFrame(const VideoFrame& frame);
            void operator=(const VideoFrame& frame);
            ~VideoFrame();


            CVOpenGLTextureRef cachedCVTexture() const;
			void *cachedCIImage() const;
            GLuint glTextureRef() const;

			void drawQImage(QPainter *p, const QRect &rect) const;
			void drawCIImage(const CGRect &rect, float opacity = 1.0f) const;
			void drawCIImage(const QRect &rect, float opacity = 1.0f) const;
            void drawCVTexture(const QRect &rect, float opacity = 1.0f) const;
            void drawGLTexture(const QRect &rect, float opacity = 1.0f) const;

            void applyCoreImageFilter(void *filter);
            void setColors(qreal brightness, qreal contrast, qreal hue, qreal saturation);
			bool hasColorAdjustments();
            void setBaseOpacity(qreal opacity);
            void setBackgroundFrame(const VideoFrame &frame);

            bool isEmpty();
            QRect frameRect() const;
            QuickTimeVideoPlayer *videoPlayer();

            void retain() const;
            void release() const;

			static CGRect QRectToCGRect(const QRect & qrect);

        private:
            CVOpenGLTextureRef m_cachedCVTextureRef;
            void *m_cachedCIImage;
			QImage m_cachedQImage;
            NSBitmapImageRep *m_cachedNSBitmap;

            QuickTimeVideoPlayer *m_videoPlayer;
            VideoFrame *m_backgroundFrame;

            qreal m_brightness;
            qreal m_contrast;
            qreal m_hue;
            qreal m_saturation;
            qreal m_opacity;

            void initMembers();
            void copyMembers(const VideoFrame& frame);
            void invalidateImage() const;
    };

}} //namespace Phonon::QT7

QT_END_NAMESPACE
#endif // Phonon_QT7_VIDEOFRAME_H
