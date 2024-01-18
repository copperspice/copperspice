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

#include "avfcameraviewfindersettingscontrol.h"
#include "avfcamerarenderercontrol.h"
#include "avfcamerasession.h"
#include "avfcameraservice.h"
#include "avfcameradebug.h"
#include <qabstractvideosurface.h>
#include <qabstractvideobuffer.h>
#include <qvideosurfaceformat.h>

#include <qabstractvideobuffer_p.h>

#ifdef Q_OS_IOS
#include <qopengl.h>
#endif

class CVImageVideoBuffer : public QAbstractPlanarVideoBuffer
{
public:
    CVImageVideoBuffer(CVImageBufferRef buffer, AVFCameraRendererControl *renderer)
        : QAbstractPlanarVideoBuffer(NoHandle), m_buffer(buffer), m_mode(NotMapped)
    {

        (void) renderer;
        CVPixelBufferRetain(m_buffer);
    }

    ~CVImageVideoBuffer()
    {
        CVImageVideoBuffer::unmap();
        CVPixelBufferRelease(m_buffer);
    }

    MapMode mapMode() const { return m_mode; }

    int map(QAbstractVideoBuffer::MapMode mode, int *numBytes, int bytesPerLine[4], uchar *data[4])
    {
        // We only support RGBA or NV12 (or Apple's version of NV12),
        // they are either 0 planes or 2.
        const size_t nPlanes = CVPixelBufferGetPlaneCount(m_buffer);
        Q_ASSERT(nPlanes <= 2);

        if (!nPlanes) {
            data[0] = map(mode, numBytes, bytesPerLine);
            return data[0] ? 1 : 0;
        }

        // For a bi-planar format we have to set the parameters correctly:
        if (mode != QAbstractVideoBuffer::NotMapped && m_mode == QAbstractVideoBuffer::NotMapped) {
            CVPixelBufferLockBaseAddress(m_buffer, mode == QAbstractVideoBuffer::ReadOnly
                                                               ? kCVPixelBufferLock_ReadOnly
                                                               : 0);

            if (numBytes)
                *numBytes = CVPixelBufferGetDataSize(m_buffer);

            if (bytesPerLine) {
                // At the moment we handle only bi-planar format.
                bytesPerLine[0] = CVPixelBufferGetBytesPerRowOfPlane(m_buffer, 0);
                bytesPerLine[1] = CVPixelBufferGetBytesPerRowOfPlane(m_buffer, 1);
            }

            if (data) {
                data[0] = (uchar *)CVPixelBufferGetBaseAddressOfPlane(m_buffer, 0);
                data[1] = (uchar *)CVPixelBufferGetBaseAddressOfPlane(m_buffer, 1);
            }

            m_mode = mode;
        }

        return nPlanes;
    }

    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine)
    {
        if (mode != NotMapped && m_mode == NotMapped) {
            CVPixelBufferLockBaseAddress(m_buffer, mode == QAbstractVideoBuffer::ReadOnly
                                                               ? kCVPixelBufferLock_ReadOnly
                                                               : 0);
            if (numBytes)
                *numBytes = CVPixelBufferGetDataSize(m_buffer);

            if (bytesPerLine)
                *bytesPerLine = CVPixelBufferGetBytesPerRow(m_buffer);

            m_mode = mode;
            return (uchar*)CVPixelBufferGetBaseAddress(m_buffer);
        } else {
            return nullptr;
        }
    }

    void unmap()
    {
        if (m_mode != NotMapped) {
            CVPixelBufferUnlockBaseAddress(m_buffer, m_mode == QAbstractVideoBuffer::ReadOnly
                                                                   ? kCVPixelBufferLock_ReadOnly
                                                                   : 0);
            m_mode = NotMapped;
        }
    }

    QVariant handle() const
    {
#ifdef Q_OS_IOS
        // Called from the render thread, so there is a current OpenGL context

        if (!m_renderer->m_textureCache) {
            CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                                        NULL,
                                                        [EAGLContext currentContext],
                                                        NULL,
                                                        &m_renderer->m_textureCache);

            if (err != kCVReturnSuccess)
                qWarning("Error creating texture cache");
        }

        if (m_renderer->m_textureCache && !m_texture) {
            CVOpenGLESTextureCacheFlush(m_renderer->m_textureCache, 0);

            CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                        m_renderer->m_textureCache,
                                                                        m_buffer,
                                                                        NULL,
                                                                        GL_TEXTURE_2D,
                                                                        GL_RGBA,
                                                                        CVPixelBufferGetWidth(m_buffer),
                                                                        CVPixelBufferGetHeight(m_buffer),
                                                                        GL_RGBA,
                                                                        GL_UNSIGNED_BYTE,
                                                                        0,
                                                                        &m_texture);
            if (err != kCVReturnSuccess)
                qWarning("Error creating texture from buffer");
        }

        if (m_texture)
            return CVOpenGLESTextureGetName(m_texture);
        else
            return 0;
