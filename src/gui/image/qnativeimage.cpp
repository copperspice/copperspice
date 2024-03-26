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

#include <qnativeimage_p.h>

#include <qdebug.h>
#include <qplatform_screen.h>
#include <qscreen.h>

#include <qapplication_p.h>
#include <qpaintengine_raster_p.h>

QNativeImage::QNativeImage(int width, int height, QImage::Format format, bool, QWindow *)
   : image(width, height, format)
{
}

QNativeImage::~QNativeImage()
{
}

QImage::Format QNativeImage::systemFormat()
{
   if (!QGuiApplication::primaryScreen()) {
      return QImage::Format_Invalid;
   }
   return QGuiApplication::primaryScreen()->handle()->format();
}



