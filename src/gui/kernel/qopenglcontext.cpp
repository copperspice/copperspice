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

#include <qopenglcontext.h>
#include <qopenglcontext_p.h>

#include <qdebug.h>
#include <qplatform_integration.h>
#include <qplatform_nativeinterface.h>
#include <qplatform_openglcontext.h>
#include <qscreen.h>
#include <qthread.h>
#include <qthreadstorage.h>
#include <qwindow.h>

#include <qapplication_p.h>
#include <qopengl_extensions_p.h>
#include <qopengl_p.h>
#include <qopengl_texturehelper_p.h>
#include <qopengl_version_functions_p.h>
#include <qwindow_p.h>

#ifndef QT_OPENGL_ES_2
#include <QOpenGLFunctions_1_0>
#include <QOpenGLFunctions_3_2_Core>
#endif

class QOpenGLVersionProfilePrivate
{
 public:
   QOpenGLVersionProfilePrivate()
      : majorVersion(0), minorVersion(0), profile(QSurfaceFormat::NoProfile)
   { }

   int majorVersion;
   int minorVersion;
   QSurfaceFormat::OpenGLContextProfile profile;
};

QOpenGLVersionProfile::QOpenGLVersionProfile()
   : d(new QOpenGLVersionProfilePrivate)
{
}

QOpenGLVersionProfile::QOpenGLVersionProfile(const QSurfaceFormat &format)
   : d(new QOpenGLVersionProfilePrivate)
{
   d->majorVersion = format.majorVersion();
   d->minorVersion = format.minorVersion();
   d->profile = format.profile();
}

QOpenGLVersionProfile::QOpenGLVersionProfile(const QOpenGLVersionProfile &other)
   : d(new QOpenGLVersionProfilePrivate)
{
   *d = *(other.d);
}

QOpenGLVersionProfile::~QOpenGLVersionProfile()
{
   delete d;
}

QOpenGLVersionProfile &QOpenGLVersionProfile::operator=(const QOpenGLVersionProfile &rhs)
{
   if (this == &rhs) {
      return *this;
   }
   *d = *(rhs.d);
   return *this;
}

QPair<int, int> QOpenGLVersionProfile::version() const
{
   return qMakePair( d->majorVersion, d->minorVersion);
}

void QOpenGLVersionProfile::setVersion(int majorVersion, int minorVersion)
{
   d->majorVersion = majorVersion;
   d->minorVersion = minorVersion;
}

QSurfaceFormat::OpenGLContextProfile QOpenGLVersionProfile::profile() const
{
   return d->profile;
}

void QOpenGLVersionProfile::setProfile(QSurfaceFormat::OpenGLContextProfile profile)
{
   d->profile = profile;
}

bool QOpenGLVersionProfile::hasProfiles() const
{
   return ( d->majorVersion > 3 || (d->majorVersion == 3 && d->minorVersion > 1));
}

bool QOpenGLVersionProfile::isLegacyVersion() const
{
   return (d->majorVersion < 3 || (d->majorVersion == 3 && d->minorVersion == 0));
}

bool QOpenGLVersionProfile::isValid() const
{
   return d->majorVersion > 0 && d->minorVersion >= 0;
}

class QGuiGLThreadContext
{
 public:
   QGuiGLThreadContext()
      : context(nullptr)
   {
   }

   ~QGuiGLThreadContext() {
      if (context) {
         context->doneCurrent();
      }
   }

   QOpenGLContext *context;
};

static QThreadStorage<QGuiGLThreadContext *> *qwindow_context_storage()
{
   static QThreadStorage<QGuiGLThreadContext *> retval;
   return &retval;
}

static QOpenGLContext *global_share_context = nullptr;

void qt_gl_set_global_share_context(QOpenGLContext *context)
{
   global_share_context = context;
}

// internal
QOpenGLContext *qt_gl_global_share_context()
{
   return global_share_context;
}