#else
        return QVariant();
#endif
    }

private:
#ifdef Q_OS_IOS
    mutable CVOpenGLESTextureRef m_texture;
    AVFCameraRendererControl *m_renderer;
#endif
    CVImageBufferRef m_buffer;
    MapMode m_mode;
};


@interface AVFCaptureFramesDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
{
@private
    AVFCameraRendererControl *m_renderer;
}

- (AVFCaptureFramesDelegate *) initWithRenderer:(AVFCameraRendererControl*)renderer;

- (void) captureOutput:(AVCaptureOutput *)captureOutput
         didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection *)connection;

@end

@implementation AVFCaptureFramesDelegate

- (AVFCaptureFramesDelegate *) initWithRenderer:(AVFCameraRendererControl*)renderer
{
    if (!(self = [super init]))
        return nil;

    self->m_renderer = renderer;
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput
         didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection *)connection
{
    (void) connection;
    (void) captureOutput;

    // NB: on iOS captureOutput/connection can be nil (when recording a video -
    // avfmediaassetwriter).

    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);

    int width = CVPixelBufferGetWidth(imageBuffer);
    int height = CVPixelBufferGetHeight(imageBuffer);
    QVideoFrame::PixelFormat format =
            AVFCameraViewfinderSettingsControl2::QtPixelFormatFromCVFormat(CVPixelBufferGetPixelFormatType(imageBuffer));
    if (format == QVideoFrame::Format_Invalid)
        return;

    QVideoFrame frame(new CVImageVideoBuffer(imageBuffer, m_renderer),
                      QSize(width, height),
                      format);

    m_renderer->syncHandleViewfinderFrame(frame);
}

@end


AVFCameraRendererControl::AVFCameraRendererControl(QObject *parent)
   : QVideoRendererControl(parent), m_surface(nullptr), m_supportsTextures(false),
     m_needsHorizontalMirroring(false)
#ifdef Q_OS_IOS
   , m_textureCache(0)
#endif
{
    m_viewfinderFramesDelegate = [[AVFCaptureFramesDelegate alloc] initWithRenderer:this];
}

AVFCameraRendererControl::~AVFCameraRendererControl()
{
    [m_cameraSession->captureSession() removeOutput:m_videoDataOutput];
    [m_viewfinderFramesDelegate release];

    if (m_delegateQueue)
        dispatch_release(m_delegateQueue);
#ifdef Q_OS_IOS
    if (m_textureCache)
        CFRelease(m_textureCache);
#endif
}

QAbstractVideoSurface *AVFCameraRendererControl::surface() const
{
    return m_surface;
}

void AVFCameraRendererControl::setSurface(QAbstractVideoSurface *surface)
{
    if (m_surface != surface) {
        m_surface = surface;
#ifdef Q_OS_IOS
        m_supportsTextures = m_surface
                ? m_surface->supportedPixelFormats(QAbstractVideoBuffer::GLTextureHandle).contains(QVideoFrame::Format_BGRA32)
                : false;
#endif
        Q_EMIT surfaceChanged(surface);
    }
}

