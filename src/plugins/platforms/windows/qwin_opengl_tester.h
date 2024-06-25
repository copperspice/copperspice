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

#ifndef QWINDOWSOPENGLTESTER_H
#define QWINDOWSOPENGLTESTER_H

#include <qbytearray.h>
#include <qflags.h>
#include <qversionnumber.h>

class QDebug;
class QVariant;

struct GpuDescription {
   GpuDescription() :  vendorId(0), deviceId(0), revision(0), subSysId(0) {}

   static GpuDescription detect();
   QString toString() const;
   QVariant toVariant() const;

   uint vendorId;
   uint deviceId;
   uint revision;
   uint subSysId;
   QVersionNumber driverVersion;
   QByteArray driverName;
   QByteArray description;
};

class QWindowsOpenGLTester
{
 public:
   enum Renderer {
      InvalidRenderer         = 0x0000,
      DesktopGl               = 0x0001,
      AngleRendererD3d11      = 0x0002,
      AngleRendererD3d9       = 0x0004,
      AngleRendererD3d11Warp  = 0x0008, // "Windows Advanced Rasterization Platform"
      AngleBackendMask        = AngleRendererD3d11 | AngleRendererD3d9 | AngleRendererD3d11Warp,
      Gles                    = 0x0010, // ANGLE/unspecified or Generic GLES for Windows CE.
      GlesMask                = Gles | AngleBackendMask,
      SoftwareRasterizer      = 0x0020,
      RendererMask            = 0x00FF,
      DisableRotationFlag     = 0x0100
   };

   using Renderers = QFlags<Renderer>;

   static Renderer requestedGlesRenderer();
   static Renderer requestedRenderer();

   static Renderers supportedGlesRenderers();
   static Renderers supportedRenderers();

 private:
   static QWindowsOpenGLTester::Renderers detectSupportedRenderers(const GpuDescription &gpu, bool glesOnly);
   static bool testDesktopGL();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWindowsOpenGLTester::Renderers)

#endif
