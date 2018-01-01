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

#include "qdirectfbscreen.h"
#include "qdirectfbcursor.h"

QT_BEGIN_NAMESPACE

QDirectFbScreen::QDirectFbScreen(int display)
    : QPlatformScreen()
    , m_layer(QDirectFbConvenience::dfbDisplayLayer(display))
{
    m_layer->SetCooperativeLevel(m_layer.data(), DLSCL_SHARED);

    DFBDisplayLayerConfig config;
    m_layer->GetConfiguration(m_layer.data(), &config);

    m_format = QDirectFbConvenience::imageFormatFromSurfaceFormat(config.pixelformat, config.surface_caps);
    m_geometry = QRect(0, 0, config.width, config.height);
    const int dpi = 72;
    const qreal inch = 25.4;
    m_depth = QDirectFbConvenience::colorDepthForSurface(config.pixelformat);
    m_physicalSize = QSize(config.width, config.height) * inch / dpi;

    m_cursor.reset(new QDirectFBCursor(this));
}

IDirectFBDisplayLayer *QDirectFbScreen::dfbLayer() const
{
    return m_layer.data();
}


QT_END_NAMESPACE
