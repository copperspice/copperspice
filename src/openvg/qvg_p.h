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

#ifndef QVG_P_H
#define QVG_P_H

#include "qvg.h"

// vgDrawGlyphs() only exists in OpenVG 1.1 and higher.
#if !defined(OPENVG_VERSION_1_1) && !defined(QVG_NO_DRAW_GLYPHS)
#define QVG_NO_DRAW_GLYPHS 1
#endif

#include <QtGui/qimage.h>

#if !defined(QT_NO_EGL)
#include <qeglcontext_p.h>
#endif

QT_BEGIN_NAMESPACE

class QVGPaintEngine;

#if !defined(QT_NO_EGL)

class QEglContext;

// Create an EGL context, but don't bind it to a surface.  If single-context
// mode is enabled, this will return the previously-created context.
// "devType" indicates the type of device using the context, usually
// QInternal::Widget or QInternal::Pixmap.
Q_OPENVG_EXPORT QEglContext *qt_vg_create_context
    (QPaintDevice *device, int devType);

// Destroy an EGL context that was created by qt_vg_create_context().
// If single-context mode is enabled, this will decrease the reference count.
// "devType" indicates the type of device destroying the context, usually
// QInternal::Widget or QInternal::Pixmap.
Q_OPENVG_EXPORT void qt_vg_destroy_context
    (QEglContext *context, int devType);

// Return the shared pbuffer surface that can be made current to
// destroy VGImage objects when there is no other surface available.
Q_OPENVG_EXPORT EGLSurface qt_vg_shared_surface(void);

// Convert the configuration format in a context to a VG or QImage format.
Q_OPENVG_EXPORT VGImageFormat qt_vg_config_to_vg_format(QEglContext *context);
Q_OPENVG_EXPORT QImage::Format qt_vg_config_to_image_format(QEglContext *context);

#endif

// Create a paint engine.  Returns the common engine in single-context mode.
Q_OPENVG_EXPORT QVGPaintEngine *qt_vg_create_paint_engine(void);

// Destroy a paint engine.  Does nothing in single-context mode.
Q_OPENVG_EXPORT void qt_vg_destroy_paint_engine(QVGPaintEngine *engine);

// Convert between QImage and VGImage format values.
Q_OPENVG_EXPORT VGImageFormat qt_vg_image_to_vg_format(QImage::Format format);

QT_END_NAMESPACE

#endif
