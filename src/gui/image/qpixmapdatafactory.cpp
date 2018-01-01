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

#include <qpixmapdatafactory_p.h>

#ifdef Q_WS_QWS
# include <QtGui/qscreen_qws.h>
#endif

#ifdef Q_WS_X11
# include <qpixmap_x11_p.h>
#endif

#if defined(Q_OS_WIN)
# include <qpixmap_raster_p.h>
#endif

#ifdef Q_OS_MAC
# include <qpixmap_mac_p.h>
#endif

#ifdef Q_WS_QPA
# include <qpixmap_raster_p.h>
#endif

#include <qapplication_p.h>
#include <qgraphicssystem_p.h>

QT_BEGIN_NAMESPACE

#if !defined(Q_WS_QWS)

class QSimplePixmapDataFactory : public QPixmapDataFactory
{
 public:
   ~QSimplePixmapDataFactory() {}
   QPixmapData *create(QPixmapData::PixelType type) override;
};

QPixmapData *QSimplePixmapDataFactory::create(QPixmapData::PixelType type)
{
   if (QApplicationPrivate::graphicsSystem()) {
      return QApplicationPrivate::graphicsSystem()->createPixmapData(type);
   }

#if defined(Q_WS_X11)
   return new QX11PixmapData(type);

#elif defined(Q_OS_WIN)
   return new QRasterPixmapData(type);

#elif defined(Q_OS_MAC)
   return new QMacPixmapData(type);

#elif defined(Q_WS_QPA)
   return new QRasterPixmapData(type);

#error QSimplePixmapDataFactory::create() not implemented
#endif
}

Q_GLOBAL_STATIC(QSimplePixmapDataFactory, factory)
#endif

QPixmapDataFactory::~QPixmapDataFactory()
{
}

QPixmapDataFactory *QPixmapDataFactory::instance(int screen)
{
   Q_UNUSED(screen);
#ifdef Q_WS_QWS
   return QScreen::instance()->pixmapDataFactory();
#else
   return factory();
#endif
}

QT_END_NAMESPACE
