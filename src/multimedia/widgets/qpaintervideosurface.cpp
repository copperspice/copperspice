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

#include <qpaintervideosurface_p.h>

#include <qmath.h>
#include <qpainter.h>
#include <qvariant.h>
#include <qvideosurfaceformat.h>
#include <qdebug.h>

#include <qmediaopenglhelper_p.h>

#if ! defined(QT_NO_OPENGL) && ! defined(QT_OPENGL_ES_1_CL) && ! defined(QT_OPENGL_ES_1)

#include <qglshaderprogram.h>
#include <qopenglcontext.h>
#include <qopenglfunctions.h>
#include <qwindow.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_RGB8
#define GL_RGB8 0x8051
#endif

#endif

QVideoSurfacePainter::~QVideoSurfacePainter()
{
}

class QVideoSurfaceGenericPainter : public QVideoSurfacePainter
{
 public:
   QVideoSurfaceGenericPainter();

   QList<QVideoFrame::PixelFormat> supportedPixelFormats(
      QAbstractVideoBuffer::HandleType handleType) const override;

   bool isFormatSupported(const QVideoSurfaceFormat &format) const override;

   QAbstractVideoSurface::Error start(const QVideoSurfaceFormat &format) override;
   void stop() override;

   QAbstractVideoSurface::Error setCurrentFrame(const QVideoFrame &frame) override;

   QAbstractVideoSurface::Error paint(
      const QRectF &target, QPainter *painter, const QRectF &source) override;

   void updateColors(int brightness, int contrast, int hue, int saturation) override;

 private:
   QList<QVideoFrame::PixelFormat> m_imagePixelFormats;
   QVideoFrame m_frame;
   QSize m_imageSize;
   QImage::Format m_imageFormat;
   QVideoSurfaceFormat::Direction m_scanLineDirection;
   bool m_mirrored;
};

QVideoSurfaceGenericPainter::QVideoSurfaceGenericPainter()
   : m_imageFormat(QImage::Format_Invalid)
   , m_scanLineDirection(QVideoSurfaceFormat::TopToBottom)
   , m_mirrored(false)
{
   m_imagePixelFormats << QVideoFrame::Format_RGB32;

   // The raster formats should be a subset of the GL formats.
#ifndef QT_NO_OPENGL
   if (QOpenGLContext::openGLModuleType() != QOpenGLContext::LibGLES)
#endif
      m_imagePixelFormats << QVideoFrame::Format_RGB24;

   m_imagePixelFormats << QVideoFrame::Format_ARGB32
      << QVideoFrame::Format_RGB565;
}

QList<QVideoFrame::PixelFormat> QVideoSurfaceGenericPainter::supportedPixelFormats(
   QAbstractVideoBuffer::HandleType handleType) const
{
   switch (handleType) {
      case QAbstractVideoBuffer::QPixmapHandle:
      case QAbstractVideoBuffer::NoHandle:
         return m_imagePixelFormats;

      default:
         break;
   }

   return QList<QVideoFrame::PixelFormat>();
}

bool QVideoSurfaceGenericPainter::isFormatSupported(const QVideoSurfaceFormat &format) const
{
   switch (format.handleType()) {
      case QAbstractVideoBuffer::QPixmapHandle:
         return true;

      case QAbstractVideoBuffer::NoHandle:
         return m_imagePixelFormats.contains(format.pixelFormat())
            && !format.frameSize().isEmpty();

      default:
         break;
   }

   return false;
}

QAbstractVideoSurface::Error QVideoSurfaceGenericPainter::start(const QVideoSurfaceFormat &format)
{
   m_frame = QVideoFrame();
   m_imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
   m_imageSize   = format.frameSize();
   m_scanLineDirection = format.scanLineDirection();
   m_mirrored = format.property("mirrored").toBool();

   const QAbstractVideoBuffer::HandleType t = format.handleType();

   if (t == QAbstractVideoBuffer::NoHandle) {
      bool ok = m_imageFormat != QImage::Format_Invalid && !m_imageSize.isEmpty();

#ifndef QT_NO_OPENGL
      if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
         ok &= format.pixelFormat() != QVideoFrame::Format_RGB24;
      }
#endif

      if (ok) {
         return QAbstractVideoSurface::NoError;
      }

   } else if (t == QAbstractVideoBuffer::QPixmapHandle) {
      return QAbstractVideoSurface::NoError;
   }

   return QAbstractVideoSurface::UnsupportedFormatError;
}

void QVideoSurfaceGenericPainter::stop()
{
   m_frame = QVideoFrame();
}

QAbstractVideoSurface::Error QVideoSurfaceGenericPainter::setCurrentFrame(const QVideoFrame &frame)
{
   m_frame = frame;

   return QAbstractVideoSurface::NoError;
}

QAbstractVideoSurface::Error QVideoSurfaceGenericPainter::paint(
   const QRectF &target, QPainter *painter, const QRectF &source)
{
   if (!m_frame.isValid()) {
      painter->fillRect(target, Qt::black);
      return QAbstractVideoSurface::NoError;
   }

   if (m_frame.handleType() == QAbstractVideoBuffer::QPixmapHandle) {
      painter->drawPixmap(target, m_frame.handle().value<QPixmap>(), source);

   } else if (m_frame.map(QAbstractVideoBuffer::ReadOnly)) {
      QImage image(
         m_frame.bits(),
         m_imageSize.width(),
         m_imageSize.height(),
         m_frame.bytesPerLine(),
         m_imageFormat);

      const QTransform oldTransform = painter->transform();
      QTransform transform = oldTransform;
      QRectF targetRect = target;

      if (m_scanLineDirection == QVideoSurfaceFormat::BottomToTop) {
         transform.scale(1, -1);
         transform.translate(0, -target.bottom());
         targetRect.setY(0);
      }

      if (m_mirrored) {
         transform.scale(-1, 1);
         transform.translate(-target.right(), 0);
         targetRect.setX(0);
      }

      painter->setTransform(transform);
      painter->drawImage(targetRect, image, source);
      painter->setTransform(oldTransform);

      m_frame.unmap();

   } else if (m_frame.isValid()) {
      return QAbstractVideoSurface::IncorrectFormatError;

   } else {
      painter->fillRect(target, Qt::black);
   }

   return QAbstractVideoSurface::NoError;
}

void QVideoSurfaceGenericPainter::updateColors(int, int, int, int)
{
}

#if ! defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_1_CL) && !defined(QT_OPENGL_ES_1)

#ifndef APIENTRYP
#  ifdef APIENTRY
#    define APIENTRYP APIENTRY *
#  else
#    define APIENTRY
#    define APIENTRYP *
#  endif
#endif

#ifndef GL_TEXTURE0
#  define GL_TEXTURE0    0x84C0
#  define GL_TEXTURE1    0x84C1
#  define GL_TEXTURE2    0x84C2
#endif

#ifndef GL_PROGRAM_ERROR_STRING_ARB
#  define GL_PROGRAM_ERROR_STRING_ARB       0x8874
#endif

#ifndef GL_UNSIGNED_SHORT_5_6_5
#  define GL_UNSIGNED_SHORT_5_6_5 33635
#endif

class QVideoSurfaceGLPainter : public QVideoSurfacePainter, protected QOpenGLFunctions
{
 public:
   QVideoSurfaceGLPainter(QGLContext *context);
   ~QVideoSurfaceGLPainter();

   QList<QVideoFrame::PixelFormat> supportedPixelFormats(
      QAbstractVideoBuffer::HandleType handleType) const override;