void AVFCameraRendererControl::configureAVCaptureSession(AVFCameraSession *cameraSession)
{
    m_cameraSession = cameraSession;
    connect(m_cameraSession, SIGNAL(readyToConfigureConnections()),
            this, SLOT(updateCaptureConnection()));

    m_needsHorizontalMirroring = false;

    m_videoDataOutput = [[[AVCaptureVideoDataOutput alloc] init] autorelease];

    // Configure video output
    m_delegateQueue = dispatch_queue_create("vf_queue", nullptr);

    [m_videoDataOutput
            setSampleBufferDelegate:m_viewfinderFramesDelegate
            queue:m_delegateQueue];

    [m_cameraSession->captureSession() addOutput:m_videoDataOutput];
}

void AVFCameraRendererControl::updateCaptureConnection()
{
    AVCaptureConnection *connection = [m_videoDataOutput connectionWithMediaType:AVMediaTypeVideo];
    if (connection == nil || !m_cameraSession->videoCaptureDevice())
        return;

    // Frames of front-facing cameras should be mirrored horizontally (it's the default when using
    // AVCaptureVideoPreviewLayer but not with AVCaptureVideoDataOutput)
    if (connection.isVideoMirroringSupported)
        connection.videoMirrored = m_cameraSession->videoCaptureDevice().position == AVCaptureDevicePositionFront;

    // If the connection does't support mirroring, we'll have to do it ourselves
    m_needsHorizontalMirroring = !connection.isVideoMirrored
            && m_cameraSession->videoCaptureDevice().position == AVCaptureDevicePositionFront;
}

//can be called from non main thread
void AVFCameraRendererControl::syncHandleViewfinderFrame(const QVideoFrame &frame)
{
    QMutexLocker lock(&m_vfMutex);
    if (!m_lastViewfinderFrame.isValid()) {
        static QMetaMethod handleViewfinderFrameSlot = metaObject()->method(
                    metaObject()->indexOfMethod("handleViewfinderFrame()"));

        handleViewfinderFrameSlot.invoke(this, Qt::QueuedConnection);
    }

    m_lastViewfinderFrame = frame;

    if (m_cameraSession && m_lastViewfinderFrame.isValid())
        m_cameraSession->onCameraFrameFetched(m_lastViewfinderFrame);
}

AVCaptureVideoDataOutput *AVFCameraRendererControl::videoDataOutput() const
{
    return m_videoDataOutput;
}

#ifdef Q_OS_IOS

AVFCaptureFramesDelegate *AVFCameraRendererControl::captureDelegate() const
{
    return m_viewfinderFramesDelegate;
}

void AVFCameraRendererControl::resetCaptureDelegate() const
{
    [m_videoDataOutput setSampleBufferDelegate:m_viewfinderFramesDelegate queue:m_delegateQueue];
}

#endif

void AVFCameraRendererControl::handleViewfinderFrame()
{
    QVideoFrame frame;
    {
        QMutexLocker lock(&m_vfMutex);
        frame = m_lastViewfinderFrame;
        m_lastViewfinderFrame = QVideoFrame();
    }

    if (m_surface && frame.isValid()) {
        if (m_surface->isActive() && (m_surface->surfaceFormat().pixelFormat() != frame.pixelFormat()
                                      || m_surface->surfaceFormat().frameSize() != frame.size())) {
            m_surface->stop();
        }

        if (!m_surface->isActive()) {
            QVideoSurfaceFormat format(frame.size(), frame.pixelFormat(), frame.handleType());
            if (m_needsHorizontalMirroring)
                format.setProperty(make_view(QString("mirrored")), true);

            if (! m_surface->start(format)) {
                qWarning() << "Failed to start viewfinder m_surface, format:" << format;

            } else {
                qDebugCamera() << "Viewfinder started: " << format;
            }
        }

        if (m_surface->isActive())
            m_surface->present(frame);
    }
}

