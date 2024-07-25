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

#ifndef QPAINTERVIDEOSURFACE_P_H
#define QPAINTERVIDEOSURFACE_P_H

#include <qabstractvideosurface.h>
#include <qimage.h>
#include <qmatrix4x4.h>
#include <qpaintengine.h>
#include <qsize.h>
#include <qvideoframe.h>

class QGLContext;

class QVideoSurfacePainter
{
 public:
   virtual ~QVideoSurfacePainter();

   virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(
      QAbstractVideoBuffer::HandleType handleType) const = 0;

   virtual bool isFormatSupported(const QVideoSurfaceFormat &format) const = 0;

   virtual QAbstractVideoSurface::Error start(const QVideoSurfaceFormat &format) = 0;
   virtual void stop() = 0;

   virtual QAbstractVideoSurface::Error setCurrentFrame(const QVideoFrame &frame) = 0;

   virtual QAbstractVideoSurface::Error paint(
      const QRectF &target, QPainter *painter, const QRectF &source) = 0;

   virtual void updateColors(int brightness, int contrast, int hue, int saturation) = 0;
   virtual void viewportDestroyed() {}
};


class QPainterVideoSurface : public QAbstractVideoSurface
{
   MULTI_CS_OBJECT(QPainterVideoSurface)

 public:
   explicit QPainterVideoSurface(QObject *parent = nullptr);
   ~QPainterVideoSurface();

   QList<QVideoFrame::PixelFormat> supportedPixelFormats(
      QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const override;

   bool isFormatSupported(const QVideoSurfaceFormat &format) const override;

   bool start(const QVideoSurfaceFormat &format) override;
   void stop() override;

   bool present(const QVideoFrame &frame) override;

   int brightness() const;
   void setBrightness(int brightness);

   int contrast() const;
   void setContrast(int contrast);

   int hue() const;
   void setHue(int hue);

   int saturation() const;
   void setSaturation(int saturation);

   bool isReady() const;
   void setReady(bool ready);

   void paint(QPainter *painter, const QRectF &target, const QRectF &source = QRectF(0, 0, 1, 1));

#if ! defined(QT_NO_OPENGL) && ! defined(QT_OPENGL_ES_1_CL) && !defined(QT_OPENGL_ES_1)
   const QGLContext *glContext() const;
   void setGLContext(QGLContext *context);

   enum ShaderType {
      NoShaders = 0x00,
      FragmentProgramShader = 0x01,
      GlslShader = 0x02
   };

   using ShaderTypes = QFlags<ShaderType>;

   ShaderTypes supportedShaderTypes() const;

   ShaderType shaderType() const;
   void setShaderType(ShaderType type);
#endif

   MULTI_CS_SLOT_1(Public, void viewportDestroyed())
   MULTI_CS_SLOT_2(viewportDestroyed)

   MULTI_CS_SIGNAL_1(Public, void frameChanged())
   MULTI_CS_SIGNAL_2(frameChanged)

 private:
   void createPainter();

   QVideoSurfacePainter *m_painter;

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_1_CL) && !defined(QT_OPENGL_ES_1)
   QGLContext *m_glContext;
   ShaderTypes m_shaderTypes;
   ShaderType m_shaderType;
#endif

   int m_brightness;
   int m_contrast;
   int m_hue;
   int m_saturation;

   QVideoFrame::PixelFormat m_pixelFormat;
   QSize m_frameSize;
   QRect m_sourceRect;
   bool m_colorsDirty;
   bool m_ready;
};

#if ! defined(QT_NO_OPENGL) && ! defined(QT_OPENGL_ES_1_CL) && !defined(QT_OPENGL_ES_1)
Q_DECLARE_OPERATORS_FOR_FLAGS(QPainterVideoSurface::ShaderTypes)
#endif

#endif