   bool isFormatSupported(const QVideoSurfaceFormat &format) const override;

   void stop() override;

   QAbstractVideoSurface::Error setCurrentFrame(const QVideoFrame &frame) override;

   QAbstractVideoSurface::Error paint(
      const QRectF &target, QPainter *painter, const QRectF &source) override;

   void updateColors(int brightness, int contrast, int hue, int saturation) override;
   void viewportDestroyed() override;

 protected:
   void initRgbTextureInfo(GLenum internalFormat, GLuint format, GLenum type, const QSize &size);
   void initYuv420PTextureInfo(const QSize &size);
   void initYv12TextureInfo(const QSize &size);

   bool needsSwizzling(const QVideoSurfaceFormat &format) const {
      return !QMediaOpenGLHelper::isANGLE()
         && (format.pixelFormat() == QVideoFrame::Format_RGB32
            || format.pixelFormat() == QVideoFrame::Format_ARGB32);
   }

#if ! defined(QT_OPENGL_ES) && !defined(QT_OPENGL_DYNAMIC)
   typedef void (APIENTRY *_glActiveTexture) (GLenum);
   _glActiveTexture glActiveTexture;
#endif

   QList<QVideoFrame::PixelFormat> m_imagePixelFormats;
   QList<QVideoFrame::PixelFormat> m_glPixelFormats;
   QMatrix4x4 m_colorMatrix;
   QVideoFrame m_frame;

   QGLContext *m_context;
   QAbstractVideoBuffer::HandleType m_handleType;
   QVideoSurfaceFormat::Direction m_scanLineDirection;
   bool m_mirrored;
   QVideoSurfaceFormat::YCbCrColorSpace m_colorSpace;
   GLenum m_textureFormat;
   GLuint m_textureInternalFormat;
   GLenum m_textureType;
   int m_textureCount;

   static constexpr const uint Max_Textures = 3;

   GLuint m_textureIds[Max_Textures];
   int m_textureWidths[Max_Textures];
   int m_textureHeights[Max_Textures];
   int m_textureOffsets[Max_Textures];
   bool m_yuv;
};

QVideoSurfaceGLPainter::QVideoSurfaceGLPainter(QGLContext *context)
   : m_context(context), m_handleType(QAbstractVideoBuffer::NoHandle),
     m_scanLineDirection(QVideoSurfaceFormat::TopToBottom), m_mirrored(false),
     m_colorSpace(QVideoSurfaceFormat::YCbCr_BT601), m_textureFormat(0),
     m_textureInternalFormat(0), m_textureType(0), m_textureCount(0), m_yuv(false)
{
#if ! defined(QT_OPENGL_ES) && !defined(QT_OPENGL_DYNAMIC)
   glActiveTexture = (_glActiveTexture)m_context->getProcAddress("glActiveTexture");
#endif

   memset(m_textureIds, 0, sizeof(m_textureIds));
   memset(m_textureWidths, 0, sizeof(m_textureWidths));
   memset(m_textureHeights, 0, sizeof(m_textureHeights));
   memset(m_textureOffsets, 0, sizeof(m_textureOffsets));

   initializeOpenGLFunctions();
}

QVideoSurfaceGLPainter::~QVideoSurfaceGLPainter()
{
}

void QVideoSurfaceGLPainter::viewportDestroyed()
{
   m_context = nullptr;
}

QList<QVideoFrame::PixelFormat> QVideoSurfaceGLPainter::supportedPixelFormats(
   QAbstractVideoBuffer::HandleType handleType) const
{
   switch (handleType) {
      case QAbstractVideoBuffer::NoHandle:
         return m_imagePixelFormats;

      case QAbstractVideoBuffer::QPixmapHandle:
      case QAbstractVideoBuffer::GLTextureHandle:
         return m_glPixelFormats;

      default:
         break;
   }

   return QList<QVideoFrame::PixelFormat>();
}

bool QVideoSurfaceGLPainter::isFormatSupported(const QVideoSurfaceFormat &format) const
{
   if (format.frameSize().isEmpty()) {
      return false;

   } else {
      switch (format.handleType()) {
         case QAbstractVideoBuffer::NoHandle:
            return m_imagePixelFormats.contains(format.pixelFormat());

         case QAbstractVideoBuffer::QPixmapHandle:
         case QAbstractVideoBuffer::GLTextureHandle:
            return m_glPixelFormats.contains(format.pixelFormat());

         default:
            break;
      }
   }

   return false;
}

void QVideoSurfaceGLPainter::stop()
{
   m_frame = QVideoFrame();
}