// internal
QOpenGLContext *QOpenGLContextPrivate::setCurrentContext(QOpenGLContext *context)
{
   QGuiGLThreadContext *threadContext = qwindow_context_storage()->localData();

  if (! threadContext) {
      if (! QThread::currentThread()) {
         qWarning("QOpenGLContext::setCurrentContext() CurrentContext requires an active QThread");
         return nullptr;
      }

      threadContext = new QGuiGLThreadContext;
      qwindow_context_storage()->setLocalData(threadContext);
   }

   QOpenGLContext *previous = threadContext->context;
   threadContext->context = context;
   return previous;
}

int QOpenGLContextPrivate::maxTextureSize()
{
   if (max_texture_size != -1) {
      return max_texture_size;
   }

   Q_Q(QOpenGLContext);
   QOpenGLFunctions *funcs = q->functions();
   funcs->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

#ifndef QT_OPENGL_ES
   if (! q->isOpenGLES()) {
      GLenum proxy = GL_PROXY_TEXTURE_2D;

      GLint size;
      GLint next = 64;
      funcs->glTexImage2D(proxy, 0, GL_RGBA, next, next, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

      QOpenGLFunctions_1_0 *gl1funcs      = nullptr;
      QOpenGLFunctions_3_2_Core *gl3funcs = nullptr;

      if (q->format().profile() == QSurfaceFormat::CoreProfile) {
         gl3funcs = q->versionFunctions<QOpenGLFunctions_3_2_Core>();
      } else {
         gl1funcs = q->versionFunctions<QOpenGLFunctions_1_0>();
      }

      Q_ASSERT(gl1funcs || gl3funcs);

      if (gl1funcs) {
         gl1funcs->glGetTexLevelParameteriv(proxy, 0, GL_TEXTURE_WIDTH, &size);
      } else {
         gl3funcs->glGetTexLevelParameteriv(proxy, 0, GL_TEXTURE_WIDTH, &size);
      }

      if (size == 0) {
         return max_texture_size;
      }
      do {
         size = next;
         next = size * 2;

         if (next > max_texture_size) {
            break;
         }
         funcs->glTexImage2D(proxy, 0, GL_RGBA, next, next, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
         if (gl1funcs) {
            gl1funcs->glGetTexLevelParameteriv(proxy, 0, GL_TEXTURE_WIDTH, &next);
         } else {
            gl3funcs->glGetTexLevelParameteriv(proxy, 0, GL_TEXTURE_WIDTH, &next);
         }

      } while (next > size);

      max_texture_size = size;
   }
#endif // QT_OPENGL_ES

   return max_texture_size;
}

QOpenGLContext *QOpenGLContext::currentContext()
{
   QGuiGLThreadContext *threadContext = qwindow_context_storage()->localData();

   if (threadContext) {
      return threadContext->context;
   }

   return nullptr;
}

bool QOpenGLContext::areSharing(QOpenGLContext *first, QOpenGLContext *second)
{
   return first->shareGroup() == second->shareGroup();
}

// internal
QPlatformOpenGLContext *QOpenGLContext::handle() const
{
   Q_D(const QOpenGLContext);
   return d->platformGLContext;
}

// internal
QPlatformOpenGLContext *QOpenGLContext::shareHandle() const
{
   Q_D(const QOpenGLContext);

   if (d->shareContext) {
      return d->shareContext->handle();
   }

   return nullptr;
}

QOpenGLContext::QOpenGLContext(QObject *parent)
   : QObject(parent), d_ptr(new QOpenGLContextPrivate())
{
   d_ptr->q_ptr = this;
   setScreen(QGuiApplication::primaryScreen());
}

void QOpenGLContext::setFormat(const QSurfaceFormat &format)
{
   Q_D(QOpenGLContext);
   d->requestedFormat = format;
}

void QOpenGLContext::setShareContext(QOpenGLContext *shareContext)
{
   Q_D(QOpenGLContext);
   d->shareContext = shareContext;
}

void QOpenGLContext::setScreen(QScreen *screen)
{
   Q_D(QOpenGLContext);

   if (d->screen) {
      disconnect(d->screen, &QScreen::destroyed, this, &QOpenGLContext::_q_screenDestroyed);
   }

   d->screen = screen;

   if (!d->screen) {
      d->screen = QGuiApplication::primaryScreen();
   }

   if (d->screen) {
      connect(d->screen, &QScreen::destroyed, this, &QOpenGLContext::_q_screenDestroyed);
   }
}

void QOpenGLContextPrivate::_q_screenDestroyed(QObject *object)
{
   Q_Q(QOpenGLContext);

   if (object == static_cast<QObject *>(screen)) {
      screen = nullptr;
      q->setScreen(nullptr);
   }
}

void QOpenGLContext::setNativeHandle(const QVariant &handle)
{
   Q_D(QOpenGLContext);
   d->nativeHandle = handle;
}

QVariant QOpenGLContext::nativeHandle() const
{
   Q_D(const QOpenGLContext);
   return d->nativeHandle;
}

bool QOpenGLContext::create()
{
   Q_D(QOpenGLContext);

   if (d->platformGLContext) {
      destroy();
   }

   d->platformGLContext = QGuiApplicationPrivate::platformIntegration()->createPlatformOpenGLContext(this);

   if (! d->platformGLContext) {
      return false;
   }

   d->platformGLContext->initialize();
   d->platformGLContext->setContext(this);

   if (! d->platformGLContext->isSharing()) {
      d->shareContext = nullptr;
   }

   d->shareGroup = d->shareContext ? d->shareContext->shareGroup() : new QOpenGLContextGroup;
   d->shareGroup->d_func()->addContext(this);

   return isValid();
}

void QOpenGLContext::destroy()
{
   deleteQGLContext();

   Q_D(QOpenGLContext);

   if (d->platformGLContext) {
      emit aboutToBeDestroyed();
   }

   if (QOpenGLContext::currentContext() == this) {
      doneCurrent();
   }

   if (d->shareGroup) {
      d->shareGroup->d_func()->removeContext(this);
   }

   d->shareGroup = nullptr;

   delete d->platformGLContext;
   d->platformGLContext = nullptr;

   delete d->functions;
   d->functions = nullptr;

   for (QAbstractOpenGLFunctions *func : d->externalVersionFunctions) {
      QAbstractOpenGLFunctionsPrivate *func_d = QAbstractOpenGLFunctionsPrivate::get(func);
      func_d->owningContext = nullptr;
      func_d->initialized = false;
   }

   d->externalVersionFunctions.clear();
   qDeleteAll(d->versionFunctions);
   d->versionFunctions.clear();
   qDeleteAll(d->versionFunctionsBackend);
   d->versionFunctionsBackend.clear();

   delete d->textureFunctions;
   d->textureFunctions = nullptr;

   d->nativeHandle = QVariant();
}

QOpenGLContext::~QOpenGLContext()
{
   destroy();
}

bool QOpenGLContext::isValid() const
{
   Q_D(const QOpenGLContext);
   return d->platformGLContext && d->platformGLContext->isValid();
}

QOpenGLFunctions *QOpenGLContext::functions() const
{
   Q_D(const QOpenGLContext);

   if (!d->functions) {
      const_cast<QOpenGLFunctions *&>(d->functions) = new QOpenGLExtensions(QOpenGLContext::currentContext());
   }

   return d->functions;
}

QOpenGLExtraFunctions *QOpenGLContext::extraFunctions() const
{
   return static_cast<QOpenGLExtraFunctions *>(functions());
}

QAbstractOpenGLFunctions *QOpenGLContext::versionFunctions(const QOpenGLVersionProfile &versionProfile) const
{
#ifndef QT_OPENGL_ES_2

   if (isOpenGLES()) {
      qWarning("QOpenGLContext::versionFunctions() Method not supported on OpenGL ES");
      return nullptr;
   }
#endif

   Q_D(const QOpenGLContext);
   const QSurfaceFormat f = format();

   // Ensure we have a valid version and profile. Default to context's if none specified
   QOpenGLVersionProfile vp = versionProfile;
   if (!vp.isValid()) {
      vp = QOpenGLVersionProfile(f);
   }

   // Check that context is compatible with requested version
   const QPair<int, int> v = qMakePair(f.majorVersion(), f.minorVersion());
   if (v < vp.version()) {
      return nullptr;
   }

   // If this context only offers core profile functions then we can't create
   // function objects for legacy or compatibility profile requests
   if (((vp.hasProfiles() && vp.profile() != QSurfaceFormat::CoreProfile) || vp.isLegacyVersion())
      && f.profile() == QSurfaceFormat::CoreProfile) {
      return nullptr;
   }

   // Create object if suitable one not cached
   QAbstractOpenGLFunctions *funcs = nullptr;
   if (!d->versionFunctions.contains(vp)) {
      funcs = QOpenGLVersionFunctionsFactory::create(vp);
      if (funcs) {
         funcs->setOwningContext(this);
         d->versionFunctions.insert(vp, funcs);
      }
   } else {
      funcs = d->versionFunctions.value(vp);
   }

   if (funcs && QOpenGLContext::currentContext() == this) {
      funcs->initializeOpenGLFunctions();
   }

   return funcs;
}

QSet<QByteArray> QOpenGLContext::extensions() const
{
   Q_D(const QOpenGLContext);

   if (d->extensionNames.isEmpty()) {
      QOpenGLExtensionMatcher matcher;
      d->extensionNames = matcher.extensions();
   }

   return d->extensionNames;
}

bool QOpenGLContext::hasExtension(const QByteArray &extension) const
{
   return extensions().contains(extension);
}

GLuint QOpenGLContext::defaultFramebufferObject() const
{
   if (! isValid()) {
      return 0;
   }

   Q_D(const QOpenGLContext);
   if (! d->surface || !d->surface->surfaceHandle()) {
      return 0;
   }

   if (d->defaultFboRedirect) {
      return d->defaultFboRedirect;
   }

   return d->platformGLContext->defaultFramebufferObject(d->surface->surfaceHandle());
}

bool QOpenGLContext::makeCurrent(QSurface *surface)
{
   Q_D(QOpenGLContext);
   if (!isValid()) {
      return false;
   }

   if (thread() != QThread::currentThread()) {
      qFatal("Cannot make QOpenGLContext current in a different thread");
   }

   if (!surface) {
      doneCurrent();
      return true;
   }

   if (!surface->surfaceHandle()) {
      return false;
   }
   if (!surface->supportsOpenGL()) {
      qWarning() << "QOpenGLContext::makeCurrent() Called with a Non OpenGL surface" << surface;
      return false;
   }

   QOpenGLContext *previous = QOpenGLContextPrivate::setCurrentContext(this);

   if (d->platformGLContext->makeCurrent(surface->surfaceHandle())) {
      d->surface = surface;
      d->shareGroup->d_func()->deletePendingResources(this);

      return true;
   }

   QOpenGLContextPrivate::setCurrentContext(previous);

   return false;
}

void QOpenGLContext::doneCurrent()
{
   Q_D(QOpenGLContext);
   if (!isValid()) {
      return;
   }

   if (QOpenGLContext::currentContext() == this) {
      d->shareGroup->d_func()->deletePendingResources(this);
   }

   d->platformGLContext->doneCurrent();
   QOpenGLContextPrivate::setCurrentContext(nullptr);

   d->surface = nullptr;
}

QSurface *QOpenGLContext::surface() const
{
   Q_D(const QOpenGLContext);
   return d->surface;
}

void QOpenGLContext::swapBuffers(QSurface *surface)
{
   Q_D(QOpenGLContext);
   if (!isValid()) {
      return;
   }

   if (!surface) {
      qWarning("QOpenGLContext::swapBuffers() Surface is invalid (nullptr)");
      return;
   }

   if (!surface->supportsOpenGL()) {
      qWarning("QOpenGLContext::swapBuffers() Called with a Non OpenGL surface");
      return;
   }

   if (surface->surfaceClass() == QSurface::Window
         && ! qt_window_private(static_cast<QWindow *>(surface))->receivedExpose) {
      qWarning("QOpenGLContext::swapBuffers() Called with a non exposed window");
   }

   QPlatformSurface *surfaceHandle = surface->surfaceHandle();
   if (!surfaceHandle) {
      return;
   }

   if (surface->format().swapBehavior() == QSurfaceFormat::SingleBuffer) {
      functions()->glFlush();
   }
   d->platformGLContext->swapBuffers(surfaceHandle);
}

QOpenGLContext::FP_Void QOpenGLContext::getProcAddress(const QByteArray &procName) const
{
   Q_D(const QOpenGLContext);

   if (! d->platformGLContext) {
      return nullptr;
   }

   return d->platformGLContext->getProcAddress(procName);
}

QSurfaceFormat QOpenGLContext::format() const
{
   Q_D(const QOpenGLContext);
   if (!d->platformGLContext) {
      return d->requestedFormat;
   }
   return d->platformGLContext->format();
}

QOpenGLContextGroup *QOpenGLContext::shareGroup() const
{
   Q_D(const QOpenGLContext);
   return d->shareGroup;
}

QOpenGLContext *QOpenGLContext::shareContext() const
{
   Q_D(const QOpenGLContext);
   return d->shareContext;
}

QScreen *QOpenGLContext::screen() const
{
   Q_D(const QOpenGLContext);
   return d->screen;
}

// internal
void *QOpenGLContext::qGLContextHandle() const
{
   Q_D(const QOpenGLContext);
   return d->qGLContextHandle;
}

// internal
void QOpenGLContext::setQGLContextHandle(void *handle, void (*qGLContextDeleteFunction)(void *))
{
   Q_D(QOpenGLContext);
   d->qGLContextHandle = handle;
   d->qGLContextDeleteFunction = qGLContextDeleteFunction;
}

// internal
void QOpenGLContext::deleteQGLContext()
{
   Q_D(QOpenGLContext);
   if (d->qGLContextDeleteFunction && d->qGLContextHandle) {
      d->qGLContextDeleteFunction(d->qGLContextHandle);
      d->qGLContextDeleteFunction = nullptr;
      d->qGLContextHandle = nullptr;
   }
}

void *QOpenGLContext::openGLModuleHandle()
{
#ifdef QT_OPENGL_DYNAMIC
   QPlatformNativeInterface *ni = QGuiApplication::platformNativeInterface();
   Q_ASSERT(ni);
   return ni->nativeResourceForIntegration(QByteArrayLiteral("glhandle"));
#else
   return nullptr;
#endif
}

QOpenGLContext::OpenGLModuleType QOpenGLContext::openGLModuleType()
{
#if defined(QT_OPENGL_DYNAMIC)
   Q_ASSERT(qGuiApp);
   return QGuiApplicationPrivate::instance()->platformIntegration()->openGLModuleType();
#elif defined(QT_OPENGL_ES_2)
   return LibGLES;
#else
   return LibGL;
#endif
}

bool QOpenGLContext::isOpenGLES() const
{
   return format().renderableType() == QSurfaceFormat::OpenGLES;
}

bool QOpenGLContext::supportsThreadedOpenGL()
{
   Q_ASSERT(qGuiApp);
   return QGuiApplicationPrivate::instance()->platformIntegration()->hasCapability(QPlatformIntegration::ThreadedOpenGL);
}

QOpenGLContext *QOpenGLContext::globalShareContext()
{
   Q_ASSERT(qGuiApp);
   return qt_gl_global_share_context();
}

// internal
QOpenGLVersionFunctionsBackend *QOpenGLContext::functionsBackend(const QOpenGLVersionStatus &v) const
{
   Q_D(const QOpenGLContext);
   return d->versionFunctionsBackend.value(v, nullptr);
}

// internal
void QOpenGLContext::insertFunctionsBackend(const QOpenGLVersionStatus &v,
   QOpenGLVersionFunctionsBackend *backend)
{
   Q_D(QOpenGLContext);
   d->versionFunctionsBackend.insert(v, backend);
}

// internal
void QOpenGLContext::removeFunctionsBackend(const QOpenGLVersionStatus &v)
{
   Q_D(QOpenGLContext);
   d->versionFunctionsBackend.remove(v);
}

// internal
void QOpenGLContext::insertExternalFunctions(QAbstractOpenGLFunctions *f)
{
   Q_D(QOpenGLContext);
   d->externalVersionFunctions.insert(f);
}

// internal
void QOpenGLContext::removeExternalFunctions(QAbstractOpenGLFunctions *f)
{
   Q_D(QOpenGLContext);
   d->externalVersionFunctions.remove(f);
}

// internal
QOpenGLTextureHelper *QOpenGLContext::textureFunctions() const
{
   Q_D(const QOpenGLContext);
   return d->textureFunctions;
}

// internal
void QOpenGLContext::setTextureFunctions(QOpenGLTextureHelper *textureFuncs)
{
   Q_D(QOpenGLContext);
   d->textureFunctions = textureFuncs;
}

QOpenGLContextGroup::QOpenGLContextGroup()
   : d_ptr(new QOpenGLContextGroupPrivate())
{
   d_ptr->q_ptr = this;
}

// internal
QOpenGLContextGroup::~QOpenGLContextGroup()
{
   Q_D(QOpenGLContextGroup);
   d->cleanup();
}

QList<QOpenGLContext *> QOpenGLContextGroup::shares() const
{
   Q_D(const QOpenGLContextGroup);
   return d->m_shares;
}

QOpenGLContextGroup *QOpenGLContextGroup::currentContextGroup()
{
   QOpenGLContext *current = QOpenGLContext::currentContext();
   return current ? current->shareGroup() : nullptr;
}

void QOpenGLContextGroupPrivate::addContext(QOpenGLContext *ctx)
{
   QRecursiveMutexLocker locker(&m_mutex);
   m_refs.ref();
   m_shares << ctx;
}

void QOpenGLContextGroupPrivate::removeContext(QOpenGLContext *ctx)
{
   Q_Q(QOpenGLContextGroup);

   bool deleteObject = false;

   {
      QRecursiveMutexLocker locker(&m_mutex);
      m_shares.removeOne(ctx);

      if (ctx == m_context && !m_shares.isEmpty()) {
         m_context = m_shares.first();
      }

      if (!m_refs.deref()) {
         cleanup();
         deleteObject = true;
      }
   }

   if (deleteObject) {
      if (q->thread() == QThread::currentThread()) {
         delete q;   // Delete directly to prevent leak, refer to QTBUG-29056
      } else {
         q->deleteLater();
      }
   }
}

void QOpenGLContextGroupPrivate::cleanup()
{
   Q_Q(QOpenGLContextGroup);
   {
      QHash<QOpenGLMultiGroupSharedResource *, QOpenGLSharedResource *>::const_iterator it, end;
      end = m_resources.constEnd();
      for (it = m_resources.constBegin(); it != end; ++it) {
         it.key()->cleanup(q, it.value());
      }
      m_resources.clear();
   }

   QList<QOpenGLSharedResource *>::iterator it = m_sharedResources.begin();
   QList<QOpenGLSharedResource *>::iterator end = m_sharedResources.end();

   while (it != end) {
      (*it)->invalidateResource();
      (*it)->m_group = nullptr;
      ++it;
   }

   m_sharedResources.clear();

   qDeleteAll(m_pendingDeletion.begin(), m_pendingDeletion.end());
   m_pendingDeletion.clear();
}

void QOpenGLContextGroupPrivate::deletePendingResources(QOpenGLContext *ctx)
{
   QRecursiveMutexLocker locker(&m_mutex);

   const QList<QOpenGLSharedResource *> pending = m_pendingDeletion;
   m_pendingDeletion.clear();

   QList<QOpenGLSharedResource *>::const_iterator it = pending.begin();
   QList<QOpenGLSharedResource *>::const_iterator end = pending.end();

   while (it != end) {
      (*it)->freeResource(ctx);
      delete *it;
      ++it;
   }
}

QOpenGLSharedResource::QOpenGLSharedResource(QOpenGLContextGroup *group)
   : m_group(group)
{
   QRecursiveMutexLocker locker(&m_group->d_func()->m_mutex);
   m_group->d_func()->m_sharedResources << this;
}

QOpenGLSharedResource::~QOpenGLSharedResource()
{
}

// schedule the resource for deletion at an appropriate time
void QOpenGLSharedResource::free()
{
   if (! m_group) {
      delete this;
      return;
   }

   QRecursiveMutexLocker locker(&m_group->d_func()->m_mutex);
   m_group->d_func()->m_sharedResources.removeOne(this);
   m_group->d_func()->m_pendingDeletion << this;

   // can we delete right away?
   QOpenGLContext *current = QOpenGLContext::currentContext();
   if (current && current->shareGroup() == m_group) {
      m_group->d_func()->deletePendingResources(current);
   }
}

// internal
void QOpenGLSharedResourceGuard::freeResource(QOpenGLContext *context)
{
   if (m_id) {
      QOpenGLFunctions functions(context);
      m_func(&functions, m_id);
      m_id = 0;
   }
}

// internal
QOpenGLMultiGroupSharedResource::QOpenGLMultiGroupSharedResource()
   : active(0)
{
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
   qDebug("QOpenGLMultiGroupSharedResource() Creating context group resource object %p", this);
#endif
}

QOpenGLMultiGroupSharedResource::~QOpenGLMultiGroupSharedResource()
{
   for (int i = 0; i < m_groups.size(); ++i) {
      if (!m_groups.at(i)->shares().isEmpty()) {
         QOpenGLContext *context = m_groups.at(i)->shares().first();
         QOpenGLSharedResource *resource = value(context);

         if (resource) {
            resource->free();
         }
      }
      m_groups.at(i)->d_func()->m_resources.remove(this);
      active.deref();
   }
}

void QOpenGLMultiGroupSharedResource::insert(QOpenGLContext *context, QOpenGLSharedResource *value)
{
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
   qDebug("QOpenGLMultiGroupSharedResource::insert() Inserting context group resource "
      "%p for context %p, managed by %p.", value, context, this);
#endif

   QOpenGLContextGroup *group = context->shareGroup();
   Q_ASSERT(!group->d_func()->m_resources.contains(this));
   group->d_func()->m_resources.insert(this, value);
   m_groups.append(group);
   active.ref();
}

QOpenGLSharedResource *QOpenGLMultiGroupSharedResource::value(QOpenGLContext *context)
{
   QOpenGLContextGroup *group = context->shareGroup();
   return group->d_func()->m_resources.value(this, nullptr);
}

QList<QOpenGLSharedResource *> QOpenGLMultiGroupSharedResource::resources() const
{
   QList<QOpenGLSharedResource *> result;

   for (auto item : m_groups) {
      QOpenGLSharedResource *resource = item->d_func()->m_resources.value(const_cast<QOpenGLMultiGroupSharedResource *>(this), nullptr);

      if (resource) {
         result << resource;
      }
   }

   return result;
}

void QOpenGLMultiGroupSharedResource::cleanup(QOpenGLContextGroup *group, QOpenGLSharedResource *value)
{
#if defined(CS_SHOW_DEBUG_GUI_OPENGL)
   qDebug("QOpenGLMultiGroupSharedResource::cleanup() Context group resource %p, for group %p in thread %p",
         this, group, QThread::currentThread());
#endif

   value->invalidateResource();
   value->free();
   active.deref();

   Q_ASSERT(m_groups.contains(group));
   m_groups.removeOne(group);
}

void QOpenGLContext::_q_screenDestroyed(QObject *object)
{
   Q_D(QOpenGLContext);
   d->_q_screenDestroyed(object);
}
