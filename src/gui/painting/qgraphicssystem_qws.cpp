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

#include <qscreen_qws.h>
#include <qgraphicssystem_qws_p.h>
#include <qpixmap_raster_p.h>
#include <qwindowsurface_qws_p.h>

QT_BEGIN_NAMESPACE

QPixmapData *QWSGraphicsSystem::createPixmapData(QPixmapData::PixelType type) const
{
   if (screen->pixmapDataFactory()) {
      return screen->pixmapDataFactory()->create(type);   //### For 4.4 compatibility
   } else {
      return new QRasterPixmapData(type);
   }
}

QWindowSurface *QWSGraphicsSystem::createWindowSurface(QWidget *widget) const
{
   return screen->createSurface(widget);
}

QT_END_NAMESPACE