QAbstractVideoSurface::Error QVideoSurfaceGLPainter::setCurrentFrame(const QVideoFrame &frame)
{
   m_frame = frame;

   if (m_handleType == QAbstractVideoBuffer::GLTextureHandle) {
      m_textureIds[0] = frame.handle().toInt();
      glBindTexture(GL_TEXTURE_2D, m_textureIds[0]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   } else if (m_frame.map(QAbstractVideoBuffer::ReadOnly)) {
      m_context->makeCurrent();

      for (int i = 0; i < m_textureCount; ++i) {
         glBindTexture(GL_TEXTURE_2D, m_textureIds[i]);

         glTexImage2D(GL_TEXTURE_2D, 0, m_textureInternalFormat, m_textureWidths[i], m_textureHeights[i],
            0, m_textureFormat, m_textureType, m_frame.bits() + m_textureOffsets[i]);

         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }

      m_frame.unmap();

   } else if (m_handleType != QAbstractVideoBuffer::QPixmapHandle && m_frame.isValid()) {
      return QAbstractVideoSurface::IncorrectFormatError;
   }

   return QAbstractVideoSurface::NoError;
}

QAbstractVideoSurface::Error QVideoSurfaceGLPainter::paint(
   const QRectF &target, QPainter *painter, const QRectF &source)
{
   if (! m_frame.isValid()) {
      painter->fillRect(target, Qt::black);
      return QAbstractVideoSurface::NoError;
   }

   if (m_frame.handleType() == QAbstractVideoBuffer::QPixmapHandle) {
      painter->drawPixmap(target, m_frame.handle().value<QPixmap>(), source);

   } else if (m_frame.isValid()) {
      return QAbstractVideoSurface::IncorrectFormatError;

   } else {
      painter->fillRect(target, Qt::black);
   }

   return QAbstractVideoSurface::NoError;
}

void QVideoSurfaceGLPainter::updateColors(int brightness, int contrast, int hue, int saturation)
{
   const qreal b = brightness / 200.0;
   const qreal c = contrast / 100.0 + 1.0;
   const qreal h = hue / 100.0;
   const qreal s = saturation / 100.0 + 1.0;

   const qreal cosH = qCos(M_PI * h);
   const qreal sinH = qSin(M_PI * h);

   const qreal h11 =  0.787 * cosH - 0.213 * sinH + 0.213;
   const qreal h21 = -0.213 * cosH + 0.143 * sinH + 0.213;
   const qreal h31 = -0.213 * cosH - 0.787 * sinH + 0.213;

   const qreal h12 = -0.715 * cosH - 0.715 * sinH + 0.715;
   const qreal h22 =  0.285 * cosH + 0.140 * sinH + 0.715;
   const qreal h32 = -0.715 * cosH + 0.715 * sinH + 0.715;

   const qreal h13 = -0.072 * cosH + 0.928 * sinH + 0.072;
   const qreal h23 = -0.072 * cosH - 0.283 * sinH + 0.072;
   const qreal h33 =  0.928 * cosH + 0.072 * sinH + 0.072;

   const qreal sr = (1.0 - s) * 0.3086;
   const qreal sg = (1.0 - s) * 0.6094;
   const qreal sb = (1.0 - s) * 0.0820;

   const qreal sr_s = sr + s;
   const qreal sg_s = sg + s;
   const qreal sb_s = sr + s;

   const float m4 = (s + sr + sg + sb) * (0.5 - 0.5 * c + b);

   m_colorMatrix(0, 0) = c * (sr_s * h11 + sg * h21 + sb * h31);
   m_colorMatrix(0, 1) = c * (sr_s * h12 + sg * h22 + sb * h32);
   m_colorMatrix(0, 2) = c * (sr_s * h13 + sg * h23 + sb * h33);
   m_colorMatrix(0, 3) = m4;

   m_colorMatrix(1, 0) = c * (sr * h11 + sg_s * h21 + sb * h31);
   m_colorMatrix(1, 1) = c * (sr * h12 + sg_s * h22 + sb * h32);
   m_colorMatrix(1, 2) = c * (sr * h13 + sg_s * h23 + sb * h33);
   m_colorMatrix(1, 3) = m4;

   m_colorMatrix(2, 0) = c * (sr * h11 + sg * h21 + sb_s * h31);
   m_colorMatrix(2, 1) = c * (sr * h12 + sg * h22 + sb_s * h32);
   m_colorMatrix(2, 2) = c * (sr * h13 + sg * h23 + sb_s * h33);
   m_colorMatrix(2, 3) = m4;

   m_colorMatrix(3, 0) = 0.0;
   m_colorMatrix(3, 1) = 0.0;
   m_colorMatrix(3, 2) = 0.0;
   m_colorMatrix(3, 3) = 1.0;

   if (m_yuv) {
      QMatrix4x4 colorSpaceMatrix;

      switch (m_colorSpace) {
         case QVideoSurfaceFormat::YCbCr_JPEG:
            colorSpaceMatrix = QMatrix4x4(
                  1.0f,  0.000f,  1.402f, -0.701f,
                  1.0f, -0.344f, -0.714f,  0.529f,
                  1.0f,  1.772f,  0.000f, -0.886f,
                  0.0f,  0.000f,  0.000f,  1.0000f);
            break;

         case QVideoSurfaceFormat::YCbCr_BT709:
         case QVideoSurfaceFormat::YCbCr_xvYCC709:
            colorSpaceMatrix = QMatrix4x4(
                  1.164f,  0.000f,  1.793f, -0.5727f,
                  1.164f, -0.534f, -0.213f,  0.3007f,
                  1.164f,  2.115f,  0.000f, -1.1302f,
                  0.0f,    0.000f,  0.000f,  1.0000f);
            break;

         default: //BT 601:
            colorSpaceMatrix = QMatrix4x4(
                  1.164f,  0.000f,  1.596f, -0.8708f,
                  1.164f, -0.392f, -0.813f,  0.5296f,
                  1.164f,  2.017f,  0.000f, -1.081f,
                  0.0f,    0.000f,  0.000f,  1.0000f);
      }

      m_colorMatrix = m_colorMatrix * colorSpaceMatrix;
   }
}

void QVideoSurfaceGLPainter::initRgbTextureInfo(
   GLenum internalFormat, GLuint format, GLenum type, const QSize &size)
{
   m_yuv = false;
   m_textureInternalFormat = internalFormat;
   m_textureFormat = format;
   m_textureType = type;
   m_textureCount = 1; // Note: ensure this is always <= Max_Textures
   m_textureWidths[0] = size.width();
   m_textureHeights[0] = size.height();
   m_textureOffsets[0] = 0;
}

void QVideoSurfaceGLPainter::initYuv420PTextureInfo(const QSize &size)
{
   int bytesPerLine = (size.width() + 3) & ~3;
   int bytesPerLine2 = (size.width() / 2 + 3) & ~3;

   m_yuv = true;
   m_textureInternalFormat = GL_LUMINANCE;
   m_textureFormat = GL_LUMINANCE;
   m_textureType = GL_UNSIGNED_BYTE;
   m_textureCount = 3; // Note: ensure this is always <= Max_Textures
   m_textureWidths[0] = bytesPerLine;
   m_textureHeights[0] = size.height();
   m_textureOffsets[0] = 0;
   m_textureWidths[1] = bytesPerLine2;
   m_textureHeights[1] = size.height() / 2;
   m_textureOffsets[1] = bytesPerLine * size.height();
   m_textureWidths[2] = bytesPerLine2;
   m_textureHeights[2] = size.height() / 2;
   m_textureOffsets[2] = bytesPerLine * size.height() + bytesPerLine2 * size.height() / 2;
}

void QVideoSurfaceGLPainter::initYv12TextureInfo(const QSize &size)
{
   int bytesPerLine = (size.width() + 3) & ~3;
   int bytesPerLine2 = (size.width() / 2 + 3) & ~3;

   m_yuv = true;
   m_textureInternalFormat = GL_LUMINANCE;
   m_textureFormat = GL_LUMINANCE;
   m_textureType = GL_UNSIGNED_BYTE;
   m_textureCount = 3; // Note: ensure this is always <= Max_Textures
   m_textureWidths[0] = bytesPerLine;
   m_textureHeights[0] = size.height();
   m_textureOffsets[0] = 0;
   m_textureWidths[1] = bytesPerLine2;
   m_textureHeights[1] = size.height() / 2;
   m_textureOffsets[1] = bytesPerLine * size.height() + bytesPerLine2 * size.height() / 2;
   m_textureWidths[2] = bytesPerLine2;
   m_textureHeights[2] = size.height() / 2;
   m_textureOffsets[2] = bytesPerLine * size.height();
}

#if !defined(QT_OPENGL_ES) && !defined(QT_OPENGL_DYNAMIC)

# ifndef GL_FRAGMENT_PROGRAM_ARB
#  define GL_FRAGMENT_PROGRAM_ARB           0x8804
#  define GL_PROGRAM_FORMAT_ASCII_ARB       0x8875
# endif

// Paints an RGB32 frame
static const char *qt_arbfp_xrgbShaderProgram =
   "!!ARBfp1.0\n"
   "PARAM matrix[4] = { program.local[0..2],"
   "{ 0.0, 0.0, 0.0, 1.0 } };\n"
   "TEMP xrgb;\n"
   "TEX xrgb.xyz, fragment.texcoord[0], texture[0], 2D;\n"
   "MOV xrgb.w, matrix[3].w;\n"
   "DP4 result.color.x, xrgb.zyxw, matrix[0];\n"
   "DP4 result.color.y, xrgb.zyxw, matrix[1];\n"
   "DP4 result.color.z, xrgb.zyxw, matrix[2];\n"
   "END";

// Paints an ARGB frame.
static const char *qt_arbfp_argbShaderProgram =
   "!!ARBfp1.0\n"
   "PARAM matrix[4] = { program.local[0..2],"
   "{ 0.0, 0.0, 0.0, 1.0 } };\n"
   "TEMP argb;\n"
   "TEX argb, fragment.texcoord[0], texture[0], 2D;\n"
   "MOV argb.w, matrix[3].w;\n"
   "DP4 result.color.x, argb.zyxw, matrix[0];\n"
   "DP4 result.color.y, argb.zyxw, matrix[1];\n"
   "DP4 result.color.z, argb.zyxw, matrix[2];\n"
   "TEX result.color.w, fragment.texcoord[0], texture, 2D;\n"
   "END";

// Paints an RGB(A) frame.
static const char *qt_arbfp_rgbShaderProgram =
   "!!ARBfp1.0\n"
   "PARAM matrix[4] = { program.local[0..2],"
   "{ 0.0, 0.0, 0.0, 1.0 } };\n"
   "TEMP rgb;\n"
   "TEX rgb, fragment.texcoord[0], texture[0], 2D;\n"
   "MOV rgb.w, matrix[3].w;\n"
   "DP4 result.color.x, rgb, matrix[0];\n"
   "DP4 result.color.y, rgb, matrix[1];\n"
   "DP4 result.color.z, rgb, matrix[2];\n"
   "TEX result.color.w, fragment.texcoord[0], texture, 2D;\n"
   "END";

// Paints a YUV420P or YV12 frame.
static const char *qt_arbfp_yuvPlanarShaderProgram =
   "!!ARBfp1.0\n"
   "PARAM matrix[4] = { program.local[0..2],"
   "{ 0.0, 0.0, 0.0, 1.0 } };\n"
   "TEMP yuv;\n"
   "TEX yuv.x, fragment.texcoord[0], texture[0], 2D;\n"
   "TEX yuv.y, fragment.texcoord[0], texture[1], 2D;\n"
   "TEX yuv.z, fragment.texcoord[0], texture[2], 2D;\n"
   "MOV yuv.w, matrix[3].w;\n"
   "DP4 result.color.x, yuv, matrix[0];\n"
   "DP4 result.color.y, yuv, matrix[1];\n"
   "DP4 result.color.z, yuv, matrix[2];\n"
   "END";

// Paints a YUV444 frame.
static const char *qt_arbfp_xyuvShaderProgram =
   "!!ARBfp1.0\n"
   "PARAM matrix[4] = { program.local[0..2],"
   "{ 0.0, 0.0, 0.0, 1.0 } };\n"
   "TEMP ayuv;\n"
   "TEX ayuv, fragment.texcoord[0], texture[0], 2D;\n"
   "MOV ayuv.x, matrix[3].w;\n"
   "DP4 result.color.x, ayuv.yzwx, matrix[0];\n"
   "DP4 result.color.y, ayuv.yzwx, matrix[1];\n"
   "DP4 result.color.z, ayuv.yzwx, matrix[2];\n"
   "END";

// Paints a AYUV444 frame.
static const char *qt_arbfp_ayuvShaderProgram =
   "!!ARBfp1.0\n"
   "PARAM matrix[4] = { program.local[0..2],"
   "{ 0.0, 0.0, 0.0, 1.0 } };\n"
   "TEMP ayuv;\n"
   "TEX ayuv, fragment.texcoord[0], texture[0], 2D;\n"
   "MOV ayuv.x, matrix[3].w;\n"
   "DP4 result.color.x, ayuv.yzwx, matrix[0];\n"
   "DP4 result.color.y, ayuv.yzwx, matrix[1];\n"
   "DP4 result.color.z, ayuv.yzwx, matrix[2];\n"
   "TEX result.color.w, fragment.texcoord[0], texture, 2D;\n"
   "END";

class QVideoSurfaceArbFpPainter : public QVideoSurfaceGLPainter
{
 public:
   QVideoSurfaceArbFpPainter(QGLContext *context);

   QAbstractVideoSurface::Error start(const QVideoSurfaceFormat &format) override;
   void stop() override;

   QAbstractVideoSurface::Error paint(const QRectF &target, QPainter *painter, const QRectF &source) override;

 private:
   typedef void (APIENTRY *_glProgramStringARB) (GLenum, GLenum, GLsizei, const GLvoid *);
   typedef void (APIENTRY *_glBindProgramARB) (GLenum, GLuint);
   typedef void (APIENTRY *_glDeleteProgramsARB) (GLsizei, const GLuint *);
   typedef void (APIENTRY *_glGenProgramsARB) (GLsizei, GLuint *);
   typedef void (APIENTRY *_glProgramLocalParameter4fARB) (GLenum, GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
   typedef void (APIENTRY *_glActiveTexture) (GLenum);

   _glProgramStringARB glProgramStringARB;
   _glBindProgramARB glBindProgramARB;
   _glDeleteProgramsARB glDeleteProgramsARB;
   _glGenProgramsARB glGenProgramsARB;
   _glProgramLocalParameter4fARB glProgramLocalParameter4fARB;

   GLuint m_programId;
   QSize m_frameSize;
};

QVideoSurfaceArbFpPainter::QVideoSurfaceArbFpPainter(QGLContext *context)
   : QVideoSurfaceGLPainter(context), m_programId(0)
{
   glProgramStringARB  = (_glProgramStringARB) m_context->getProcAddress("glProgramStringARB");
   glBindProgramARB    = (_glBindProgramARB) m_context->getProcAddress("glBindProgramARB");
   glDeleteProgramsARB = (_glDeleteProgramsARB) m_context->getProcAddress("glDeleteProgramsARB");
   glGenProgramsARB    = (_glGenProgramsARB) m_context->getProcAddress("glGenProgramsARB");

   glProgramLocalParameter4fARB = (_glProgramLocalParameter4fARB) m_context->getProcAddress("glProgramLocalParameter4fARB");

   m_imagePixelFormats
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_BGR32
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_BGR24
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_AYUV444
            << QVideoFrame::Format_YUV444
            << QVideoFrame::Format_YV12
            << QVideoFrame::Format_YUV420P;

   m_glPixelFormats
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_BGR32
            << QVideoFrame::Format_BGRA32;
}

QAbstractVideoSurface::Error QVideoSurfaceArbFpPainter::start(const QVideoSurfaceFormat &format)
{
   Q_ASSERT(m_textureCount == 0);

   QAbstractVideoSurface::Error error = QAbstractVideoSurface::NoError;

   m_context->makeCurrent();

   const char *program = nullptr;

   if (format.handleType() == QAbstractVideoBuffer::NoHandle) {
      switch (format.pixelFormat()) {
         case QVideoFrame::Format_RGB32:
            initRgbTextureInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            program = qt_arbfp_xrgbShaderProgram;
            break;

         case QVideoFrame::Format_BGR32:
            initRgbTextureInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            program = qt_arbfp_rgbShaderProgram;
            break;

         case QVideoFrame::Format_ARGB32:
            initRgbTextureInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            program = qt_arbfp_argbShaderProgram;
            break;

         case QVideoFrame::Format_RGB24:
            initRgbTextureInfo(GL_RGB8, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            program = qt_arbfp_rgbShaderProgram;
            break;

         case QVideoFrame::Format_BGR24:
            initRgbTextureInfo(GL_RGB8, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            program = qt_arbfp_xrgbShaderProgram;
            break;

         case QVideoFrame::Format_RGB565:
            initRgbTextureInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, format.frameSize());
            program = qt_arbfp_rgbShaderProgram;
            break;

         case QVideoFrame::Format_YUV444:
            initRgbTextureInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, format.frameSize());
            program = qt_arbfp_xyuvShaderProgram;
            m_yuv = true;
            break;

         case QVideoFrame::Format_AYUV444:
            initRgbTextureInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            program = qt_arbfp_ayuvShaderProgram;
            m_yuv = true;
            break;

         case QVideoFrame::Format_YV12:
            initYv12TextureInfo(format.frameSize());
            program = qt_arbfp_yuvPlanarShaderProgram;
            break;

         case QVideoFrame::Format_YUV420P:
            initYuv420PTextureInfo(format.frameSize());
            program = qt_arbfp_yuvPlanarShaderProgram;
            break;

         default:
            break;
      }

   } else if (format.handleType() == QAbstractVideoBuffer::GLTextureHandle) {
      switch (format.pixelFormat()) {
         case QVideoFrame::Format_RGB32:
         case QVideoFrame::Format_ARGB32:
         case QVideoFrame::Format_BGR32:
         case QVideoFrame::Format_BGRA32:
            m_yuv = false;
            m_textureCount = 1;
            if (needsSwizzling(format)) {
               program = qt_arbfp_xrgbShaderProgram;
            } else {
               program = qt_arbfp_rgbShaderProgram;
            }
            break;

         default:
            break;
      }

   } else if (format.handleType() == QAbstractVideoBuffer::QPixmapHandle) {
      m_handleType = QAbstractVideoBuffer::QPixmapHandle;
      return QAbstractVideoSurface::NoError;
   }

   if (!program) {
      error = QAbstractVideoSurface::UnsupportedFormatError;

   } else {
      // clear previous unrelated errors
      while (glGetError() != GL_NO_ERROR) {
      }

      glGenProgramsARB(1, &m_programId);

      GLenum glError = glGetError();
      if (glError != GL_NO_ERROR) {
         qWarning("QPainterVideoSurface: ARBfb Shader allocation error %x", int(glError));
         m_textureCount = 0;
         m_programId = 0;

         error = QAbstractVideoSurface::ResourceError;

      } else {
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_programId);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
               qstrlen(program), reinterpret_cast<const GLvoid *>(program));

         if ((glError = glGetError()) != GL_NO_ERROR) {
            const GLubyte *errorString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

            qWarning("QPainterVideoSurface: ARBfp Shader compile error %x, %s", int(glError),
               reinterpret_cast<const char *>(errorString));

            glDeleteProgramsARB(1, &m_programId);

            m_textureCount = 0;
            m_programId = 0;

            error = QAbstractVideoSurface::ResourceError;

         } else {
            m_handleType = format.handleType();
            m_scanLineDirection = format.scanLineDirection();
            m_mirrored = format.property("mirrored").toBool();
            m_frameSize = format.frameSize();
            m_colorSpace = format.yCbCrColorSpace();

            if (m_handleType == QAbstractVideoBuffer::NoHandle) {
               glGenTextures(m_textureCount, m_textureIds);
            }
         }
      }
   }

   return error;
}

