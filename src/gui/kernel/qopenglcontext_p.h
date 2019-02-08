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

#ifndef QOPENGLCONTEXT_P_H
#define QOPENGLCONTEXT_P_H

#ifndef QT_NO_OPENGL

#include <qopengl.h>
#include <qopenglcontext.h>
#include <qmutex.h>

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QSet>

class QOpenGLFunctions;
class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOpenGLMultiGroupSharedResource;

class Q_GUI_EXPORT QOpenGLSharedResource
{
 public:
   QOpenGLSharedResource(QOpenGLContextGroup *group);
   virtual ~QOpenGLSharedResource() = 0;

   QOpenGLContextGroup *group() const {
      return m_group;
   }

   // schedule the resource for deletion at an appropriate time
   void free();

 protected:
   // the resource's share group no longer exists, invalidate the resource
   virtual void invalidateResource() = 0;

   // a valid context in the group is current, free the resource
   virtual void freeResource(QOpenGLContext *context) = 0;

 private:
   QOpenGLContextGroup *m_group;

   friend class QOpenGLContextGroup;
   friend class QOpenGLContextGroupPrivate;
   friend class QOpenGLMultiGroupSharedResource;

   Q_DISABLE_COPY(QOpenGLSharedResource)
};

class Q_GUI_EXPORT QOpenGLSharedResourceGuard : public QOpenGLSharedResource
{
 public:
   typedef void (*FreeResourceFunc)(QOpenGLFunctions *functions, GLuint id);
   QOpenGLSharedResourceGuard(QOpenGLContext *context, GLuint id, FreeResourceFunc func)
      : QOpenGLSharedResource(context->shareGroup())
      , m_id(id)
      , m_func(func) {
   }

   GLuint id() const {
      return m_id;
   }

 protected:
   void invalidateResource() override {
      m_id = 0;
   }

   void freeResource(QOpenGLContext *context) override;

 private:
   GLuint m_id;
   FreeResourceFunc m_func;
};

class Q_GUI_EXPORT QOpenGLContextGroupPrivate
{
   Q_DECLARE_PUBLIC(QOpenGLContextGroup)

 public:
   QOpenGLContextGroupPrivate()
      : m_context(0)
      , m_mutex(QMutex::Recursive)
      , m_refs(0) {
   }

   void addContext(QOpenGLContext *ctx);
   void removeContext(QOpenGLContext *ctx);

   void cleanup();

   void deletePendingResources(QOpenGLContext *ctx);

   QOpenGLContext *m_context;

   QList<QOpenGLContext *> m_shares;
   QMutex m_mutex;

   QHash<QOpenGLMultiGroupSharedResource *, QOpenGLSharedResource *> m_resources;
   QAtomicInt m_refs;

   QList<QOpenGLSharedResource *> m_sharedResources;
   QList<QOpenGLSharedResource *> m_pendingDeletion;

 protected:
   QOpenGLContextGroup *q_ptr;

};

class Q_GUI_EXPORT QOpenGLMultiGroupSharedResource
{
 public:
   QOpenGLMultiGroupSharedResource();
   ~QOpenGLMultiGroupSharedResource();

   void insert(QOpenGLContext *context, QOpenGLSharedResource *value);
   void cleanup(QOpenGLContextGroup *group, QOpenGLSharedResource *value);

   QOpenGLSharedResource *value(QOpenGLContext *context);

   QList<QOpenGLSharedResource *> resources() const;

   template <typename T>
   T *value(QOpenGLContext *context) {
      QOpenGLContextGroup *group = context->shareGroup();
      // Have to use our own mutex here, not the group's, since
      // m_groups has to be protected too against any concurrent access.
      QMutexLocker locker(&m_mutex);
      T *resource = static_cast<T *>(group->d_func()->m_resources.value(this, 0));
      if (!resource) {
         resource = new T(context);
         insert(context, resource);
      }
      return resource;
   }

