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

#ifndef QDIRECTFBSCREEN_H
#define QDIRECTFBSCREEN_H

#include "qdirectfbconvenience.h"
#include "qdirectfbcursor.h"

#include <QtGui/QPlatformIntegration>

#include <directfb.h>

QT_BEGIN_NAMESPACE


class QDirectFbScreen : public QPlatformScreen
{
public:
    QDirectFbScreen(int display);

    QRect geometry() const { return m_geometry; }
    int depth() const { return m_depth; }
    QImage::Format format() const { return m_format; }
    QSize physicalSize() const { return m_physicalSize; }

    // DirectFb helpers
    IDirectFBDisplayLayer *dfbLayer() const;

public:
    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    QSize m_physicalSize;

    QDirectFBPointer<IDirectFBDisplayLayer> m_layer;

private:
    QScopedPointer<QDirectFBCursor> m_cursor;
};

QT_END_NAMESPACE

#endif
