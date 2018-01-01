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

#ifndef QGRAPHICSSYSTEM_RASTER_P_H
#define QGRAPHICSSYSTEM_RASTER_P_H

#include "qgraphicssystem_p.h"
#include <QMap>

QT_BEGIN_NAMESPACE

class Q_OPENGL_EXPORT QGLGraphicsSystem : public QGraphicsSystem

{
 public:
   QGLGraphicsSystem(bool useX11GL);

   QPixmapData *createPixmapData(QPixmapData::PixelType type) const override;
   QWindowSurface *createWindowSurface(QWidget *widget) const override;

 private:
   bool m_useX11GL;
};

QT_END_NAMESPACE

#endif