void QVideoSurfaceArbFpPainter::stop()
{
   if (m_context) {
      m_context->makeCurrent();

      if (m_handleType != QAbstractVideoBuffer::GLTextureHandle) {
         glDeleteTextures(m_textureCount, m_textureIds);
      }
      glDeleteProgramsARB(1, &m_programId);
   }

   m_textureCount = 0;
   m_programId = 0;
   m_handleType = QAbstractVideoBuffer::NoHandle;

   QVideoSurfaceGLPainter::stop();
}

QAbstractVideoSurface::Error QVideoSurfaceArbFpPainter::paint(
   const QRectF &target, QPainter *painter, const QRectF &source)
{
   if (!m_frame.isValid()) {
      painter->fillRect(target, Qt::black);
      return QAbstractVideoSurface::NoError;
   }

   const QAbstractVideoBuffer::HandleType h = m_frame.handleType();
   if (h == QAbstractVideoBuffer::NoHandle || h == QAbstractVideoBuffer::GLTextureHandle) {
      bool stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
      bool scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);

      painter->beginNativePainting();

      if (stencilTestEnabled) {
         glEnable(GL_STENCIL_TEST);
      }
      if (scissorTestEnabled) {
         glEnable(GL_SCISSOR_TEST);
      }

      const float txLeft = m_mirrored ? source.right() / m_frameSize.width()
         : source.left() / m_frameSize.width();
      const float txRight = m_mirrored ? source.left() / m_frameSize.width()
         : source.right() / m_frameSize.width();
      const float txTop = m_scanLineDirection == QVideoSurfaceFormat::TopToBottom
         ? source.top() / m_frameSize.height()
         : source.bottom() / m_frameSize.height();
      const float txBottom = m_scanLineDirection == QVideoSurfaceFormat::TopToBottom
         ? source.bottom() / m_frameSize.height()
         : source.top() / m_frameSize.height();

      const float tx_array[] = {
         txLeft, txBottom,
         txRight, txBottom,
         txLeft, txTop,
         txRight, txTop
      };

      const GLfloat v_array[] = {
         GLfloat(target.left()), GLfloat(target.bottom() + 1),
         GLfloat(target.right() + 1), GLfloat(target.bottom() + 1),
         GLfloat(target.left()), GLfloat(target.top()),
         GLfloat(target.right() + 1), GLfloat(target.top())
      };

      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_programId);

      glProgramLocalParameter4fARB(
         GL_FRAGMENT_PROGRAM_ARB,
         0,
         m_colorMatrix(0, 0),
         m_colorMatrix(0, 1),
         m_colorMatrix(0, 2),
         m_colorMatrix(0, 3));
      glProgramLocalParameter4fARB(
         GL_FRAGMENT_PROGRAM_ARB,
         1,
         m_colorMatrix(1, 0),
         m_colorMatrix(1, 1),
         m_colorMatrix(1, 2),
         m_colorMatrix(1, 3));
      glProgramLocalParameter4fARB(
         GL_FRAGMENT_PROGRAM_ARB,
         2,
         m_colorMatrix(2, 0),
         m_colorMatrix(2, 1),
         m_colorMatrix(2, 2),
         m_colorMatrix(2, 3));

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_textureIds[0]);

      if (m_textureCount == 3) {
         glActiveTexture(GL_TEXTURE1);
         glBindTexture(GL_TEXTURE_2D, m_textureIds[1]);
         glActiveTexture(GL_TEXTURE2);
         glBindTexture(GL_TEXTURE_2D, m_textureIds[2]);
         glActiveTexture(GL_TEXTURE0);
      }

      glVertexPointer(2, GL_FLOAT, 0, v_array);
      glTexCoordPointer(2, GL_FLOAT, 0, tx_array);

      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisable(GL_FRAGMENT_PROGRAM_ARB);

      painter->endNativePainting();

      return QAbstractVideoSurface::NoError;
   }

   return QVideoSurfaceGLPainter::paint(target, painter, source);
}

