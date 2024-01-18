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

#ifndef QPLATFORM_BACKINGSTORE_H
#define QPLATFORM_BACKINGSTORE_H

#include <qrect.h>
#include <qobject.h>
#include <qwindow.h>
#include <qregion.h>
#include <qopengl.h>

class QRegion;
class QRect;
class QPoint;
class QImage;
class QPlatformBackingStorePrivate;
class QPlatformWindow;
class QPlatformTextureList;
class QPlatformTextureListPrivate;
class QOpenGLContext;
class QPlatformGraphicsBuffer;

#ifndef QT_NO_OPENGL
class Q_GUI_EXPORT QPlatformTextureList : public QObject
{
   GUI_CS_OBJECT(QPlatformTextureList)
   Q_DECLARE_PRIVATE(QPlatformTextureList)

 public:
   enum Flag {
      StacksOnTop = 0x01
   };
   using Flags = QFlags<Flag>;

   explicit QPlatformTextureList(QObject *parent = nullptr);
   ~QPlatformTextureList();

   int count() const;
   bool isEmpty() const {
      return count() == 0;
   }

   GLuint textureId(int index) const;
   QRect geometry(int index) const;
   QRect clipRect(int index) const;
   void *source(int index);
   Flags flags(int index) const;
   void lock(bool on);
   bool isLocked() const;

   void appendTexture(void *source, GLuint texture_id, const QRect &geometry,
      const QRect &clipRect = QRect(), Flags flags = Qt::EmptyFlag);

   void clear();

   GUI_CS_SIGNAL_1(Public, void locked(bool isLocked))
   GUI_CS_SIGNAL_2(locked, isLocked)

 protected:
   QScopedPointer<QPlatformTextureListPrivate> d_ptr;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QPlatformTextureList::Flags)

#endif

class Q_GUI_EXPORT QPlatformBackingStore
{
 public:
   explicit QPlatformBackingStore(QWindow *window);
   virtual ~QPlatformBackingStore();

   QWindow *window() const;

   virtual QPaintDevice *paintDevice() = 0;

   // 'window' can be a child window, in which case 'region' is in child window coordinates and
   // offset is the (child) window's offset in relation to the window surface.
   virtual void flush(QWindow *window, const QRegion &region, const QPoint &offset) = 0;

#ifndef QT_NO_OPENGL
   virtual void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
      QPlatformTextureList *textures, QOpenGLContext *context,
      bool translucentBackground);

   virtual QImage toImage() const;
   enum TextureFlag {
      TextureSwizzle = 0x01,
      TextureFlip = 0x02,
      TexturePremultiplied = 0x04,
   };
   using TextureFlags = QFlags<TextureFlag>;

   virtual GLuint toTexture(const QRegion &dirtyRegion, QSize *textureSize, TextureFlags *flags) const;
#endif

   virtual QPlatformGraphicsBuffer *graphicsBuffer() const;

   virtual void resize(const QSize &size, const QRegion &staticContents) = 0;

   virtual bool scroll(const QRegion &area, int dx, int dy);

   virtual void beginPaint(const QRegion &);
   virtual void endPaint();

 private:
   QPlatformBackingStorePrivate *d_ptr;
};

#ifndef QT_NO_OPENGL
Q_DECLARE_OPERATORS_FOR_FLAGS(QPlatformBackingStore::TextureFlags)
#endif

#endif