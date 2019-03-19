/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qnsview.h"

#include <QtGui/QWindowSystemInterface>

#include <QtCore/QDebug>

@implementation QNSView

- (id) init
{
    self = [super init];
    if (self) {
        m_cgImage = 0;
        m_widget = 0;
        m_buttons = Qt::NoButton;
    }
    return self;
}

- (id)initWithWidget:(QWidget *)widget {
    self = [self init];
    if (self) {
        m_widget = widget;
    }
    return self;
}

- (void) setImage:(QImage *)image
{
    CGImageRelease(m_cgImage);

    const uchar *imageData = image->bits();
    int bitDepth = image->depth();
    int colorBufferSize = 8;
    int bytesPrLine = image->bytesPerLine();
    int width = image->width();
    int height = image->height();

    CGColorSpaceRef cgColourSpaceRef = CGColorSpaceCreateDeviceRGB();

    CGDataProviderRef cgDataProviderRef = CGDataProviderCreateWithData(
                NULL,
                imageData,
                image->byteCount(),
                NULL);

    m_cgImage = CGImageCreate(width,
                              height,
                              colorBufferSize,
                              bitDepth,
                              bytesPrLine,
                              cgColourSpaceRef,
                              kCGImageAlphaNone,
                              cgDataProviderRef,
                              NULL,
                              false,
                              kCGRenderingIntentDefault);

    CGColorSpaceRelease(cgColourSpaceRef);

}

- (void) drawRect:(NSRect)dirtyRect
{
    if (!m_cgImage)
        return;

    CGRect dirtyCGRect = NSRectToCGRect(dirtyRect);

    NSGraphicsContext *nsGraphicsContext = [NSGraphicsContext currentContext];
    CGContextRef cgContext = (CGContextRef) [nsGraphicsContext graphicsPort];

    CGContextSaveGState( cgContext );
    int dy = dirtyCGRect.origin.y + CGRectGetMaxY(dirtyCGRect);
    CGContextTranslateCTM(cgContext, 0, dy);
    CGContextScaleCTM(cgContext, 1, -1);

    CGImageRef subImage = CGImageCreateWithImageInRect(m_cgImage, dirtyCGRect);
    CGContextDrawImage(cgContext,dirtyCGRect,subImage);

    CGContextRestoreGState(cgContext);

    CGImageRelease(subImage);

}

- (BOOL) isFlipped
{
    return YES;
}

- (void)handleMouseEvent:(NSEvent *)theEvent;
{
    NSPoint point = [self convertPoint: [theEvent locationInWindow] fromView: nil];
    QPoint qt_localPoint(point.x,point.y);

    NSTimeInterval timestamp = [theEvent timestamp];
    ulong qt_timestamp = timestamp * 1000;

    QWindowSystemInterface::handleMouseEvent(m_widget,qt_timestamp,qt_localPoint,QPoint(),m_buttons);

}
    - (void)mouseDown:(NSEvent *)theEvent
    {
        m_buttons |= Qt::LeftButton;
        [self handleMouseEvent:theEvent];
    }
    - (void)mouseDragged:(NSEvent *)theEvent
    {
        if (!(m_buttons & Qt::LeftButton))
            qWarning("Internal Mousebutton tracking invalid(missing Qt::LeftButton");
        [self handleMouseEvent:theEvent];
    }
    - (void)mouseUp:(NSEvent *)theEvent
    {
        m_buttons &= QFlag(~int(Qt::LeftButton));
        [self handleMouseEvent:theEvent];
    }

- (void)mouseMoved:(NSEvent *)theEvent
{
    qDebug() << "mouseMove";
    [self handleMouseEvent:theEvent];
}
- (void)mouseEntered:(NSEvent *)theEvent
{
        Q_UNUSED(theEvent);
        QWindowSystemInterface::handleEnterEvent(m_widget);
}
- (void)mouseExited:(NSEvent *)theEvent
{
        Q_UNUSED(theEvent);
        QWindowSystemInterface::handleLeaveEvent(m_widget);
}
- (void)rightMouseDown:(NSEvent *)theEvent
{
        m_buttons |= Qt::RightButton;
    [self handleMouseEvent:theEvent];
}
- (void)rightMouseDragged:(NSEvent *)theEvent
{
        if (!(m_buttons & Qt::LeftButton))
            qWarning("Internal Mousebutton tracking invalid(missing Qt::LeftButton");
        [self handleMouseEvent:theEvent];
}
- (void)rightMouseUp:(NSEvent *)theEvent
{
        m_buttons &= QFlag(~int(Qt::RightButton));
        [self handleMouseEvent:theEvent];
}
- (void)otherMouseDown:(NSEvent *)theEvent
{
        m_buttons |= Qt::RightButton;
    [self handleMouseEvent:theEvent];
}
- (void)otherMouseDragged:(NSEvent *)theEvent
{
        if (!(m_buttons & Qt::LeftButton))
            qWarning("Internal Mousebutton tracking invalid(missing Qt::LeftButton");
        [self handleMouseEvent:theEvent];
}
- (void)otherMouseUp:(NSEvent *)theEvent
{
        m_buttons &= QFlag(~int(Qt::MiddleButton));
        [self handleMouseEvent:theEvent];
}



@end
