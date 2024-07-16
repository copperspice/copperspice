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

#include <qplatform_backingstore.h>

#include <qpixmap.h>
#include <qwindow.h>
#include <qwindow_p.h>

#include <qplatform_graphicsbuffer.h>
#include <qplatform_graphicsbufferhelper.h>

#include <qopengl.h>
#include <qopenglcontext.h>
#include <qmatrix4x4.h>
#include <qopengl_shaderprogram.h>
#include <qopenglcontext.h>
#include <qopenglfunctions.h>

#ifndef QT_NO_OPENGL
#include <qopengl_textureblitter_p.h>
#endif

#ifndef GL_TEXTURE_BASE_LEVEL
#define GL_TEXTURE_BASE_LEVEL             0x813C
#endif

#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL              0x813D
#endif

#ifndef GL_UNPACK_ROW_LENGTH
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#endif

#ifndef GL_RGB10_A2
#define GL_RGB10_A2                       0x8059
#endif

#ifndef GL_UNSIGNED_INT_2_10_10_10_REV
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#endif

class QPlatformBackingStorePrivate
{
 public:
   QPlatformBackingStorePrivate(QWindow *w)
      : window(w)
#ifndef QT_NO_OPENGL
      , textureId(0), blitter(nullptr)
#endif
   {
   }

   ~QPlatformBackingStorePrivate() {
#ifndef QT_NO_OPENGL
      QOpenGLContext *ctx = QOpenGLContext::currentContext();

      if (ctx) {
         if (textureId) {
            ctx->functions()->glDeleteTextures(1, &textureId);
         }

         if (blitter) {
            blitter->destroy();
         }

      } else if (textureId || blitter) {
         qWarning("QPlatformBackingStore() No current OpenGL context in destructor, resources not released");
      }
      delete blitter;
#endif
   }

   QWindow *window;

#ifndef QT_NO_OPENGL
   mutable GLuint textureId;
   mutable QSize textureSize;
   mutable bool needsSwizzle;
   mutable bool premultiplied;
   QOpenGLTextureBlitter *blitter;
#endif
};

#ifndef QT_NO_OPENGL

struct QBackingstoreTextureInfo {
   void *source; // may be null
   GLuint textureId;
   QRect rect;
   QRect clipRect;
   QPlatformTextureList::Flags flags;
};

class QPlatformTextureListPrivate
{
 public:
   QPlatformTextureListPrivate()
      : locked(false) {
   }

   QVector<QBackingstoreTextureInfo> textures;
   bool locked;
};

QPlatformTextureList::QPlatformTextureList(QObject *parent)
   : QObject(parent), d_ptr(new QPlatformTextureListPrivate)
{
}

QPlatformTextureList::~QPlatformTextureList()
{
}

int QPlatformTextureList::count() const
{
   Q_D(const QPlatformTextureList);
   return d->textures.count();
}

GLuint QPlatformTextureList::textureId(int index) const
{
   Q_D(const QPlatformTextureList);
   return d->textures.at(index).textureId;
}

void *QPlatformTextureList::source(int index)
{
   Q_D(const QPlatformTextureList);
   return d->textures.at(index).source;
}

QPlatformTextureList::Flags QPlatformTextureList::flags(int index) const
{
   Q_D(const QPlatformTextureList);
   return d->textures.at(index).flags;
}

QRect QPlatformTextureList::geometry(int index) const
{
   Q_D(const QPlatformTextureList);
   return d->textures.at(index).rect;
}

QRect QPlatformTextureList::clipRect(int index) const
{
   Q_D(const QPlatformTextureList);
   return d->textures.at(index).clipRect;
}

void QPlatformTextureList::lock(bool on)
{
   Q_D(QPlatformTextureList);
   if (on != d->locked) {
      d->locked = on;
      emit locked(on);
   }
}

bool QPlatformTextureList::isLocked() const
{
   Q_D(const QPlatformTextureList);
   return d->locked;
}

void QPlatformTextureList::appendTexture(void *source, GLuint textureId, const QRect &geometry,
   const QRect &clipRect, Flags flags)
{
   Q_D(QPlatformTextureList);
   QBackingstoreTextureInfo bi;
   bi.source = source;
   bi.textureId = textureId;
   bi.rect = geometry;
   bi.clipRect = clipRect;
   bi.flags = flags;
   d->textures.append(bi);
}

void QPlatformTextureList::clear()
{
   Q_D(QPlatformTextureList);
   d->textures.clear();
}
#endif // QT_NO_OPENGL

