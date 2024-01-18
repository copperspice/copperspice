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

#include <qwin_gdi_integration.h>
#include <qwin_context.h>
#include <qwin_backingstore.h>
#include <qwin_gdi_nativeinterface.h>
#include <qdebug.h>

#include <qpixmap_raster_p.h>

class QWindowsGdiIntegrationPrivate
{
 public:
   QWindowsGdiNativeInterface m_nativeInterface;
};

QWindowsGdiIntegration::QWindowsGdiIntegration(const QStringList &paramList)
   : QWindowsIntegration(paramList), d(new QWindowsGdiIntegrationPrivate)
{
}

QWindowsGdiIntegration::~QWindowsGdiIntegration()
{
}

QPlatformNativeInterface *QWindowsGdiIntegration::nativeInterface() const
{
   return &d->m_nativeInterface;
}

QPlatformPixmap *QWindowsGdiIntegration::createPlatformPixmap(QPlatformPixmap::PixelType type) const
{
   return new QRasterPlatformPixmap(type);
}

QPlatformBackingStore *QWindowsGdiIntegration::createPlatformBackingStore(QWindow *window) const
{
   return new QWindowsBackingStore(window);
}