#endif // !QT_OPENGL_ES && !QT_OPENGL_DYNAMIC

static const char *qt_glsl_vertexShaderProgram =
   "attribute highp vec4 vertexCoordArray;\n"
   "attribute highp vec2 textureCoordArray;\n"
   "uniform highp mat4 positionMatrix;\n"
   "varying highp vec2 textureCoord;\n"
   "void main(void)\n"
   "{\n"
   "   gl_Position = positionMatrix * vertexCoordArray;\n"
   "   textureCoord = textureCoordArray;\n"
   "}\n";

// Paints an RGB32 frame
static const char *qt_glsl_xrgbShaderProgram =
   "uniform sampler2D texRgb;\n"
   "uniform mediump mat4 colorMatrix;\n"
   "varying highp vec2 textureCoord;\n"
   "void main(void)\n"
   "{\n"
   "    highp vec4 color = vec4(texture2D(texRgb, textureCoord.st).bgr, 1.0);\n"
   "    gl_FragColor = colorMatrix * color;\n"
   "}\n";

// Paints an ARGB frame.
static const char *qt_glsl_argbShaderProgram =
   "uniform sampler2D texRgb;\n"
   "uniform mediump mat4 colorMatrix;\n"
   "varying highp vec2 textureCoord;\n"
   "void main(void)\n"
   "{\n"
   "    highp vec4 color = vec4(texture2D(texRgb, textureCoord.st).bgr, 1.0);\n"
   "    color = colorMatrix * color;\n"
   "    gl_FragColor = vec4(color.rgb, texture2D(texRgb, textureCoord.st).a);\n"
   "}\n";