#ifndef QT_NO_OPENGL

static inline QRect deviceRect(const QRect &rect, QWindow *window)
{
   QRect deviceRect(rect.topLeft() * window->devicePixelRatio(),
      rect.size() * window->devicePixelRatio());
   return deviceRect;
}

static inline QPoint deviceOffset(const QPoint &pt, QWindow *window)
{
   return pt * window->devicePixelRatio();
}

static QRegion deviceRegion(const QRegion &region, QWindow *window, const QPoint &offset)
{
   if (offset.isNull() && window->devicePixelRatio() <= 1) {
      return region;
   }

   QVector<QRect> rects;
   const QVector<QRect> regionRects = region.rects();
   rects.reserve(regionRects.count());

   for (const QRect &rect : regionRects) {
      rects.append(deviceRect(rect.translated(offset), window));
   }

   QRegion deviceRegion;
   deviceRegion.setRects(rects.constData(), rects.count());
   return deviceRegion;
}

static inline QRect toBottomLeftRect(const QRect &topLeftRect, int windowHeight)
{
   return QRect(topLeftRect.x(), windowHeight - topLeftRect.bottomRight().y() - 1,
         topLeftRect.width(), topLeftRect.height());
}

static void blitTextureForWidget(const QPlatformTextureList *textures, int idx, QWindow *window, const QRect &deviceWindowRect,
   QOpenGLTextureBlitter *blitter, const QPoint &offset)
{
   const QRect clipRect = textures->clipRect(idx);
   if (clipRect.isEmpty()) {
      return;
   }

   QRect rectInWindow = textures->geometry(idx);
   // relative to the TLW, not necessarily our window (if the flush is for a native child widget), have to adjust
   rectInWindow.translate(-offset);

   const QRect clippedRectInWindow = rectInWindow & clipRect.translated(rectInWindow.topLeft());
   const QRect srcRect = toBottomLeftRect(clipRect, rectInWindow.height());

   const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(deviceRect(clippedRectInWindow, window),
         deviceWindowRect);

   const QMatrix3x3 source = QOpenGLTextureBlitter::sourceTransform(deviceRect(srcRect, window),
         deviceRect(rectInWindow, window).size(),
         QOpenGLTextureBlitter::OriginBottomLeft);

   blitter->blit(textures->textureId(idx), target, source);
}

