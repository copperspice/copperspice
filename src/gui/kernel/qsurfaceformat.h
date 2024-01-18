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

#ifndef QSURFACEFORMAT_H
#define QSURFACEFORMAT_H

#include <qglobal.h>
#include <qpair.h>

class QOpenGLContext;
class QSurfaceFormatPrivate;

class Q_GUI_EXPORT QSurfaceFormat
{
 public:
   enum FormatOption {
      StereoBuffers            = 0x0001,
      DebugContext             = 0x0002,
      DeprecatedFunctions      = 0x0004,
      ResetNotification        = 0x0008
   };
   using FormatOptions = QFlags<FormatOption>;

   enum SwapBehavior {
      DefaultSwapBehavior,
      SingleBuffer,
      DoubleBuffer,
      TripleBuffer
   };

   enum RenderableType {
      DefaultRenderableType = 0x0,
      OpenGL                = 0x1,
      OpenGLES              = 0x2,
      OpenVG                = 0x4      // not documented
   };

   enum OpenGLContextProfile {
      NoProfile,
      CoreProfile,
      CompatibilityProfile
   };

   QSurfaceFormat();
   QSurfaceFormat(FormatOptions options);
   QSurfaceFormat(const QSurfaceFormat &other);
   QSurfaceFormat &operator=(const QSurfaceFormat &other);
   ~QSurfaceFormat();

   void setDepthBufferSize(int size);
   int depthBufferSize() const;

   void setStencilBufferSize(int size);
   int stencilBufferSize() const;

   void setRedBufferSize(int size);
   int redBufferSize() const;
   void setGreenBufferSize(int size);
   int greenBufferSize() const;
   void setBlueBufferSize(int size);
   int blueBufferSize() const;
   void setAlphaBufferSize(int size);
   int alphaBufferSize() const;

   void setSamples(int numSamples);
   int samples() const;

   void setSwapBehavior(SwapBehavior behavior);
   SwapBehavior swapBehavior() const;

   bool hasAlpha() const;

   void setProfile(OpenGLContextProfile profile);
   OpenGLContextProfile profile() const;

   void setRenderableType(RenderableType type);
   RenderableType renderableType() const;

   void setMajorVersion(int majorVersion);
   int majorVersion() const;

   void setMinorVersion(int minorVersion);
   int minorVersion() const;

   QPair<int, int> version() const;
   void setVersion(int major, int minor);

   bool stereo() const;
   void setStereo(bool enable);

   void setOptions(QSurfaceFormat::FormatOptions options);
   void setOption(FormatOption option, bool on = true);
   bool testOption(FormatOption option) const;
   QSurfaceFormat::FormatOptions options() const;

   int swapInterval() const;
   void setSwapInterval(int interval);

   static void setDefaultFormat(const QSurfaceFormat &format);
   static QSurfaceFormat defaultFormat();

 private:
   QSurfaceFormatPrivate *d;

   void detach();

   friend Q_GUI_EXPORT bool operator==(const QSurfaceFormat &a, const QSurfaceFormat &b);
   friend Q_GUI_EXPORT bool operator!=(const QSurfaceFormat &a, const QSurfaceFormat &b);

   friend Q_GUI_EXPORT QDebug operator<<(QDebug, const QSurfaceFormat &);
};

Q_GUI_EXPORT bool operator==(const QSurfaceFormat &a, const QSurfaceFormat &b);
Q_GUI_EXPORT bool operator!=(const QSurfaceFormat &a, const QSurfaceFormat &b);

Q_GUI_EXPORT QDebug operator<<(QDebug, const QSurfaceFormat &);

Q_DECLARE_OPERATORS_FOR_FLAGS(QSurfaceFormat::FormatOptions)

inline bool QSurfaceFormat::stereo() const
{
   return testOption(QSurfaceFormat::StereoBuffers);
}


#endif //QSURFACEFORMAT_H