// Paints an RGB(A) frame.
static const char *qt_glsl_rgbShaderProgram =
   "uniform sampler2D texRgb;\n"
   "uniform mediump mat4 colorMatrix;\n"
   "varying highp vec2 textureCoord;\n"
   "void main(void)\n"
   "{\n"
   "    highp vec4 color = vec4(texture2D(texRgb, textureCoord.st).rgb, 1.0);\n"
   "    color = colorMatrix * color;\n"
   "    gl_FragColor = vec4(color.rgb, texture2D(texRgb, textureCoord.st).a);\n"
   "}\n";

// Paints a YUV420P or YV12 frame.
static const char *qt_glsl_yuvPlanarShaderProgram =
   "uniform sampler2D texY;\n"
   "uniform sampler2D texU;\n"
   "uniform sampler2D texV;\n"
   "uniform mediump mat4 colorMatrix;\n"
   "varying highp vec2 textureCoord;\n"
   "void main(void)\n"
   "{\n"
   "    highp vec4 color = vec4(\n"
   "           texture2D(texY, textureCoord.st).r,\n"
   "           texture2D(texU, textureCoord.st).r,\n"
   "           texture2D(texV, textureCoord.st).r,\n"
   "           1.0);\n"
   "    gl_FragColor = colorMatrix * color;\n"
   "}\n";

// Paints a YUV444 frame.
static const char *qt_glsl_xyuvShaderProgram =
   "uniform sampler2D texRgb;\n"
   "uniform mediump mat4 colorMatrix;\n"
   "varying highp vec2 textureCoord;\n"
   "void main(void)\n"
   "{\n"
   "    highp vec4 color = vec4(texture2D(texRgb, textureCoord.st).gba, 1.0);\n"
   "    gl_FragColor = colorMatrix * color;\n"
   "}\n";

// Paints a AYUV444 frame.
static const char *qt_glsl_ayuvShaderProgram =
   "uniform sampler2D texRgb;\n"
   "uniform mediump mat4 colorMatrix;\n"
   "varying highp vec2 textureCoord;\n"
   "void main(void)\n"
   "{\n"
   "    highp vec4 color = vec4(texture2D(texRgb, textureCoord.st).gba, 1.0);\n"
   "    color = colorMatrix * color;\n"
   "    gl_FragColor = vec4(color.rgb, texture2D(texRgb, textureCoord.st).r);\n"
   "}\n";

class QVideoSurfaceGlslPainter : public QVideoSurfaceGLPainter
{
 public:
   QVideoSurfaceGlslPainter(QGLContext *context);

   QAbstractVideoSurface::Error start(const QVideoSurfaceFormat &format) override;
   void stop() override;

   QAbstractVideoSurface::Error paint(
      const QRectF &target, QPainter *painter, const QRectF &source) override;

 private:
   QGLShaderProgram m_program;
   QSize m_frameSize;
};

QVideoSurfaceGlslPainter::QVideoSurfaceGlslPainter(QGLContext *context)
   : QVideoSurfaceGLPainter(context), m_program(context)
{
   m_imagePixelFormats
         << QVideoFrame::Format_RGB32
         << QVideoFrame::Format_BGR32
         << QVideoFrame::Format_ARGB32;

   if (! context->contextHandle()->isOpenGLES()) {
      m_imagePixelFormats
         << QVideoFrame::Format_RGB24
         << QVideoFrame::Format_BGR24;
   }

   m_imagePixelFormats
         << QVideoFrame::Format_RGB565
         << QVideoFrame::Format_YUV444
         << QVideoFrame::Format_AYUV444
         << QVideoFrame::Format_YV12
         << QVideoFrame::Format_YUV420P;

   m_glPixelFormats
         << QVideoFrame::Format_RGB32
         << QVideoFrame::Format_ARGB32
         << QVideoFrame::Format_BGR32
         << QVideoFrame::Format_BGRA32;
}