void QPlatformBackingStore::composeAndFlush(QWindow *window, const QRegion &region,
   const QPoint &offset, QPlatformTextureList *textures, QOpenGLContext *context, bool translucentBackground)
{
   if (! qt_window_private(window)->receivedExpose) {
      return;
   }

   if (! context->makeCurrent(window)) {
      qWarning("QPlatformBackingStore::composeAndFlush() Unable to set the current OpenGL context");
      return;
   }

   QWindowPrivate::get(window)->lastComposeTime.start();

   QOpenGLFunctions *funcs = context->functions();
   funcs->glViewport(0, 0, window->width() * window->devicePixelRatio(), window->height() * window->devicePixelRatio());
   funcs->glClearColor(0, 0, 0, translucentBackground ? 0 : 1);
   funcs->glClear(GL_COLOR_BUFFER_BIT);

   if (!d_ptr->blitter) {
      d_ptr->blitter = new QOpenGLTextureBlitter;
      d_ptr->blitter->create();
   }

   d_ptr->blitter->bind();

   const QRect deviceWindowRect = deviceRect(QRect(QPoint(), window->size()), window);
   const QPoint deviceWindowOffset = deviceOffset(offset, window);

   // Textures for renderToTexture widgets.
   for (int i = 0; i < textures->count(); ++i) {
      if (!textures->flags(i).testFlag(QPlatformTextureList::StacksOnTop)) {
         blitTextureForWidget(textures, i, window, deviceWindowRect, d_ptr->blitter, offset);
      }
   }

   // Backingstore texture with the normal widgets.
   GLuint textureId = 0;
   QOpenGLTextureBlitter::Origin origin = QOpenGLTextureBlitter::OriginTopLeft;
   if (QPlatformGraphicsBuffer *graphicsBuffer = this->graphicsBuffer()) {
      if (graphicsBuffer->size() != d_ptr->textureSize) {
         if (d_ptr->textureId) {
            funcs->glDeleteTextures(1, &d_ptr->textureId);
         }
         funcs->glGenTextures(1, &d_ptr->textureId);
         funcs->glBindTexture(GL_TEXTURE_2D, d_ptr->textureId);
         QOpenGLContext *ctx = QOpenGLContext::currentContext();
         if (!ctx->isOpenGLES() || ctx->format().majorVersion() >= 3) {
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
         }
         funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

         if (QPlatformGraphicsBufferHelper::lockAndBindToTexture(graphicsBuffer, &d_ptr->needsSwizzle, &d_ptr->premultiplied)) {
            d_ptr->textureSize = graphicsBuffer->size();
         } else {
            d_ptr->textureSize = QSize(0, 0);
         }

         graphicsBuffer->unlock();
      } else if (!region.isEmpty()) {
         funcs->glBindTexture(GL_TEXTURE_2D, d_ptr->textureId);
         QPlatformGraphicsBufferHelper::lockAndBindToTexture(graphicsBuffer, &d_ptr->needsSwizzle, &d_ptr->premultiplied);
      }

      if (graphicsBuffer->origin() == QPlatformGraphicsBuffer::OriginBottomLeft) {
         origin = QOpenGLTextureBlitter::OriginBottomLeft;
      }
      textureId = d_ptr->textureId;

   } else {
      TextureFlags flags = Qt::EmptyFlag;
      textureId = toTexture(deviceRegion(region, window, offset), &d_ptr->textureSize, &flags);
      d_ptr->needsSwizzle = (flags & TextureSwizzle) != 0;
      d_ptr->premultiplied = (flags & TexturePremultiplied) != 0;

      if (flags & TextureFlip) {
         origin = QOpenGLTextureBlitter::OriginBottomLeft;
      }
   }

   funcs->glEnable(GL_BLEND);
   if (d_ptr->premultiplied) {
      funcs->glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
   } else {
      funcs->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
   }

   if (textureId) {
      if (d_ptr->needsSwizzle) {
         d_ptr->blitter->setSwizzleRB(true);
      }
      // The backingstore is for the entire tlw.
      // In case of native children offset tells the position relative to the tlw.
      const QRect srcRect = toBottomLeftRect(deviceWindowRect.translated(deviceWindowOffset), d_ptr->textureSize.height());
      const QMatrix3x3 source = QOpenGLTextureBlitter::sourceTransform(srcRect,
            d_ptr->textureSize,
            origin);
      d_ptr->blitter->blit(textureId, QMatrix4x4(), source);
      if (d_ptr->needsSwizzle) {
         d_ptr->blitter->setSwizzleRB(false);
      }
   }

   // Textures for renderToTexture widgets that have WA_AlwaysStackOnTop set.
   for (int i = 0; i < textures->count(); ++i) {
      if (textures->flags(i).testFlag(QPlatformTextureList::StacksOnTop)) {
         blitTextureForWidget(textures, i, window, deviceWindowRect, d_ptr->blitter, offset);
      }
   }

   funcs->glDisable(GL_BLEND);
   d_ptr->blitter->release();

   context->swapBuffers(window);
}

QImage QPlatformBackingStore::toImage() const
{
   return QImage();
}