 private:
   QAtomicInt active;
   QList<QOpenGLContextGroup *> m_groups;
   QMutex m_mutex;
};

class QPaintEngineEx;
class QOpenGLFunctions;
class QOpenGLTextureHelper;

class Q_GUI_EXPORT QOpenGLContextPrivate
{
   Q_DECLARE_PUBLIC(QOpenGLContext)

 public:
   QOpenGLContextPrivate()
      : qGLContextHandle(0)
      , qGLContextDeleteFunction(0)
      , platformGLContext(0)
      , shareContext(0)
      , shareGroup(0)
      , screen(0)
      , surface(0)
      , functions(0)
      , textureFunctions(0)
      , max_texture_size(-1)
      , workaround_brokenFBOReadBack(false)
      , workaround_brokenTexSubImage(false)
      , workaround_missingPrecisionQualifiers(false)
      , active_engine(0)
      , qgl_current_fbo_invalid(false)
      , qgl_current_fbo(nullptr)
      , defaultFboRedirect(0) {
      requestedFormat = QSurfaceFormat::defaultFormat();
   }

   virtual ~QOpenGLContextPrivate() {
      //do not delete the QOpenGLContext handle here as it is deleted in
      //QWidgetPrivate::deleteTLSysExtra()
   }

   mutable QHash<QOpenGLVersionProfile, QAbstractOpenGLFunctions *> versionFunctions;
   mutable QHash<QOpenGLVersionStatus, QOpenGLVersionFunctionsBackend *> versionFunctionsBackend;
   mutable QSet<QAbstractOpenGLFunctions *> externalVersionFunctions;

   void *qGLContextHandle;
   void (*qGLContextDeleteFunction)(void *handle);

   QSurfaceFormat requestedFormat;
   QPlatformOpenGLContext *platformGLContext;
   QOpenGLContext *shareContext;
   QOpenGLContextGroup *shareGroup;
   QScreen *screen;
   QSurface *surface;
   QOpenGLFunctions *functions;
   mutable QSet<QByteArray> extensionNames;
   QOpenGLTextureHelper *textureFunctions;

   GLint max_texture_size;

   bool workaround_brokenFBOReadBack;
   bool workaround_brokenTexSubImage;
   bool workaround_missingPrecisionQualifiers;

   QPaintEngineEx *active_engine;

   bool qgl_current_fbo_invalid;

   // Set and unset in QOpenGLFramebufferObject::bind()/unbind().
   // (Only meaningful for QOGLFBO since an FBO might be bound by other means)
   // Saves us from querying the driver for the current FBO in most paths.
   QOpenGLFramebufferObject *qgl_current_fbo;

   QVariant nativeHandle;
   GLuint defaultFboRedirect;

   static QOpenGLContext *setCurrentContext(QOpenGLContext *context);

   int maxTextureSize();

   static QOpenGLContextPrivate *get(QOpenGLContext *context) {
      return context ? context->d_func() : nullptr;
   }

#if ! defined(QT_NO_DEBUG)
   static bool toggleMakeCurrentTracker(QOpenGLContext *context, bool value) {
      QMutexLocker locker(&makeCurrentTrackerMutex);
      bool old = makeCurrentTracker.value(context, false);
      makeCurrentTracker.insert(context, value);
      return old;
   }
   static void cleanMakeCurrentTracker(QOpenGLContext *context) {
      QMutexLocker locker(&makeCurrentTrackerMutex);
      makeCurrentTracker.remove(context);
   }
   static QHash<QOpenGLContext *, bool> makeCurrentTracker;
   static QMutex makeCurrentTrackerMutex;
#endif

   void _q_screenDestroyed(QObject *object);

 protected:
   QOpenGLContext *q_ptr;

};

Q_GUI_EXPORT void qt_gl_set_global_share_context(QOpenGLContext *context);
Q_GUI_EXPORT QOpenGLContext *qt_gl_global_share_context();


#endif // QT_NO_OPENGL
#endif