QAbstractVideoSurface::Error QVideoSurfaceGlslPainter::start(const QVideoSurfaceFormat &format)
{
   Q_ASSERT(m_textureCount == 0);

   QAbstractVideoSurface::Error error = QAbstractVideoSurface::NoError;

   m_context->makeCurrent();

   const char *fragmentProgram = nullptr;

   if (format.handleType() == QAbstractVideoBuffer::NoHandle) {
      switch (format.pixelFormat()) {
         case QVideoFrame::Format_RGB32:
            initRgbTextureInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            fragmentProgram = qt_glsl_xrgbShaderProgram;
            break;

         case QVideoFrame::Format_BGR32:
            initRgbTextureInfo(GL_RGB, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            fragmentProgram = qt_glsl_rgbShaderProgram;
            break;

         case QVideoFrame::Format_ARGB32:
            initRgbTextureInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            fragmentProgram = qt_glsl_argbShaderProgram;
            break;

         case QVideoFrame::Format_RGB24:
            if (!m_context->contextHandle()->isOpenGLES()) {
               initRgbTextureInfo(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, format.frameSize());
               fragmentProgram = qt_glsl_rgbShaderProgram;
            }
            break;

         case QVideoFrame::Format_BGR24:
            if (!m_context->contextHandle()->isOpenGLES()) {
               initRgbTextureInfo(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, format.frameSize());
               fragmentProgram = qt_glsl_argbShaderProgram;
            }
            break;

         case QVideoFrame::Format_RGB565:
            initRgbTextureInfo(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, format.frameSize());
            fragmentProgram = qt_glsl_rgbShaderProgram;
            break;

         case QVideoFrame::Format_YUV444:
            initRgbTextureInfo(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, format.frameSize());
            fragmentProgram = qt_glsl_xyuvShaderProgram;
            m_yuv = true;
            break;

         case QVideoFrame::Format_AYUV444:
            initRgbTextureInfo(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, format.frameSize());
            fragmentProgram = qt_glsl_ayuvShaderProgram;
            m_yuv = true;
            break;

         case QVideoFrame::Format_YV12:
            initYv12TextureInfo(format.frameSize());
            fragmentProgram = qt_glsl_yuvPlanarShaderProgram;
            break;

         case QVideoFrame::Format_YUV420P:
            initYuv420PTextureInfo(format.frameSize());
            fragmentProgram = qt_glsl_yuvPlanarShaderProgram;
            break;

         default:
            break;
      }

   } else if (format.handleType() == QAbstractVideoBuffer::GLTextureHandle) {
      switch (format.pixelFormat()) {
         case QVideoFrame::Format_RGB32:
         case QVideoFrame::Format_ARGB32:
         case QVideoFrame::Format_BGR32:
         case QVideoFrame::Format_BGRA32:
            m_yuv = false;
            m_textureCount = 1;
            if (needsSwizzling(format)) {
               fragmentProgram = qt_glsl_xrgbShaderProgram;
            } else {
               fragmentProgram = qt_glsl_rgbShaderProgram;
            }
            break;

         default:
            break;
      }

   } else if (format.handleType() == QAbstractVideoBuffer::QPixmapHandle) {
      m_handleType = QAbstractVideoBuffer::QPixmapHandle;
      return QAbstractVideoSurface::NoError;
   }

   if (! fragmentProgram) {
      error = QAbstractVideoSurface::UnsupportedFormatError;

   } else if (!m_program.addShaderFromSourceCode(QGLShader::Vertex, qt_glsl_vertexShaderProgram)) {
      qWarning("QPainterVideoSurface: Vertex shader compile error %s", csPrintable(m_program.log()));
      error = QAbstractVideoSurface::ResourceError;

   } else if (!m_program.addShaderFromSourceCode(QGLShader::Fragment, fragmentProgram)) {
      qWarning("QPainterVideoSurface: Shader compile error %s", csPrintable(m_program.log()));
      error = QAbstractVideoSurface::ResourceError;
      m_program.removeAllShaders();

   } else if (!m_program.link()) {
      qWarning("QPainterVideoSurface: Shader link error %s", csPrintable(m_program.log()));
      m_program.removeAllShaders();
      error = QAbstractVideoSurface::ResourceError;

   } else {
      m_handleType = format.handleType();
      m_scanLineDirection = format.scanLineDirection();
      m_mirrored = format.property("mirrored").toBool();
      m_frameSize = format.frameSize();
      m_colorSpace = format.yCbCrColorSpace();

      if (m_handleType == QAbstractVideoBuffer::NoHandle) {
         glGenTextures(m_textureCount, m_textureIds);
      }
   }

   return error;
}

void QVideoSurfaceGlslPainter::stop()
{
   if (m_context) {
      m_context->makeCurrent();

      if (m_handleType != QAbstractVideoBuffer::GLTextureHandle) {
         glDeleteTextures(m_textureCount, m_textureIds);
      }
   }

   m_program.removeAllShaders();

   m_textureCount = 0;
   m_handleType = QAbstractVideoBuffer::NoHandle;

   QVideoSurfaceGLPainter::stop();
}

QAbstractVideoSurface::Error QVideoSurfaceGlslPainter::paint(
   const QRectF &target, QPainter *painter, const QRectF &source)
{
   if (! m_frame.isValid()) {
      painter->fillRect(target, Qt::black);
      return QAbstractVideoSurface::NoError;
   }

   const QAbstractVideoBuffer::HandleType h = m_frame.handleType();
   if (h == QAbstractVideoBuffer::NoHandle || h == QAbstractVideoBuffer::GLTextureHandle) {
      bool stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
      bool scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);

      painter->beginNativePainting();

      if (stencilTestEnabled) {
         glEnable(GL_STENCIL_TEST);
      }

      if (scissorTestEnabled) {
         glEnable(GL_SCISSOR_TEST);
      }

      const int width = QOpenGLContext::currentContext()->surface()->size().width();
      const int height = QOpenGLContext::currentContext()->surface()->size().height();

      const QTransform transform = painter->deviceTransform();

      const GLfloat wfactor = 2.0 / width;
      const GLfloat hfactor = -2.0 / height;

      const GLfloat positionMatrix[4][4] = {
         {
            /*(0,0)*/ GLfloat(wfactor * transform.m11() - transform.m13()),
            /*(0,1)*/ GLfloat(hfactor * transform.m12() + transform.m13()),
            /*(0,2)*/ 0.0,
            /*(0,3)*/ GLfloat(transform.m13())
         }, {
            /*(1,0)*/ GLfloat(wfactor * transform.m21() - transform.m23()),
            /*(1,1)*/ GLfloat(hfactor * transform.m22() + transform.m23()),
            /*(1,2)*/ 0.0,
            /*(1,3)*/ GLfloat(transform.m23())
         }, {
            /*(2,0)*/ 0.0,
            /*(2,1)*/ 0.0,
            /*(2,2)*/ -1.0,
            /*(2,3)*/ 0.0
         }, {
            /*(3,0)*/ GLfloat(wfactor * transform.dx() - transform.m33()),
            /*(3,1)*/ GLfloat(hfactor * transform.dy() + transform.m33()),
            /*(3,2)*/ 0.0,
            /*(3,3)*/ GLfloat(transform.m33())
         }
      };

      const GLfloat vertexCoordArray[] = {
         GLfloat(target.left()), GLfloat(target.bottom() + 1),
         GLfloat(target.right() + 1), GLfloat(target.bottom() + 1),
         GLfloat(target.left()), GLfloat(target.top()),
         GLfloat(target.right() + 1), GLfloat(target.top())
      };

      const GLfloat txLeft = m_mirrored ? source.right() / m_frameSize.width()
         : source.left() / m_frameSize.width();
      const GLfloat txRight = m_mirrored ? source.left() / m_frameSize.width()
         : source.right() / m_frameSize.width();
      const GLfloat txTop = m_scanLineDirection == QVideoSurfaceFormat::TopToBottom
         ? source.top() / m_frameSize.height()
         : source.bottom() / m_frameSize.height();
      const GLfloat txBottom = m_scanLineDirection == QVideoSurfaceFormat::TopToBottom
         ? source.bottom() / m_frameSize.height()
         : source.top() / m_frameSize.height();

      const GLfloat textureCoordArray[] = {
         txLeft, txBottom,
         txRight, txBottom,
         txLeft, txTop,
         txRight, txTop
      };

      m_program.bind();

      m_program.enableAttributeArray("vertexCoordArray");
      m_program.enableAttributeArray("textureCoordArray");
      m_program.setAttributeArray("vertexCoordArray", vertexCoordArray, 2);
      m_program.setAttributeArray("textureCoordArray", textureCoordArray, 2);
      m_program.setUniformValue("positionMatrix", positionMatrix);

      if (m_textureCount == 3) {
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, m_textureIds[0]);
         glActiveTexture(GL_TEXTURE1);
         glBindTexture(GL_TEXTURE_2D, m_textureIds[1]);
         glActiveTexture(GL_TEXTURE2);
         glBindTexture(GL_TEXTURE_2D, m_textureIds[2]);
         glActiveTexture(GL_TEXTURE0);

         m_program.setUniformValue("texY", 0);
         m_program.setUniformValue("texU", 1);
         m_program.setUniformValue("texV", 2);
      } else {
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, m_textureIds[0]);

         m_program.setUniformValue("texRgb", 0);
      }
      m_program.setUniformValue("colorMatrix", m_colorMatrix);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      m_program.release();

      painter->endNativePainting();

      return QAbstractVideoSurface::NoError;
   }

   return QVideoSurfaceGLPainter::paint(target, painter, source);
}

#endif

// internal
QPainterVideoSurface::QPainterVideoSurface(QObject *parent)
   : QAbstractVideoSurface(parent), m_painter(nullptr),

#if ! defined(QT_NO_OPENGL) && ! defined(QT_OPENGL_ES_1_CL) && ! defined(QT_OPENGL_ES_1)
     m_glContext(nullptr), m_shaderTypes(NoShaders), m_shaderType(NoShaders),
#endif

     m_brightness(0), m_contrast(0), m_hue(0), m_saturation(0), m_pixelFormat(QVideoFrame::Format_Invalid),
     m_colorsDirty(true), m_ready(false)
{
}

QPainterVideoSurface::~QPainterVideoSurface()
{
   if (isActive()) {
      m_painter->stop();
   }

   delete m_painter;
}

QList<QVideoFrame::PixelFormat> QPainterVideoSurface::supportedPixelFormats(
   QAbstractVideoBuffer::HandleType handleType) const
{
   if (!m_painter) {
      const_cast<QPainterVideoSurface *>(this)->createPainter();
   }

   return m_painter->supportedPixelFormats(handleType);
}