GLuint QPlatformBackingStore::toTexture(const QRegion &dirtyRegion, QSize *textureSize, TextureFlags *flags) const
{
   Q_ASSERT(textureSize);
   Q_ASSERT(flags);

   QImage image = toImage();
   QSize imageSize = image.size();

   QOpenGLContext *ctx = QOpenGLContext::currentContext();
   GLenum internalFormat = GL_RGBA;
   GLuint pixelType = GL_UNSIGNED_BYTE;

   bool needsConversion = false;
   *flags = Qt::EmptyFlag;

   switch (image.format()) {
      case QImage::Format_ARGB32_Premultiplied:
         *flags |= TexturePremultiplied;
         [[fallthrough]];

      case QImage::Format_RGB32:
      case QImage::Format_ARGB32:
         *flags |= TextureSwizzle;
         break;

      case QImage::Format_RGBA8888_Premultiplied:
         *flags |= TexturePremultiplied;
         [[fallthrough]];

      case QImage::Format_RGBX8888:
      case QImage::Format_RGBA8888:
         break;

      case QImage::Format_BGR30:
      case QImage::Format_A2BGR30_Premultiplied:
         if (!ctx->isOpenGLES() || ctx->format().majorVersion() >= 3) {
            pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;
            internalFormat = GL_RGB10_A2;
            *flags |= TexturePremultiplied;
         } else {
            needsConversion = true;
         }
         break;

      case QImage::Format_RGB30:
      case QImage::Format_A2RGB30_Premultiplied:
         if (!ctx->isOpenGLES() || ctx->format().majorVersion() >= 3) {
            pixelType = GL_UNSIGNED_INT_2_10_10_10_REV;
            internalFormat = GL_RGB10_A2;
            *flags |= TextureSwizzle | TexturePremultiplied;
         } else {
            needsConversion = true;
         }
         break;

      default:
         needsConversion = true;
         break;
   }
   if (imageSize.isEmpty()) {
      *textureSize = imageSize;
      return 0;
   }

   // Must rely on the input only, not d_ptr.
   // With the default composeAndFlush() textureSize is &d_ptr->textureSize.
   bool resized = *textureSize != imageSize;
   if (dirtyRegion.isEmpty() && !resized) {
      return d_ptr->textureId;
   }

   *textureSize = imageSize;

   if (needsConversion) {
      image = image.convertToFormat(QImage::Format_RGBA8888);
   }

   // The image provided by the backingstore may have a stride larger than width * 4, for
   // instance on platforms that manually implement client-side decorations.
   static constexpr const int bytesPerPixel  = 4;

   const int strideInPixels      = image.bytesPerLine() / bytesPerPixel;
   const bool hasUnpackRowLength = ! ctx->isOpenGLES() || ctx->format().majorVersion() >= 3;

   QOpenGLFunctions *funcs = ctx->functions();

   if (hasUnpackRowLength) {
      funcs->glPixelStorei(GL_UNPACK_ROW_LENGTH, strideInPixels);
   } else if (strideInPixels != image.width()) {
      // No UNPACK_ROW_LENGTH on ES 2.0 and yet we would need it. This case is typically
      // hit with QtWayland which is rarely used in combination with a ES2.0-only GL
      // implementation.  Therefore, accept the performance hit and do a copy.
      image = image.copy();
   }

   if (resized) {
      if (d_ptr->textureId) {
         funcs->glDeleteTextures(1, &d_ptr->textureId);
      }
      funcs->glGenTextures(1, &d_ptr->textureId);
      funcs->glBindTexture(GL_TEXTURE_2D, d_ptr->textureId);
      if (!ctx->isOpenGLES() || ctx->format().majorVersion() >= 3) {
         funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
         funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
      }
      funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      funcs->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      funcs->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imageSize.width(), imageSize.height(), 0, GL_RGBA, pixelType,
         const_cast<uchar *>(image.constBits()));
   } else {
      funcs->glBindTexture(GL_TEXTURE_2D, d_ptr->textureId);
      QRect imageRect = image.rect();
      QRect rect = dirtyRegion.boundingRect() & imageRect;

      if (hasUnpackRowLength) {
         funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x(), rect.y(), rect.width(), rect.height(), GL_RGBA, pixelType,
            image.constScanLine(rect.y()) + rect.x() * bytesPerPixel);
      } else {
         // if the rect is wide enough it's cheaper to just
         // extend it instead of doing an image copy
         if (rect.width() >= imageRect.width() / 2) {
            rect.setX(0);
            rect.setWidth(imageRect.width());
         }

         // if the sub-rect is full-width we can pass the image data directly to
         // OpenGL instead of copying, since there's no gap between scanlines

         if (rect.width() == imageRect.width()) {
            funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, rect.y(), rect.width(), rect.height(), GL_RGBA, pixelType,
               image.constScanLine(rect.y()));
         } else {
            funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x(), rect.y(), rect.width(), rect.height(), GL_RGBA, pixelType,
               image.copy(rect).constBits());
         }
      }
   }

   if (hasUnpackRowLength) {
      funcs->glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
   }

   return d_ptr->textureId;
}
#endif // QT_NO_OPENGL


QPlatformBackingStore::QPlatformBackingStore(QWindow *window)
   : d_ptr(new QPlatformBackingStorePrivate(window))
{
}

QPlatformBackingStore::~QPlatformBackingStore()
{
   delete d_ptr;
}

QWindow *QPlatformBackingStore::window() const
{
   return d_ptr->window;
}

void QPlatformBackingStore::beginPaint(const QRegion &)
{
}

void QPlatformBackingStore::endPaint()
{
}

QPlatformGraphicsBuffer *QPlatformBackingStore::graphicsBuffer() const
{
   return nullptr;
}

bool QPlatformBackingStore::scroll(const QRegion &area, int dx, int dy)
{
   (void) area;
   (void) dx;
   (void) dy;

   return false;
}


