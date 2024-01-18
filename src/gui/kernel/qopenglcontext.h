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

#ifndef QOPENGLCONTEXT_H
#define QOPENGLCONTEXT_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qnamespace.h>
#include <qobject.h>
#include <qopengl.h>
#include <qopengl_versionfunctions.h>
#include <qhashfunc.h>
#include <qpair.h>
#include <qscopedpointer.h>
#include <qsurfaceformat.h>
#include <qvariant.h>

#ifdef __GLEW_H__
#if defined(Q_CC_GNU)
#warning qopenglfunctions.h is not compatible with GLEW, GLEW defines will be undefined
#warning To use GLEW with Qt, do not include <qopengl.h> or <QOpenGLFunctions> after glew.h
#endif
#endif

class QOpenGLContextPrivate;
class QOpenGLContextGroupPrivate;
class QOpenGLFunctions;
class QOpenGLExtraFunctions;
class QPlatformOpenGLContext;

class QScreen;
class QSurface;

class QOpenGLVersionProfilePrivate;
class QOpenGLTextureHelper;

class Q_GUI_EXPORT QOpenGLVersionProfile
{
 public:
   QOpenGLVersionProfile();
   explicit QOpenGLVersionProfile(const QSurfaceFormat &format);
   QOpenGLVersionProfile(const QOpenGLVersionProfile &other);
   ~QOpenGLVersionProfile();

   QOpenGLVersionProfile &operator=(const QOpenGLVersionProfile &rhs);

   QPair<int, int> version() const;
   void setVersion(int majorVersion, int minorVersion);

   QSurfaceFormat::OpenGLContextProfile profile() const;
   void setProfile(QSurfaceFormat::OpenGLContextProfile profile);

   bool hasProfiles() const;
   bool isLegacyVersion() const;
   bool isValid() const;

 private:
   QOpenGLVersionProfilePrivate *d;
};

inline uint qHash(const QOpenGLVersionProfile &v, uint seed = 0)
{
   return qHash(static_cast<int>(v.profile() * 1000)
         + v.version().first * 100 + v.version().second * 10, seed);
}

inline bool operator==(const QOpenGLVersionProfile &lhs, const QOpenGLVersionProfile &rhs)
{
   if (lhs.profile() != rhs.profile()) {
      return false;
   }

   return lhs.version() == rhs.version();
}

inline bool operator!=(const QOpenGLVersionProfile &lhs, const QOpenGLVersionProfile &rhs)
{
   return !operator==(lhs, rhs);
}

class Q_GUI_EXPORT QOpenGLContextGroup : public QObject
{
   GUI_CS_OBJECT(QOpenGLContextGroup)
   Q_DECLARE_PRIVATE(QOpenGLContextGroup)

 public:
   ~QOpenGLContextGroup();

   QList<QOpenGLContext *> shares() const;

   static QOpenGLContextGroup *currentContextGroup();

 private:
   QOpenGLContextGroup();

   friend class QOpenGLContext;
   friend class QOpenGLContextGroupResourceBase;
   friend class QOpenGLSharedResource;
   friend class QOpenGLMultiGroupSharedResource;

 protected:
   QScopedPointer<QOpenGLContextGroupPrivate> d_ptr;
};


class Q_GUI_EXPORT QOpenGLContext : public QObject
{
   GUI_CS_OBJECT(QOpenGLContext)
   Q_DECLARE_PRIVATE(QOpenGLContext)

 public:
   using FP_Void = void(*)();

   explicit QOpenGLContext(QObject *parent = nullptr);
   ~QOpenGLContext();

   void setFormat(const QSurfaceFormat &format);
   void setShareContext(QOpenGLContext *shareContext);
   void setScreen(QScreen *screen);
   void setNativeHandle(const QVariant &handle);

   bool create();
   bool isValid() const;

   QSurfaceFormat format() const;
   QOpenGLContext *shareContext() const;
   QOpenGLContextGroup *shareGroup() const;
   QScreen *screen() const;
   QVariant nativeHandle() const;

   GLuint defaultFramebufferObject() const;

   bool makeCurrent(QSurface *surface);
   void doneCurrent();

   void swapBuffers(QSurface *surface);
   FP_Void getProcAddress(const QByteArray &procName) const;

   QSurface *surface() const;

   static QOpenGLContext *currentContext();
   static bool areSharing(QOpenGLContext *first, QOpenGLContext *second);

   QPlatformOpenGLContext *handle() const;
   QPlatformOpenGLContext *shareHandle() const;

   QOpenGLFunctions *functions() const;
   QOpenGLExtraFunctions *extraFunctions() const;

   QAbstractOpenGLFunctions *versionFunctions(const QOpenGLVersionProfile &versionProfile = QOpenGLVersionProfile()) const;

   template <class TYPE>
   TYPE *versionFunctions() const {
      QOpenGLVersionProfile v = TYPE::versionProfile();
      return static_cast<TYPE *>(versionFunctions(v));
   }

   QSet<QByteArray> extensions() const;
   bool hasExtension(const QByteArray &extension) const;

   static void *openGLModuleHandle();

   enum OpenGLModuleType {
      LibGL,
      LibGLES
   };

   static OpenGLModuleType openGLModuleType();

   bool isOpenGLES() const;

   static bool supportsThreadedOpenGL();
   static QOpenGLContext *globalShareContext();

   GUI_CS_SIGNAL_1(Public, void aboutToBeDestroyed())
   GUI_CS_SIGNAL_2(aboutToBeDestroyed)

 private:
   QScopedPointer<QOpenGLContextPrivate> d_ptr;

   friend class QGLContext;
   friend class QGLPixelBuffer;
   friend class QOpenGLContextResourceBase;
   friend class QOpenGLPaintDevice;
   friend class QOpenGLGlyphTexture;
   friend class QOpenGLTextureGlyphCache;
   friend class QOpenGLEngineShaderManager;
   friend class QOpenGLFramebufferObject;
   friend class QOpenGLFramebufferObjectPrivate;
   friend class QOpenGL2PaintEngineEx;
   friend class QOpenGL2PaintEngineExPrivate;
   friend class QSGDistanceFieldGlyphCache;
   friend class QWidgetPrivate;
   friend class QAbstractOpenGLFunctionsPrivate;
   friend class QOpenGLTexturePrivate;

   void *qGLContextHandle() const;
   void setQGLContextHandle(void *handle, void (*qGLContextDeleteFunction)(void *));
   void deleteQGLContext();

   QOpenGLVersionFunctionsBackend *functionsBackend(const QOpenGLVersionStatus &v) const;
   void insertFunctionsBackend(const QOpenGLVersionStatus &v,
      QOpenGLVersionFunctionsBackend *backend);
   void removeFunctionsBackend(const QOpenGLVersionStatus &v);
   void insertExternalFunctions(QAbstractOpenGLFunctions *f);
   void removeExternalFunctions(QAbstractOpenGLFunctions *f);

   QOpenGLTextureHelper *textureFunctions() const;
   void setTextureFunctions(QOpenGLTextureHelper *textureFuncs);

   void destroy();

   GUI_CS_SLOT_1(Private, void _q_screenDestroyed(QObject *object))
   GUI_CS_SLOT_2(_q_screenDestroyed)
};

#endif // QT_NO_OPENGL

#endif