bool QPainterVideoSurface::isFormatSupported(const QVideoSurfaceFormat &format) const
{
   if (!m_painter) {
      const_cast<QPainterVideoSurface *>(this)->createPainter();
   }

   return m_painter->isFormatSupported(format);
}

/*!
*/
bool QPainterVideoSurface::start(const QVideoSurfaceFormat &format)
{
   if (isActive()) {
      m_painter->stop();
   }

   if (!m_painter) {
      createPainter();
   }

   if (format.frameSize().isEmpty()) {
      setError(UnsupportedFormatError);
   } else {
      QAbstractVideoSurface::Error error = m_painter->start(format);

      if (error != QAbstractVideoSurface::NoError) {
         setError(error);
      } else {
         m_pixelFormat = format.pixelFormat();
         m_frameSize = format.frameSize();
         m_sourceRect = format.viewport();
         m_colorsDirty = true;
         m_ready = true;

         return QAbstractVideoSurface::start(format);
      }
   }

   QAbstractVideoSurface::stop();

   return false;
}

/*!
*/
void QPainterVideoSurface::stop()
{
   if (isActive()) {
      m_painter->stop();
      m_ready = false;

      QAbstractVideoSurface::stop();
   }
}

/*!
*/
bool QPainterVideoSurface::present(const QVideoFrame &frame)
{
   if (!m_ready) {
      if (!isActive()) {
         setError(StoppedError);
      }
   } else if (frame.isValid()
      && (frame.pixelFormat() != m_pixelFormat || frame.size() != m_frameSize)) {
      setError(IncorrectFormatError);

      stop();
   } else {
      QAbstractVideoSurface::Error error = m_painter->setCurrentFrame(frame);

      if (error != QAbstractVideoSurface::NoError) {
         setError(error);

         stop();
      } else {
         m_ready = false;

         emit frameChanged();

         return true;
      }
   }
   return false;
}

/*!
*/
int QPainterVideoSurface::brightness() const
{
   return m_brightness;
}

/*!
*/
void QPainterVideoSurface::setBrightness(int brightness)
{
   m_brightness = brightness;

   m_colorsDirty = true;
}

/*!
*/
int QPainterVideoSurface::contrast() const
{
   return m_contrast;
}

/*!
*/
void QPainterVideoSurface::setContrast(int contrast)
{
   m_contrast = contrast;

   m_colorsDirty = true;
}

/*!
*/
int QPainterVideoSurface::hue() const
{
   return m_hue;
}

/*!
*/
void QPainterVideoSurface::setHue(int hue)
{
   m_hue = hue;

   m_colorsDirty = true;
}

/*!
*/
int QPainterVideoSurface::saturation() const
{
   return m_saturation;
}

/*!
*/
void QPainterVideoSurface::setSaturation(int saturation)
{
   m_saturation = saturation;

   m_colorsDirty = true;
}

/*!
*/
bool QPainterVideoSurface::isReady() const
{
   return m_ready;
}

/*!
*/
void QPainterVideoSurface::setReady(bool ready)
{
   m_ready = ready;
}

/*!
*/
void QPainterVideoSurface::paint(QPainter *painter, const QRectF &target, const QRectF &source)
{
   if (!isActive()) {
      painter->fillRect(target, QBrush(Qt::black));
   } else {
      if (m_colorsDirty) {
         m_painter->updateColors(m_brightness, m_contrast, m_hue, m_saturation);
         m_colorsDirty = false;
      }

      const QRectF sourceRect(
         m_sourceRect.x() + m_sourceRect.width() * source.x(),
         m_sourceRect.y() + m_sourceRect.height() * source.y(),
         m_sourceRect.width() * source.width(),
         m_sourceRect.height() * source.height());

      QAbstractVideoSurface::Error error = m_painter->paint(target, painter, sourceRect);

      if (error != QAbstractVideoSurface::NoError) {
         setError(error);

         stop();
      }
   }
}

/*!
    \fn QPainterVideoSurface::frameChanged()
*/

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_1_CL) && !defined(QT_OPENGL_ES_1)

/*!
*/
const QGLContext *QPainterVideoSurface::glContext() const
{
   return m_glContext;
}

/*!
*/
void QPainterVideoSurface::setGLContext(QGLContext *context)
{
   if (m_glContext == context) {
      return;
   }

   m_glContext = context;

   m_shaderTypes = NoShaders;

   if (m_glContext) {
      //Set a dynamic property to access the OpenGL context
      this->setProperty("GLContext", QVariant::fromValue<QObject *>(m_glContext->contextHandle()));

      m_glContext->makeCurrent();

      const QByteArray extensions(reinterpret_cast<const char *>(
            context->contextHandle()->functions()->glGetString(GL_EXTENSIONS)));

#if ! defined(QT_OPENGL_ES) && !defined(QT_OPENGL_DYNAMIC)
      if (extensions.contains("ARB_fragment_program")) {
         m_shaderTypes |= FragmentProgramShader;
      }
#endif
      if (QGLShaderProgram::hasOpenGLShaderPrograms(m_glContext)
#if !defined(QT_OPENGL_ES_2) && !defined(QT_OPENGL_DYNAMIC)
         && extensions.contains("ARB_shader_objects")
#endif
      ) {
         m_shaderTypes |= GlslShader;
      }
   }

   ShaderType type = (m_shaderType & m_shaderTypes)
      ? m_shaderType
      : NoShaders;

   if (type != m_shaderType || type != NoShaders) {
      m_shaderType = type;

      if (isActive()) {
         m_painter->stop();
         delete m_painter;
         m_painter = nullptr;
         m_ready = false;

         setError(ResourceError);
         QAbstractVideoSurface::stop();
      }
      emit supportedFormatsChanged();
   }
}

QPainterVideoSurface::ShaderTypes QPainterVideoSurface::supportedShaderTypes() const
{
   return m_shaderTypes;
}

QPainterVideoSurface::ShaderType QPainterVideoSurface::shaderType() const
{
   return m_shaderType;
}

void QPainterVideoSurface::setShaderType(ShaderType type)
{
   if (! (type & m_shaderTypes)) {
      type = NoShaders;
   }

   if (type != m_shaderType) {
      m_shaderType = type;

      if (isActive()) {
         m_painter->stop();
         delete m_painter;
         m_painter = nullptr;
         m_ready = false;

         setError(ResourceError);
         QAbstractVideoSurface::stop();
      } else {
         delete m_painter;
         m_painter = nullptr;
      }
      emit supportedFormatsChanged();
   }
}

#endif

void QPainterVideoSurface::viewportDestroyed()
{
   if (m_painter) {
      m_painter->viewportDestroyed();

      setError(ResourceError);
      stop();
      delete m_painter;
      m_painter = nullptr;
   }
}

void QPainterVideoSurface::createPainter()
{
   Q_ASSERT(!m_painter);

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_1_CL) && !defined(QT_OPENGL_ES_1)
   switch (m_shaderType) {

#if ! defined(QT_OPENGL_ES) && ! defined(QT_OPENGL_DYNAMIC)
      case FragmentProgramShader:
         Q_ASSERT(m_glContext);
         m_glContext->makeCurrent();
         m_painter = new QVideoSurfaceArbFpPainter(m_glContext);
         break;
#endif

      case GlslShader:
         Q_ASSERT(m_glContext);
         m_glContext->makeCurrent();
         m_painter = new QVideoSurfaceGlslPainter(m_glContext);
         break;

      default:
         m_painter = new QVideoSurfaceGenericPainter;
         break;
   }
#else
   m_painter = new QVideoSurfaceGenericPainter;
#endif
}
