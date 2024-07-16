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

#include <qgl.h>

#include <qapplication.h>
#include <qcolormap.h>
#include <qdebug.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qimage.h>
#include <qlibrary.h>
#include <qglfunctions.h>
#include <qmutex.h>
#include <qpixmap.h>
#include <qplatform_pixmap.h>
#include <qplatform_openglcontext.h>
#include <qplatform_window.h>
#include <qplatform_openglcontext.h>
#include <qsurfaceformat.h>

#include <qapplication_p.h>
#include <qimage_p.h>
#include <qimagepixmapcleanuphooks_p.h>
#include <qgl_p.h>
#include <qglpixelbuffer_p.h>
#include <qpaintengineex_opengl2_p.h>
#include <qglpixelbuffer.h>
#include <qglframebufferobject.h>

#ifndef QT_OPENGL_ES_2
#include <qopenglfunctions_1_1.h>
#endif

#include <stdlib.h>

class QGLDefaultExtensions
{
 public:
   QGLDefaultExtensions()
      : extensions(Qt::EmptyFlag)
   {
      QGLTemporaryContext tempContext;

      Q_ASSERT(QOpenGLContext::currentContext());
      QOpenGLExtensions *ext = qgl_extensions();

      Q_ASSERT(ext);

      extensions = ext->openGLExtensions();
      features   = ext->openGLFeatures();
   }

   QOpenGLFunctions::OpenGLFeatures features;
   QOpenGLExtensions::OpenGLExtensions extensions;
};

static QGLDefaultExtensions *qtDefaultExtensions()
{
   static QGLDefaultExtensions retval;
   return &retval;
}

bool qgl_hasFeature(QOpenGLFunctions::OpenGLFeature feature)
{
   if (QOpenGLContext::currentContext()) {
      return QOpenGLContext::currentContext()->functions()->hasOpenGLFeature(feature);
   }

   return qtDefaultExtensions()->features & feature;
}

bool qgl_hasExtension(QOpenGLExtensions::OpenGLExtension extension)
{
   if (QOpenGLContext::currentContext()) {
      return qgl_extensions()->hasOpenGLExtension(extension);
   }

   return qtDefaultExtensions()->extensions & extension;
}

QOpenGLExtensions::OpenGLExtensions extensions;

QOpenGLExtensions *qgl_extensions()
{
   if (QOpenGLContext *context = QOpenGLContext::currentContext()) {
      return static_cast<QOpenGLExtensions *>(context->functions());
   }

   Q_ASSERT(false);
   return nullptr;
}

QOpenGLFunctions *qgl_functions()
{
   return qgl_extensions(); // QOpenGLExtensions is just a subclass of QOpenGLFunctions
}

#ifndef QT_OPENGL_ES_2
QOpenGLFunctions_1_1 *qgl1_functions()
{
   QOpenGLFunctions_1_1 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_1_1>();
   f->initializeOpenGLFunctions();
   return f;
}
#endif

struct QGLThreadContext {

   ~QGLThreadContext() {

      if (context) {
         context->doneCurrent();
      }
   }

   QGLContext *context;
};

static QGLFormat *qgl_default_format()
{
   static QGLFormat retval;
   return &retval;
}

class QGLDefaultOverlayFormat: public QGLFormat
{
 public:
   inline QGLDefaultOverlayFormat() {
      setOption(QGL::FormatOption(0xffff << 16)); // turn off all options
      setOption(QGL::DirectRendering);
      setPlane(1);
   }
};

static QGLDefaultOverlayFormat *defaultOverlayFormatInstance()
{
   static QGLDefaultOverlayFormat retval;
   return &retval;
}

static QGLSignalProxy *theSignalProxy()
{
   static QGLSignalProxy retval;
   return &retval;
}

QGLSignalProxy *QGLSignalProxy::instance()
{
   QGLSignalProxy *proxy = theSignalProxy();

   if (proxy && qApp && proxy->thread() != qApp->thread()) {
      if (proxy->thread() == QThread::currentThread()) {
         proxy->moveToThread(qApp->thread());
      }
   }

   return proxy;
}

#ifndef QT_OPENGL_ES

static inline void transform_point(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#define M(row,col)  m[col*4+row]
   out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
   out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
   out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
   out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

static inline GLint qgluProject(GLdouble objx, GLdouble objy, GLdouble objz,
            const GLdouble model[16], const GLdouble proj[16], const GLint viewport[4],
            GLdouble *winx, GLdouble *winy, GLdouble *winz)
{
   GLdouble in[4], out[4];

   in[0] = objx;
   in[1] = objy;
   in[2] = objz;
   in[3] = 1.0;

   transform_point(out, model, in);
   transform_point(in, proj, out);

   if (in[3] == 0.0) {
      return GL_FALSE;
   }

   in[0] /= in[3];
   in[1] /= in[3];
   in[2] /= in[3];

   *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
   *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

   *winz = (1 + in[2]) / 2;

   return GL_TRUE;
}

#endif // !QT_OPENGL_ES

QGLFormat::QGLFormat()
{
   d = new QGLFormatPrivate;
}

QGLFormat::QGLFormat(QGL::FormatOptions options, int plane)
{
   d = new QGLFormatPrivate;
   QGL::FormatOptions newOpts = options;
   d->opts = defaultFormat().d->opts;
   d->opts |= (newOpts & 0xffff);
   d->opts &= ~(newOpts >> 16);
   d->pln = plane;
}

// internal
void QGLFormat::detach()
{
   if (d->ref.load() != 1) {
      QGLFormatPrivate *newd = new QGLFormatPrivate(d);
      if (!d->ref.deref()) {
         delete d;
      }
      d = newd;
   }
}

QGLFormat::QGLFormat(const QGLFormat &other)
{
   d = other.d;
   d->ref.ref();
}

QGLFormat &QGLFormat::operator=(const QGLFormat &other)
{
   if (d != other.d) {
      other.d->ref.ref();
      if (!d->ref.deref()) {
         delete d;
      }
      d = other.d;
   }
   return *this;
}

QGLFormat::~QGLFormat()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

QGLFormat QGLFormat::fromSurfaceFormat(const QSurfaceFormat &format)
{
   QGLFormat retFormat;
   if (format.alphaBufferSize() >= 0) {
      retFormat.setAlphaBufferSize(format.alphaBufferSize());
   }
   if (format.blueBufferSize() >= 0) {
      retFormat.setBlueBufferSize(format.blueBufferSize());
   }
   if (format.greenBufferSize() >= 0) {
      retFormat.setGreenBufferSize(format.greenBufferSize());
   }
   if (format.redBufferSize() >= 0) {
      retFormat.setRedBufferSize(format.redBufferSize());
   }
   if (format.depthBufferSize() >= 0) {
      retFormat.setDepthBufferSize(format.depthBufferSize());
   }
   if (format.samples() > 1) {
      retFormat.setSampleBuffers(true);
      retFormat.setSamples(format.samples());
   }
   if (format.stencilBufferSize() > 0) {
      retFormat.setStencil(true);
      retFormat.setStencilBufferSize(format.stencilBufferSize());
   }
   retFormat.setSwapInterval(format.swapInterval());
   retFormat.setDoubleBuffer(format.swapBehavior() != QSurfaceFormat::SingleBuffer);
   retFormat.setStereo(format.stereo());
   retFormat.setVersion(format.majorVersion(), format.minorVersion());
   retFormat.setProfile(static_cast<QGLFormat::OpenGLContextProfile>(format.profile()));
   return retFormat;
}

QSurfaceFormat QGLFormat::toSurfaceFormat(const QGLFormat &format)
{
   QSurfaceFormat retFormat;

   if (format.alpha()) {
      retFormat.setAlphaBufferSize(format.alphaBufferSize() == -1 ? 1 : format.alphaBufferSize());
   }

   if (format.blueBufferSize() >= 0) {
      retFormat.setBlueBufferSize(format.blueBufferSize());
   }

   if (format.greenBufferSize() >= 0) {
      retFormat.setGreenBufferSize(format.greenBufferSize());
   }

   if (format.redBufferSize() >= 0) {
      retFormat.setRedBufferSize(format.redBufferSize());
   }

   if (format.depth()) {
      retFormat.setDepthBufferSize(format.depthBufferSize() == -1 ? 1 : format.depthBufferSize());
   }

   retFormat.setSwapBehavior(format.doubleBuffer() ? QSurfaceFormat::DoubleBuffer : QSurfaceFormat::SingleBuffer);
   if (format.sampleBuffers()) {
      retFormat.setSamples(format.samples() == -1 ? 4 : format.samples());
   }

   if (format.stencil()) {
      retFormat.setStencilBufferSize(format.stencilBufferSize() == -1 ? 1 : format.stencilBufferSize());
   }

   retFormat.setSwapInterval(format.swapInterval());
   retFormat.setStereo(format.stereo());
   retFormat.setMajorVersion(format.majorVersion());
   retFormat.setMinorVersion(format.minorVersion());
   retFormat.setProfile(static_cast<QSurfaceFormat::OpenGLContextProfile>(format.profile()));

   // QGLFormat has no way to set DeprecatedFunctions, that is, to tell that forward
   // compatibility should not be requested. Some drivers fail to ignore the fwdcompat
   // bit with compatibility profiles so make sure it is not set.

   if (format.profile() == QGLFormat::CompatibilityProfile) {
      retFormat.setOption(QSurfaceFormat::DeprecatedFunctions);
   }

   return retFormat;
}

void QGLContextPrivate::setupSharing()
{
   Q_Q(QGLContext);
   QOpenGLContext *sharedContext = guiGlContext->shareContext();

   if (sharedContext) {
      QGLContext *actualSharedContext = QGLContext::fromOpenGLContext(sharedContext);
      sharing = true;
      QGLContextGroup::addShare(q, actualSharedContext);
   }
}

void QGLContextPrivate::refreshCurrentFbo()
{
   QOpenGLContextPrivate *guiGlContextPrivate = guiGlContext ? QOpenGLContextPrivate::get(guiGlContext) : nullptr;

   // if QOpenGLFramebufferObjects have been used in the mean-time, we've lost our cached value
   if (guiGlContextPrivate && guiGlContextPrivate->qgl_current_fbo_invalid) {
      GLint current;
      QOpenGLFunctions *funcs = qgl_functions();
      funcs->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current);

      current_fbo = current;

      guiGlContextPrivate->qgl_current_fbo_invalid = false;
   }
}

void QGLContextPrivate::setCurrentFbo(GLuint fbo)
{
   current_fbo = fbo;

   QOpenGLContextPrivate *guiGlContextPrivate = guiGlContext ? QOpenGLContextPrivate::get(guiGlContext) : nullptr;

   if (guiGlContextPrivate) {
      guiGlContextPrivate->qgl_current_fbo_invalid = false;
   }
}

void QGLFormat::setDoubleBuffer(bool enable)
{
   setOption(enable ? QGL::DoubleBuffer : QGL::SingleBuffer);
}

void QGLFormat::setDepth(bool enable)
{
   setOption(enable ? QGL::DepthBuffer : QGL::NoDepthBuffer);
}

void QGLFormat::setRgba(bool enable)
{
   setOption(enable ? QGL::Rgba : QGL::ColorIndex);
}

void QGLFormat::setAlpha(bool enable)
{
   setOption(enable ? QGL::AlphaChannel : QGL::NoAlphaChannel);
}

void QGLFormat::setAccum(bool enable)
{
   setOption(enable ? QGL::AccumBuffer : QGL::NoAccumBuffer);
}

void QGLFormat::setStencil(bool enable)
{
   setOption(enable ? QGL::StencilBuffer : QGL::NoStencilBuffer);
}

void QGLFormat::setStereo(bool enable)
{
   setOption(enable ? QGL::StereoBuffers : QGL::NoStereoBuffers);
}

void QGLFormat::setDirectRendering(bool enable)
{
   setOption(enable ? QGL::DirectRendering : QGL::IndirectRendering);
}

void QGLFormat::setSampleBuffers(bool enable)
{
   setOption(enable ? QGL::SampleBuffers : QGL::NoSampleBuffers);
}

int QGLFormat::samples() const
{
   return d->numSamples;
}

void QGLFormat::setSamples(int numSamples)
{
   detach();
   if (numSamples < 0) {
      qWarning("QGLFormat::setSamples: Cannot have negative number of samples per pixel %d", numSamples);
      return;
   }
   d->numSamples = numSamples;
   setSampleBuffers(numSamples > 0);
}

void QGLFormat::setSwapInterval(int interval)
{
   detach();
   d->swapInterval = interval;
}

int QGLFormat::swapInterval() const
{
   return d->swapInterval;
}

void QGLFormat::setOverlay(bool enable)
{
   setOption(enable ? QGL::HasOverlay : QGL::NoOverlay);
}

int QGLFormat::plane() const
{
   return d->pln;
}

void QGLFormat::setPlane(int plane)
{
   detach();
   d->pln = plane;
}

void QGLFormat::setOption(QGL::FormatOptions opt)
{
   detach();
   if (opt & 0xffff) {
      d->opts |= opt;
   } else {
      d->opts &= ~(opt >> 16);
   }
}

bool QGLFormat::testOption(QGL::FormatOptions opt) const
{
   if (opt & 0xffff) {
      return (d->opts & opt) != 0;
   } else {
      return (d->opts & (opt >> 16)) == 0;
   }
}

void QGLFormat::setDepthBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setDepthBufferSize: Cannot set negative depth buffer size %d", size);
      return;
   }
   d->depthSize = size;
   setDepth(size > 0);
}

int QGLFormat::depthBufferSize() const
{
   return d->depthSize;
}

void QGLFormat::setRedBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setRedBufferSize: Cannot set negative red buffer size %d", size);
      return;
   }
   d->redSize = size;
}

int QGLFormat::redBufferSize() const
{
   return d->redSize;
}

void QGLFormat::setGreenBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setGreenBufferSize: Cannot set negative green buffer size %d", size);
      return;
   }
   d->greenSize = size;
}

int QGLFormat::greenBufferSize() const
{
   return d->greenSize;
}

void QGLFormat::setBlueBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setBlueBufferSize: Cannot set negative blue buffer size %d", size);
      return;
   }
   d->blueSize = size;
}

int QGLFormat::blueBufferSize() const
{
   return d->blueSize;
}

void QGLFormat::setAlphaBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setAlphaBufferSize: Cannot set negative alpha buffer size %d", size);
      return;
   }
   d->alphaSize = size;
   setAlpha(size > 0);
}

int QGLFormat::alphaBufferSize() const
{
   return d->alphaSize;
}

void QGLFormat::setAccumBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setAccumBufferSize: Cannot set negative accumulate buffer size %d", size);
      return;
   }
   d->accumSize = size;
   setAccum(size > 0);
}

int QGLFormat::accumBufferSize() const
{
   return d->accumSize;
}

void QGLFormat::setStencilBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setStencilBufferSize: Cannot set negative stencil buffer size %d", size);
      return;
   }
   d->stencilSize = size;
   setStencil(size > 0);
}

int QGLFormat::stencilBufferSize() const
{
   return d->stencilSize;
}

void QGLFormat::setVersion(int major, int minor)
{
   if (major < 1 || minor < 0) {
      qWarning("QGLFormat::setVersion: Cannot set zero or negative version number %d.%d", major, minor);
      return;
   }
   detach();
   d->majorVersion = major;
   d->minorVersion = minor;
}

int QGLFormat::majorVersion() const
{
   return d->majorVersion;
}

int QGLFormat::minorVersion() const
{
   return d->minorVersion;
}

void QGLFormat::setProfile(OpenGLContextProfile profile)
{
   detach();
   d->profile = profile;
}

QGLFormat::OpenGLContextProfile QGLFormat::profile() const
{
   return d->profile;
}

bool QGLFormat::hasOpenGL()
{
   return QApplicationPrivate::platformIntegration()
      ->hasCapability(QPlatformIntegration::OpenGL);
}
bool QGLFormat::hasOpenGLOverlays()
{
   return false;
}

QGLFormat::OpenGLVersionFlags qOpenGLVersionFlagsFromString(const QString &versionString)
{
   QGLFormat::OpenGLVersionFlags versionFlags = QGLFormat::OpenGL_Version_None;

   if (versionString.startsWith(QString("OpenGL ES"))) {
      QStringList parts = versionString.split(QLatin1Char(' '));

      if (parts.size() >= 3) {
         if (parts[2].startsWith(QLatin1String("1."))) {
            if (parts[1].endsWith(QLatin1String("-CM"))) {
               versionFlags |= QGLFormat::OpenGL_ES_Common_Version_1_0 |
                  QGLFormat::OpenGL_ES_CommonLite_Version_1_0;
               if (parts[2].startsWith(QLatin1String("1.1")))
                  versionFlags |= QGLFormat::OpenGL_ES_Common_Version_1_1 |
                     QGLFormat::OpenGL_ES_CommonLite_Version_1_1;
            } else {
               // Not -CM, must be CL, CommonLite
               versionFlags |= QGLFormat::OpenGL_ES_CommonLite_Version_1_0;
               if (parts[2].startsWith(QLatin1String("1.1"))) {
                  versionFlags |= QGLFormat::OpenGL_ES_CommonLite_Version_1_1;
               }
            }
         } else {
            // OpenGL ES version 2.0 or higher
            versionFlags |= QGLFormat::OpenGL_ES_Version_2_0;
         }
      } else {
         // if < 3 parts to the name, it is an unrecognised OpenGL ES
         qWarning("Unrecognised OpenGL ES version");
      }

   } else {
      // not ES, regular OpenGL, the version numbers are first in the string
      if (versionString.startsWith(QLatin1String("1."))) {
         switch (versionString[2].toLatin1()) {
            case '5':
               versionFlags |= QGLFormat::OpenGL_Version_1_5;
               [[fallthrough]];

            case '4':
               versionFlags |= QGLFormat::OpenGL_Version_1_4;
               [[fallthrough]];

            case '3':
               versionFlags |= QGLFormat::OpenGL_Version_1_3;
               [[fallthrough]];

            case '2':
               versionFlags |= QGLFormat::OpenGL_Version_1_2;
               [[fallthrough]];

            case '1':
               versionFlags |= QGLFormat::OpenGL_Version_1_1;
               [[fallthrough]];

            default:
               break;
         }

      } else if (versionString.startsWith(QLatin1String("2."))) {
         versionFlags |= QGLFormat::OpenGL_Version_1_1 |
            QGLFormat::OpenGL_Version_1_2 |
            QGLFormat::OpenGL_Version_1_3 |
            QGLFormat::OpenGL_Version_1_4 |
            QGLFormat::OpenGL_Version_1_5 |
            QGLFormat::OpenGL_Version_2_0;

         if (versionString[2].toLatin1() == '1') {
            versionFlags |= QGLFormat::OpenGL_Version_2_1;
         }

      } else if (versionString.startsWith(QLatin1String("3."))) {
         versionFlags |= QGLFormat::OpenGL_Version_1_1 |
            QGLFormat::OpenGL_Version_1_2 |
            QGLFormat::OpenGL_Version_1_3 |
            QGLFormat::OpenGL_Version_1_4 |
            QGLFormat::OpenGL_Version_1_5 |
            QGLFormat::OpenGL_Version_2_0 |
            QGLFormat::OpenGL_Version_2_1 |
            QGLFormat::OpenGL_Version_3_0;

         switch (versionString[2].toLatin1()) {
            case '3':
               versionFlags |= QGLFormat::OpenGL_Version_3_3;
               [[fallthrough]];

            case '2':
               versionFlags |= QGLFormat::OpenGL_Version_3_2;
               [[fallthrough]];

            case '1':
               versionFlags |= QGLFormat::OpenGL_Version_3_1;
               [[fallthrough]];

            case '0':
               break;

            default:
               versionFlags |= QGLFormat::OpenGL_Version_3_1 |
                  QGLFormat::OpenGL_Version_3_2 |
                  QGLFormat::OpenGL_Version_3_3;
               break;
         }

      } else if (versionString.startsWith(QLatin1String("4."))) {
         versionFlags |= QGLFormat::OpenGL_Version_1_1 |
            QGLFormat::OpenGL_Version_1_2 |
            QGLFormat::OpenGL_Version_1_3 |
            QGLFormat::OpenGL_Version_1_4 |
            QGLFormat::OpenGL_Version_1_5 |
            QGLFormat::OpenGL_Version_2_0 |
            QGLFormat::OpenGL_Version_2_1 |
            QGLFormat::OpenGL_Version_3_0 |
            QGLFormat::OpenGL_Version_3_1 |
            QGLFormat::OpenGL_Version_3_2 |
            QGLFormat::OpenGL_Version_3_3 |
            QGLFormat::OpenGL_Version_4_0;

         switch (versionString[2].toLatin1()) {
            case '3':
               versionFlags |= QGLFormat::OpenGL_Version_4_3;
               [[fallthrough]];

            case '2':
               versionFlags |= QGLFormat::OpenGL_Version_4_2;
               [[fallthrough]];

            case '1':
               versionFlags |= QGLFormat::OpenGL_Version_4_1;
               [[fallthrough]];

            case '0':
               break;

            default:
               versionFlags |= QGLFormat::OpenGL_Version_4_1 |
                  QGLFormat::OpenGL_Version_4_2 |
                  QGLFormat::OpenGL_Version_4_3;
               break;
         }

      } else {
         versionFlags |= QGLFormat::OpenGL_Version_1_1 |
            QGLFormat::OpenGL_Version_1_2 |
            QGLFormat::OpenGL_Version_1_3 |
            QGLFormat::OpenGL_Version_1_4 |
            QGLFormat::OpenGL_Version_1_5 |
            QGLFormat::OpenGL_Version_2_0 |
            QGLFormat::OpenGL_Version_2_1 |
            QGLFormat::OpenGL_Version_3_0 |
            QGLFormat::OpenGL_Version_3_1 |
            QGLFormat::OpenGL_Version_3_2 |
            QGLFormat::OpenGL_Version_3_3 |
            QGLFormat::OpenGL_Version_4_0 |
            QGLFormat::OpenGL_Version_4_1 |
            QGLFormat::OpenGL_Version_4_2 |
            QGLFormat::OpenGL_Version_4_3;
      }
   }

   return versionFlags;
}

QGLFormat::OpenGLVersionFlags QGLFormat::openGLVersionFlags()
{
   static bool cachedDefault = false;
   static OpenGLVersionFlags defaultVersionFlags = OpenGL_Version_None;

   QGLContext *currentCtx = const_cast<QGLContext *>(QGLContext::currentContext());
   QGLTemporaryContext *tmpContext = nullptr;

   if (currentCtx && currentCtx->d_func()->version_flags_cached) {
      return currentCtx->d_func()->version_flags;
   }

   if (!currentCtx) {
      if (cachedDefault) {
         return defaultVersionFlags;
      } else {
         if (!hasOpenGL()) {
            return defaultVersionFlags;
         }
         tmpContext = new QGLTemporaryContext;
         cachedDefault = true;
      }
   }

   const QString &versionStr = cs_glGetString(GL_VERSION);
   OpenGLVersionFlags versionFlags = qOpenGLVersionFlagsFromString(versionStr);

   if (currentCtx) {
      currentCtx->d_func()->version_flags_cached = true;
      currentCtx->d_func()->version_flags = versionFlags;
   }

   if (tmpContext) {
      defaultVersionFlags = versionFlags;
      delete tmpContext;
   }

   return versionFlags;
}

QGLFormat QGLFormat::defaultFormat()
{
   return *qgl_default_format();
}

void QGLFormat::setDefaultFormat(const QGLFormat &f)
{
   *qgl_default_format() = f;
}

QGLFormat QGLFormat::defaultOverlayFormat()
{
   return *defaultOverlayFormatInstance();
}

void QGLFormat::setDefaultOverlayFormat(const QGLFormat &f)
{
   QGLFormat *defaultFormat = defaultOverlayFormatInstance();
   *defaultFormat = f;
   // Make sure the user doesn't request that the overlays themselves
   // have overlays, since it is unlikely that the system supports
   // infinitely many planes...
   defaultFormat->setOverlay(false);
}

bool operator==(const QGLFormat &a, const QGLFormat &b)
{
   return (a.d == b.d) || ((int) a.d->opts == (int) b.d->opts
         && a.d->pln == b.d->pln
         && a.d->alphaSize == b.d->alphaSize
         && a.d->accumSize == b.d->accumSize
         && a.d->stencilSize == b.d->stencilSize
         && a.d->depthSize == b.d->depthSize
         && a.d->redSize == b.d->redSize
         && a.d->greenSize == b.d->greenSize
         && a.d->blueSize == b.d->blueSize
         && a.d->numSamples == b.d->numSamples
         && a.d->swapInterval == b.d->swapInterval
         && a.d->majorVersion == b.d->majorVersion
         && a.d->minorVersion == b.d->minorVersion
         && a.d->profile == b.d->profile);
}

QDebug operator<<(QDebug dbg, const QGLFormat &f)
{
   const QGLFormatPrivate *const d = f.d;

   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QGLFormat("
      << "options " << d->opts
      << ", plane " << d->pln
      << ", depthBufferSize " << d->depthSize
      << ", accumBufferSize " << d->accumSize
      << ", stencilBufferSize " << d->stencilSize
      << ", redBufferSize " << d->redSize
      << ", greenBufferSize " << d->greenSize
      << ", blueBufferSize " << d->blueSize
      << ", alphaBufferSize " << d->alphaSize
      << ", samples " << d->numSamples
      << ", swapInterval " << d->swapInterval
      << ", majorVersion " << d->majorVersion
      << ", minorVersion " << d->minorVersion
      << ", profile " << d->profile
      << ')';

   return dbg;
}

bool operator!=(const QGLFormat &a, const QGLFormat &b)
{
   return !(a == b);
}

struct QGLContextGroupList {
   QGLContextGroupList()
   {
   }

   void append(QGLContextGroup *group) {
      QRecursiveMutexLocker locker(&m_mutex);
      m_list.append(group);
   }

   void remove(QGLContextGroup *group) {
      QRecursiveMutexLocker locker(&m_mutex);
      m_list.removeOne(group);
   }

   QList<QGLContextGroup *> m_list;
   QRecursiveMutex m_mutex;
};

static QGLContextGroupList *qt_context_groups()
{
   static QGLContextGroupList retval;
   return &retval;
}

QGLContextGroup::QGLContextGroup(const QGLContext *context)
   : m_context(context), m_refs(1)
{
   qt_context_groups()->append(this);
}

QGLContextGroup::~QGLContextGroup()
{
   qt_context_groups()->remove(this);
}

const QGLContext *qt_gl_transfer_context(const QGLContext *ctx)
{
   if (! ctx) {
      return nullptr;
   }

   QList<const QGLContext *> shares(QGLContextPrivate::contextGroup(ctx)->shares());

   if (shares.size() >= 2) {
      return (ctx == shares.at(0)) ? shares.at(1) : shares.at(0);
   } else {
      return nullptr;
   }
}

QGLContextPrivate::QGLContextPrivate(QGLContext *context)
   : internal_context(false), q_ptr(context), texture_destroyer(nullptr), functions(nullptr)
{
   group = new QGLContextGroup(context);
   texture_destroyer = new QGLTextureDestroyer;
}

QGLContextPrivate::~QGLContextPrivate()
{
   delete functions;
   if (!group->m_refs.deref()) {
      Q_ASSERT(group->context() == q_ptr);
      delete group;
   }

   delete texture_destroyer;
}

void QGLContextPrivate::init(QPaintDevice *dev, const QGLFormat &format)
{
   Q_Q(QGLContext);
   glFormat = reqFormat = format;
   valid = false;
   q->setDevice(dev);

   guiGlContext = nullptr;
   ownContext = false;

   fbo      = 0;
   crWin    = false;
   initDone = false;
   sharing  = false;
   max_texture_size = -1;
   version_flags_cached = false;
   version_flags = QGLFormat::OpenGL_Version_None;

   current_fbo   = 0;
   default_fbo   = 0;
   active_engine = nullptr;

   workaround_needsFullClearOnEveryFrame = false;
   workaround_brokenFBOReadBack = false;
   workaround_brokenTexSubImage = false;
   workaroundsCached = false;

   workaround_brokenTextureFromPixmap = false;
   workaround_brokenTextureFromPixmap_init = false;

   workaround_brokenAlphaTexSubImage = false;
   workaround_brokenAlphaTexSubImage_init = false;

   for (int i = 0; i < QT_GL_VERTEX_ARRAY_TRACKED_COUNT; ++i) {
      vertexAttributeArraysEnabledState[i] = false;
   }
}

QGLContext *QGLContext::currentCtx = nullptr;
class QGLTemporaryContextPrivate
{
 public:
   QWindow *window;
   QOpenGLContext *context;

   QGLContext *oldContext;
};

QGLTemporaryContext::QGLTemporaryContext(bool, QWidget *)
   : d(new QGLTemporaryContextPrivate)
{
   d->oldContext = const_cast<QGLContext *>(QGLContext::currentContext());

   d->window = new QWindow;
   d->window->setSurfaceType(QWindow::OpenGLSurface);
   d->window->setGeometry(QRect(0, 0, 3, 3));
   d->window->create();

   d->context = new QOpenGLContext;
#if ! defined(QT_OPENGL_ES)
   if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
      // On desktop, request latest released version
      QSurfaceFormat format;

#if defined(Q_OS_DARWIN)
      // OS X is limited to OpenGL 3.2 Core Profile at present
      // so set that here. If we use compatibility profile it
      // only reports 2.x contexts.
      format.setMajorVersion(3);
      format.setMinorVersion(2);
      format.setProfile(QSurfaceFormat::CoreProfile);
#else
      format.setMajorVersion(4);
      format.setMinorVersion(3);
#endif

      d->context->setFormat(format);
   }

#endif


   d->context->create();
   d->context->makeCurrent(d->window);
}

QGLTemporaryContext::~QGLTemporaryContext()
{
   if (d->oldContext) {
      d->oldContext->makeCurrent();
   }

   delete d->context;
   delete d->window;
}

static void convertFromGLImage(QImage &img, int w, int h, bool alpha_format, bool include_alpha)
{
   Q_ASSERT(!img.isNull());

   if constexpr (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
      // OpenGL gives RGBA; Qt wants ARGB
      uint *p = (uint *)img.bits();
      uint *end = p + w * h;
      if (alpha_format && include_alpha) {
         while (p < end) {
            uint a = *p << 24;
            *p = (*p >> 8) | a;
            p++;
         }
      } else {
         // This is an old legacy fix for PowerPC based Macs, which
         // we shouldn't remove
         while (p < end) {
            *p = 0xff000000 | (*p >> 8);
            ++p;
         }
      }
   } else {
      // OpenGL gives ABGR (i.e. RGBA backwards); Qt wants ARGB
      for (int y = 0; y < h; y++) {
         uint *q = (uint *)img.scanLine(y);
         for (int x = 0; x < w; ++x) {
            const uint pixel = *q;
            if (alpha_format && include_alpha) {
               *q = ((pixel << 16) & 0xff0000) | ((pixel >> 16) & 0xff)
                  | (pixel & 0xff00ff00);
            } else {
               *q = 0xff000000 | ((pixel << 16) & 0xff0000)
                  | ((pixel >> 16) & 0xff) | (pixel & 0x00ff00);
            }

            q++;
         }
      }

   }
   img = img.mirrored();
}

QImage cs_glRead_frameBuffer(const QSize &size, bool alpha_format, bool include_alpha)
{
   QImage img(size, (alpha_format && include_alpha) ? QImage::Format_ARGB32_Premultiplied
      : QImage::Format_RGB32);

   if (img.isNull()) {
      return QImage();
   }

   int w = size.width();
   int h = size.height();
   qgl_functions()->glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
   convertFromGLImage(img, w, h, alpha_format, include_alpha);
   return img;
}

QImage qt_gl_read_texture(const QSize &size, bool alpha_format, bool include_alpha)
{
   QImage img(size, alpha_format ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32);

   if (img.isNull()) {
      return QImage();
   }

   int w = size.width();
   int h = size.height();

#ifndef QT_OPENGL_ES
   if (! QOpenGLContext::currentContext()->isOpenGLES()) {
      qgl1_functions()->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
   }
#endif

   convertFromGLImage(img, w, h, alpha_format, include_alpha);

   return img;
}

static QGLTextureCache *qt_gl_texture_cache()
{
   static QGLTextureCache retval;
   return &retval;
}

QGLTextureCache::QGLTextureCache()
   : m_cache(64 * 1024) // cache ~64 MB worth of textures - this is not accurate though
{
   QImagePixmapCleanupHooks::instance()->addPlatformPixmapModificationHook(cleanupTexturesForPixampData);
   QImagePixmapCleanupHooks::instance()->addPlatformPixmapDestructionHook(cleanupBeforePixmapDestruction);
   QImagePixmapCleanupHooks::instance()->addImageHook(cleanupTexturesForCacheKey);
}

QGLTextureCache::~QGLTextureCache()
{
   QImagePixmapCleanupHooks::instance()->removePlatformPixmapModificationHook(cleanupTexturesForPixampData);
   QImagePixmapCleanupHooks::instance()->removePlatformPixmapDestructionHook(cleanupBeforePixmapDestruction);
   QImagePixmapCleanupHooks::instance()->removeImageHook(cleanupTexturesForCacheKey);
}

void QGLTextureCache::insert(QGLContext *ctx, qint64 key, QGLTexture *texture, int cost)
{
   QWriteLocker locker(&m_lock);
   const QGLTextureCacheKey cacheKey = {key, QGLContextPrivate::contextGroup(ctx)};
   m_cache.insert(cacheKey, texture, cost);
}

void QGLTextureCache::remove(qint64 key)
{
   QWriteLocker locker(&m_lock);

   QRecursiveMutexLocker groupLocker(&qt_context_groups()->m_mutex);
   QList<QGLContextGroup *>::const_iterator it = qt_context_groups()->m_list.constBegin();

   while (it != qt_context_groups()->m_list.constEnd()) {
      const QGLTextureCacheKey cacheKey = {key, *it};
      m_cache.remove(cacheKey);
      ++it;
   }
}

bool QGLTextureCache::remove(QGLContext *ctx, GLuint textureId)
{
   QWriteLocker locker(&m_lock);
   QList<QGLTextureCacheKey> keys = m_cache.keys();

   for (int i = 0; i < keys.size(); ++i) {
      QGLTexture *tex = m_cache.object(keys.at(i));

      if (tex->id == textureId && tex->context == ctx) {
         tex->options |= QGLContext::MemoryManagedBindOption; // forces a glDeleteTextures() call
         m_cache.remove(keys.at(i));
         return true;
      }
   }
   return false;
}

void QGLTextureCache::removeContextTextures(QGLContext *ctx)
{
   QWriteLocker locker(&m_lock);
   QList<QGLTextureCacheKey> keys = m_cache.keys();

   for (int i = 0; i < keys.size(); ++i) {
      const QGLTextureCacheKey &key = keys.at(i);
      if (m_cache.object(key)->context == ctx) {
         m_cache.remove(key);
      }
   }
}

void QGLTextureCache::cleanupTexturesForCacheKey(qint64 cacheKey)
{
   qt_gl_texture_cache()->remove(cacheKey);
}

void QGLTextureCache::cleanupTexturesForPixampData(QPlatformPixmap *pmd)
{
   cleanupTexturesForCacheKey(pmd->cacheKey());
}

void QGLTextureCache::cleanupBeforePixmapDestruction(QPlatformPixmap *pmd)
{
   // Remove any bound textures first:
   cleanupTexturesForPixampData(pmd);
}

QGLTextureCache *QGLTextureCache::instance()
{
   return qt_gl_texture_cache();
}

// DDS format structure
struct DDSFormat {
   quint32 dwSize;
   quint32 dwFlags;
   quint32 dwHeight;
   quint32 dwWidth;
   quint32 dwLinearSize;
   quint32 dummy1;
   quint32 dwMipMapCount;
   quint32 dummy2[11];

   struct {
      quint32 dummy3[2];
      quint32 dwFourCC;
      quint32 dummy4[5];
   } ddsPixelFormat;
};

// compressed texture pixel formats
#define FOURCC_DXT1  0x31545844
#define FOURCC_DXT2  0x32545844
#define FOURCC_DXT3  0x33545844
#define FOURCC_DXT4  0x34545844
#define FOURCC_DXT5  0x35545844

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif

#ifndef GL_GENERATE_MIPMAP_SGIS
#define GL_GENERATE_MIPMAP_SGIS       0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS  0x8192
#endif

QGLContext::QGLContext(const QGLFormat &format, QPaintDevice *device)
   : d_ptr(new QGLContextPrivate(this))
{
   Q_D(QGLContext);
   d->init(device, format);
}

QGLContext::QGLContext(const QGLFormat &format)
   : d_ptr(new QGLContextPrivate(this))
{
   Q_D(QGLContext);
   d->init(nullptr, format);
}

static void qDeleteQGLContext(void *handle)
{
   QGLContext *context = static_cast<QGLContext *>(handle);
   delete context;
}

QGLContext::QGLContext(QOpenGLContext *context)
   : d_ptr(new QGLContextPrivate(this))
{
   Q_D(QGLContext);
   d->init(nullptr, QGLFormat::fromSurfaceFormat(context->format()));
   d->guiGlContext = context;
   d->guiGlContext->setQGLContextHandle(this, qDeleteQGLContext);
   d->ownContext = false;
   d->valid = context->isValid();
   d->setupSharing();
}

QOpenGLContext *QGLContext::contextHandle() const
{
   Q_D(const QGLContext);
   return d->guiGlContext;
}

QGLContext *QGLContext::fromOpenGLContext(QOpenGLContext *context)
{
   if (!context) {
      return nullptr;
   }

   if (context->qGLContextHandle()) {
      return reinterpret_cast<QGLContext *>(context->qGLContextHandle());
   }

   QGLContext *glContext = new QGLContext(context);

   return glContext;
}

QGLContext::~QGLContext()
{
   // remove any textures cached in this context
   QGLTextureCache::instance()->removeContextTextures(this);

   // clean up resources specific to this context
   d_ptr->cleanup();

   QGLSignalProxy::instance()->emitAboutToDestroyContext(this);
   reset();
}

void QGLContextPrivate::cleanup()
{
}

#define ctx q_ptr
void QGLContextPrivate::setVertexAttribArrayEnabled(int arrayIndex, bool enabled)
{
   Q_Q(QGLContext);
   Q_ASSERT(arrayIndex < QT_GL_VERTEX_ARRAY_TRACKED_COUNT);

#ifdef glEnableVertexAttribArray
   Q_ASSERT(glEnableVertexAttribArray);
#endif

   if (vertexAttributeArraysEnabledState[arrayIndex] && !enabled) {
      q->functions()->glDisableVertexAttribArray(arrayIndex);
   }

   if (!vertexAttributeArraysEnabledState[arrayIndex] && enabled) {
      q->functions()->glEnableVertexAttribArray(arrayIndex);
   }

   vertexAttributeArraysEnabledState[arrayIndex] = enabled;
}

void QGLContextPrivate::syncGlState()
{
   Q_Q(QGLContext);
#ifdef glEnableVertexAttribArray
   Q_ASSERT(glEnableVertexAttribArray);
#endif
   for (int i = 0; i < QT_GL_VERTEX_ARRAY_TRACKED_COUNT; ++i) {
      if (vertexAttributeArraysEnabledState[i]) {
         q->functions()->glEnableVertexAttribArray(i);
      } else {
         q->functions()->glDisableVertexAttribArray(i);
      }
   }

}
#undef ctx
void QGLContextPrivate::swapRegion(const QRegion &)
{
   Q_Q(QGLContext);
   q->swapBuffers();
}

GLuint QGLContext::bindTexture(const QString &fileName)
{
   QGLTexture texture(this);
   QSize size = texture.bindCompressedTexture(fileName);

   if (! size.isValid()) {
      return 0;
   }

   return texture.id;
}

static inline QRgb qt_gl_convertToGLFormatHelper(QRgb src_pixel, GLenum texture_format)
{
   if (texture_format == GL_BGRA) {
      if constexpr (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
         return ((src_pixel << 24) & 0xff000000)
            | ((src_pixel >> 24) & 0x000000ff)
            | ((src_pixel << 8) & 0x00ff0000)
            | ((src_pixel >> 8) & 0x0000ff00);
      } else {
         return src_pixel;
      }
   } else {  // GL_RGBA
      if constexpr (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
         return (src_pixel << 8) | ((src_pixel >> 24) & 0xff);
      } else {
         return ((src_pixel << 16) & 0xff0000)
            | ((src_pixel >> 16) & 0xff)
            | (src_pixel & 0xff00ff00);
      }
   }
}

static void convertToGLFormatHelper(QImage &dst, const QImage &img, GLenum texture_format)
{
   Q_ASSERT(dst.depth() == 32);
   Q_ASSERT(img.depth() == 32);

   if (dst.size() != img.size()) {
      int target_width = dst.width();
      int target_height = dst.height();
      qreal sx = target_width / qreal(img.width());
      qreal sy = target_height / qreal(img.height());

      quint32 *dest = (quint32 *) dst.scanLine(0); // NB! avoid detach here
      const uchar *srcPixels = img.constScanLine(img.height() - 1);
      int sbpl = img.bytesPerLine();
      int dbpl = dst.bytesPerLine();

      int ix = int(0x00010000 / sx);
      int iy = int(0x00010000 / sy);

      quint32 basex = int(0.5 * ix);
      quint32 srcy = int(0.5 * iy);

      // scale, swizzle and mirror in one loop
      while (target_height--) {
         const uint *src = (const quint32 *) (srcPixels - (srcy >> 16) * sbpl);
         int srcx = basex;
         for (int x = 0; x < target_width; ++x) {
            dest[x] = qt_gl_convertToGLFormatHelper(src[srcx >> 16], texture_format);
            srcx += ix;
         }
         dest = (quint32 *)(((uchar *) dest) + dbpl);
         srcy += iy;
      }
   } else {
      const int width = img.width();
      const int height = img.height();
      const uint *p = (const uint *) img.scanLine(img.height() - 1);
      uint *q = (uint *) dst.scanLine(0);

      if (texture_format == GL_BGRA) {
         if constexpr (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            // mirror + swizzle
            for (int i = 0; i < height; ++i) {
               const uint *end = p + width;
               while (p < end) {
                  *q = ((*p << 24) & 0xff000000)
                     | ((*p >> 24) & 0x000000ff)
                     | ((*p << 8) & 0x00ff0000)
                     | ((*p >> 8) & 0x0000ff00);
                  p++;
                  q++;
               }
               p -= 2 * width;
            }

         } else {
            const uint bytesPerLine = img.bytesPerLine();
            for (int i = 0; i < height; ++i) {
               memcpy(q, p, bytesPerLine);
               q += width;
               p -= width;
            }
         }

      } else {
         if constexpr (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            for (int i = 0; i < height; ++i) {
               const uint *end = p + width;

               while (p < end) {
                  *q = (*p << 8) | ((*p >> 24) & 0xff);
                  p++;
                  q++;
               }

               p -= 2 * width;
            }

         } else {
            for (int i = 0; i < height; ++i) {
               const uint *end = p + width;
               while (p < end) {
                  *q = ((*p << 16) & 0xff0000) | ((*p >> 16) & 0xff) | (*p & 0xff00ff00);
                  p++;
                  q++;
               }

               p -= 2 * width;
            }
         }
      }
   }
}

/*! \internal */
QGLTexture *QGLContextPrivate::bindTexture(const QImage &image, GLenum target, GLint format,
   QGLContext::BindOptions options)
{
   Q_Q(QGLContext);

   const qint64 key    = image.cacheKey();
   QGLTexture *texture = textureCacheLookup(key, target);

   if (texture) {
      if (image.paintingActive()) {
         // A QPainter is active on the image - take the safe route and replace the texture.
         q->deleteTexture(texture->id);
         texture = nullptr;
      } else {
         qgl_functions()->glBindTexture(target, texture->id);
         return texture;
      }
   }

   if (! texture) {
      texture = bindTexture(image, target, format, key, options);
   }

   // bindTexture(const QImage&, GLenum, GLint, const qint64, bool) should never return null
   Q_ASSERT(texture);

   // Enable the cleanup hooks for this image so that the texture cache entry is removed when the
   // image gets deleted:
   QImagePixmapCleanupHooks::enableCleanupHooks(image);

   return texture;
}

#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif

// map from our ARGB endianness dependent format to GL's big-endian RGBA layout
static inline void qgl_byteSwapImage(QImage &img, GLenum pixel_type)
{
   const int width = img.width();
   const int height = img.height();

   if (pixel_type == GL_UNSIGNED_INT_8_8_8_8_REV
         || (pixel_type == GL_UNSIGNED_BYTE && QSysInfo::ByteOrder == QSysInfo::LittleEndian)) {
      for (int i = 0; i < height; ++i) {
         uint *p = (uint *) img.scanLine(i);
         for (int x = 0; x < width; ++x) {
            p[x] = ((p[x] << 16) & 0xff0000) | ((p[x] >> 16) & 0xff) | (p[x] & 0xff00ff00);
         }
      }
   } else {
      for (int i = 0; i < height; ++i) {
         uint *p = (uint *) img.scanLine(i);
         for (int x = 0; x < width; ++x) {
            p[x] = (p[x] << 8) | ((p[x] >> 24) & 0xff);
         }
      }
   }
}

QGLTexture *QGLContextPrivate::bindTexture(const QImage &image, GLenum target, GLint internalFormat,
   const qint64 key, QGLContext::BindOptions options)
{
   Q_Q(QGLContext);

   QOpenGLFunctions *funcs = qgl_functions();

#if defined(CS_SHOW_DEBUG_OPENGL)
   qDebug("QGLContext::bindTexture() imageSize = %dx%d, internalFormat = 0x%x, options = %x, key = %llx",
      image.width(), image.height(), internalFormat, int(options), key);

   QTime time;
   time.start();

   // Reset the gl error stack
   while (funcs->glGetError() != GL_NO_ERROR) {
      // do nothing
   }
#endif

   // Scale the pixmap if needed. GL textures needs to have the
   // dimensions 2^n+2(border) x 2^m+2(border), unless we're using GL
   // 2.0 or use the GL_TEXTURE_RECTANGLE texture target
   int tx_w = qNextPowerOfTwo(image.width() - 1);
   int tx_h = qNextPowerOfTwo(image.height() - 1);

   QImage img = image;

   if (! qgl_extensions()->hasOpenGLFeature(QOpenGLFunctions::NPOTTextures)
         && !(QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Version_2_0)
         && (target == GL_TEXTURE_2D && (tx_w != image.width() || tx_h != image.height()))) {
      img = img.scaled(tx_w, tx_h);

#if defined(CS_SHOW_DEBUG_OPENGL)
      qDebug("QGLContext::bindTexture() Upscaled to %dx%d (%d ms)", tx_w, tx_h, time.elapsed());
#endif
   }

   GLuint filtering = options & QGLContext::LinearFilteringBindOption ? GL_LINEAR : GL_NEAREST;

   GLuint tx_id;
   funcs->glGenTextures(1, &tx_id);
   funcs->glBindTexture(target, tx_id);
   funcs->glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filtering);

   QOpenGLContext *ctx = QOpenGLContext::currentContext();
   bool genMipmap = !ctx->isOpenGLES();

   if (glFormat.directRendering()
      && (qgl_extensions()->hasOpenGLExtension(QOpenGLExtensions::GenerateMipmap))
      && target == GL_TEXTURE_2D
      && (options & QGLContext::MipmapBindOption)) {
#if !defined(QT_OPENGL_ES_2)
      if (genMipmap) {
         funcs->glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
         funcs->glTexParameteri(target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
      } else {
         funcs->glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
         genMipmap = true;
      }
#else
      funcs->glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
      genMipmap = true;
#endif

      funcs->glTexParameteri(target, GL_TEXTURE_MIN_FILTER, options & QGLContext::LinearFilteringBindOption
         ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);

#if defined(CS_SHOW_DEBUG_OPENGL)
      qDebug("QGLContext::bindTexture() Generating mipmaps (%d ms)", time.elapsed());
#endif

   } else {
      funcs->glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filtering);
   }

   QImage::Format target_format = img.format();
   bool premul = options & QGLContext::PremultipliedAlphaBindOption;
   bool needsbyteswap = true;

   GLenum externalFormat;
   GLuint pixel_type;
   if (target_format == QImage::Format_RGBA8888
      || target_format == QImage::Format_RGBA8888_Premultiplied
      || target_format == QImage::Format_RGBX8888) {
      externalFormat = GL_RGBA;
      pixel_type = GL_UNSIGNED_BYTE;
      needsbyteswap = false;

   } else if (qgl_extensions()->hasOpenGLExtension(QOpenGLExtensions::BGRATextureFormat)) {
      externalFormat = GL_BGRA;
      needsbyteswap = false;

      if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_2) {
         pixel_type = GL_UNSIGNED_INT_8_8_8_8_REV;
      } else {
         pixel_type = GL_UNSIGNED_BYTE;
      }

   } else {
      externalFormat = GL_RGBA;
      pixel_type = GL_UNSIGNED_BYTE;
   }

   switch (target_format) {
      case QImage::Format_ARGB32:
         if (premul) {
            img = img.convertToFormat(target_format = QImage::Format_ARGB32_Premultiplied);

#if defined(CS_SHOW_DEBUG_OPENGL)
            qDebug("QGLContext::bindTexture() Converted ARGB32 to ARGB32_Premultiplied (%d ms)",
                  time.elapsed());
#endif
         }
         break;

      case QImage::Format_ARGB32_Premultiplied:
         if (! premul) {
            img = img.convertToFormat(target_format = QImage::Format_ARGB32);

#if defined(CS_SHOW_DEBUG_OPENGL)
            printf("Converted ARGB32_Premultiplied to ARGB32 (%d ms)\n", time.elapsed());
#endif
         }
         break;

      case QImage::Format_RGBA8888:
         if (premul) {
            img = img.convertToFormat(target_format = QImage::Format_RGBA8888_Premultiplied);

#if defined(CS_SHOW_DEBUG_OPENGL)
            qDebug("QGLContext::bindTexture() Converted RGBA8888 to RGBA8888_Premultiplied (%d ms)",
                  time.elapsed());
#endif
         }
         break;

      case QImage::Format_RGBA8888_Premultiplied:
         if (! premul) {
            img = img.convertToFormat(target_format = QImage::Format_RGBA8888);

#if defined(CS_SHOW_DEBUG_OPENGL)
            qDebug("QGLContext::bindTexture() Converted RGBA8888_Premultiplied to RGBA8888 (%d ms)",
                  time.elapsed());
#endif
         }
         break;

      case QImage::Format_RGB16:
         pixel_type = GL_UNSIGNED_SHORT_5_6_5;
         externalFormat = GL_RGB;
         internalFormat = GL_RGB;
         needsbyteswap = false;
         break;

      case QImage::Format_RGB32:
      case QImage::Format_RGBX8888:
         break;

      default:
         if (img.hasAlphaChannel()) {
            img = img.convertToFormat(premul ? QImage::Format_ARGB32_Premultiplied : QImage::Format_ARGB32);

#if defined(CS_SHOW_DEBUG_OPENGL)
            qDebug("QGLContext::bindTexture() Converted to 32-bit alpha format (%d ms)", time.elapsed());
#endif

         } else {
            img = img.convertToFormat(QImage::Format_RGB32);

#if defined(CS_SHOW_DEBUG_OPENGL)
            qDebug("QGLContext::bindTexture() Converted to 32-bit (%d ms)", time.elapsed());
#endif
         }
   }

   if (options & QGLContext::InvertedYBindOption) {
      if (img.isDetached()) {
         int ipl = img.bytesPerLine() / 4;
         int h = img.height();

         for (int y = 0; y < h / 2; ++y) {
            int *a = (int *) img.scanLine(y);
            int *b = (int *) img.scanLine(h - y - 1);
            for (int x = 0; x < ipl; ++x) {
               qSwap(a[x], b[x]);
            }
         }

      } else {
         // Create a new image and copy across.  If we use the
         // above in-place code then a full copy of the image is
         // made before the lines are swapped, which processes the
         // data twice.  This version should only do it once.
         img = img.mirrored();
      }

#if defined(CS_SHOW_DEBUG_OPENGL)
      qDebug("QGLContext::bindTexture() Flipped bits over y (%d ms)", time.elapsed());
#endif
   }

   if (needsbyteswap) {
      // The only case where we end up with a depth different from
      // 32 in the switch above is for the RGB16 case, where we set
      // the format to GL_RGB
      Q_ASSERT(img.depth() == 32);
      qgl_byteSwapImage(img, pixel_type);

#if defined(CS_SHOW_DEBUG_OPENGL)
      qDebug("QGLContext::bindTexture() Byte swapping (%d ms)", time.elapsed());
#endif
   }

   if (ctx->isOpenGLES()) {
      // OpenGL/ES requires that the internal and external formats be identical
      internalFormat = externalFormat;
   }

#if defined(CS_SHOW_DEBUG_OPENGL)
   qDebug("QGLContext::bindTexture() Uploading, image format = %d, externalFormat = 0x%x,"
         "internalFormat = 0x%x, pixel_type = 0x%x", img.format(), externalFormat, internalFormat, pixel_type);
#endif

   const QImage &constRef = img; // to avoid detach in bits()

   funcs->glTexImage2D(target, 0, internalFormat, img.width(), img.height(), 0, externalFormat,
      pixel_type, constRef.bits());

   if (genMipmap && ctx->isOpenGLES()) {
      q->functions()->glGenerateMipmap(target);
   }

#if defined(CS_SHOW_DEBUG_OPENGL)
   GLenum error = funcs->glGetError();
   if (error != GL_NO_ERROR) {
      qDebug("QGLContext::bindTexture() Texture upload failed, error code = 0x%x, enum = %d (%x)",
            error, target, target);
   }
#endif

#if defined(CS_SHOW_DEBUG_OPENGL)
   static int totalUploadTime = 0;
   totalUploadTime += time.elapsed();
   qDebug("QGLContext::bindTexture() Upload done in %d msecs, accumulated time = %d msecs",
         time.elapsed(), totalUploadTime);
#endif

   // this assumes the size of a texture is always smaller than the max cache size
   int cost = img.width() * img.height() * 4 / 1024;
   QGLTexture *texture = new QGLTexture(q, tx_id, target, options);
   QGLTextureCache::instance()->insert(q, key, texture, cost);

   return texture;
}

QGLTexture *QGLContextPrivate::textureCacheLookup(const qint64 key, GLenum target)
{
   Q_Q(QGLContext);
   QGLTexture *texture = QGLTextureCache::instance()->getTexture(q, key);

   if (texture && texture->target == target && (texture->context == q || QGLContext::areSharing(q, texture->context))) {
      return texture;
   }

   return nullptr;
}

QGLTexture *QGLContextPrivate::bindTexture(const QPixmap &pixmap, GLenum target, GLint format,
      QGLContext::BindOptions options)
{
   Q_Q(QGLContext);

   const qint64 key = pixmap.cacheKey();
   QGLTexture *texture = textureCacheLookup(key, target);

   if (texture) {
      if (pixmap.paintingActive()) {
         // A QPainter is active on the pixmap - take the safe route and replace the texture.
         q->deleteTexture(texture->id);
         texture = nullptr;

      } else {
         qgl_functions()->glBindTexture(target, texture->id);
         return texture;
      }
   }

   if (! texture) {
      QImage image;
      QPaintEngine *paintEngine = pixmap.paintEngine();

      if (! paintEngine || paintEngine->type() != QPaintEngine::Raster) {
         image = pixmap.toImage();

      } else {
         // QRasterPixmapData::toImage() will deep-copy the backing QImage if there's an active QPainter on it.
         // For performance reasons, we don't want that here, so we temporarily redirect the paint engine.

         QPaintDevice *currentPaintDevice = paintEngine->paintDevice();
         paintEngine->setPaintDevice(nullptr);
         image = pixmap.toImage();
         paintEngine->setPaintDevice(currentPaintDevice);
      }

      // If the system depth is 16 and the pixmap doesn't have an alpha channel
      // then we convert it to RGB16 in the hope that it gets uploaded as a 16
      // bit texture which is much faster to access than a 32-bit one.
      if (pixmap.depth() == 16 && !image.hasAlphaChannel() ) {
         image = image.convertToFormat(QImage::Format_RGB16);
      }

      texture = bindTexture(image, target, format, key, options);
   }

   // bindTexture(const QImage&, GLenum, GLint, const qint64, bool)  should never return null
   Q_ASSERT(texture);

   if (texture->id > 0) {
      QImagePixmapCleanupHooks::enableCleanupHooks(pixmap);
   }

   return texture;
}

//
int QGLContextPrivate::maxTextureSize()
{
   if (max_texture_size != -1) {
      return max_texture_size;
   }

   QOpenGLFunctions *funcs = qgl_functions();
   funcs->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

#ifndef QT_OPENGL_ES
   Q_Q(QGLContext);

   if (! q->contextHandle()->isOpenGLES()) {
      GLenum proxy = GL_PROXY_TEXTURE_2D;

      GLint size;
      GLint next = 64;
      funcs->glTexImage2D(proxy, 0, GL_RGBA, next, next, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      QOpenGLFunctions_1_1 *gl1funcs = qgl1_functions();
      gl1funcs->glGetTexLevelParameteriv(proxy, 0, GL_TEXTURE_WIDTH, &size);

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
         gl1funcs->glGetTexLevelParameteriv(proxy, 0, GL_TEXTURE_WIDTH, &next);

      } while (next > size);

      max_texture_size = size;
   }
#endif

   return max_texture_size;
}

QGLFunctions *QGLContext::functions() const
{
   QGLContextPrivate *d = const_cast<QGLContextPrivate *>(d_func());
   if (!d->functions) {
      d->functions = new QGLFunctions(this);
      d->functions->initializeGLFunctions(this);
   }
   return d->functions;
}

GLuint QGLContext::bindTexture(const QImage &image, GLenum target, GLint format)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(image, target, format, DefaultBindOption);
   return texture->id;
}

GLuint QGLContext::bindTexture(const QImage &image, GLenum target, GLint format, BindOptions options)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(image, target, format, options);
   return texture->id;
}

GLuint QGLContext::bindTexture(const QPixmap &pixmap, GLenum target, GLint format)
{
   if (pixmap.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(pixmap, target, format, DefaultBindOption);
   return texture->id;
}

GLuint QGLContext::bindTexture(const QPixmap &pixmap, GLenum target, GLint format, BindOptions options)
{
   if (pixmap.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(pixmap, target, format, options);
   return texture->id;
}

void QGLContext::deleteTexture(GLuint id)
{
   if (QGLTextureCache::instance()->remove(this, id)) {
      return;
   }

   qgl_functions()->glDeleteTextures(1, &id);
}

void qt_add_rect_to_array(const QRectF &r, GLfloat *array)
{
   qreal left = r.left();
   qreal right = r.right();
   qreal top = r.top();
   qreal bottom = r.bottom();

   array[0] = left;
   array[1] = top;
   array[2] = right;
   array[3] = top;
   array[4] = right;
   array[5] = bottom;
   array[6] = left;
   array[7] = bottom;
}

void qt_add_texcoords_to_array(qreal x1, qreal y1, qreal x2, qreal y2, GLfloat *array)
{
   array[0] = x1;
   array[1] = y1;
   array[2] = x2;
   array[3] = y1;
   array[4] = x2;
   array[5] = y2;
   array[6] = x1;
   array[7] = y2;
}

#if ! defined(QT_OPENGL_ES_2)

static void qDrawTextureRect(const QRectF &target, GLint textureWidth, GLint textureHeight, GLenum textureTarget)
{
   QOpenGLFunctions *funcs = qgl_functions();
   GLfloat tx = 1.0f;
   GLfloat ty = 1.0f;

#ifdef QT_OPENGL_ES
    (void) textureWidth;
    (void) textureHeight;
    (void) textureTarget;

#else
   if (textureTarget != GL_TEXTURE_2D && !QOpenGLContext::currentContext()->isOpenGLES()) {
      if (textureWidth == -1 || textureHeight == -1) {
         QOpenGLFunctions_1_1 *gl1funcs = qgl1_functions();
         gl1funcs->glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_WIDTH, &textureWidth);
         gl1funcs->glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_HEIGHT, &textureHeight);
      }

      tx = GLfloat(textureWidth);
      ty = GLfloat(textureHeight);
   }
#endif

   GLfloat texCoordArray[4 * 2] = {
      0, ty, tx, ty, tx, 0, 0, 0
   };

   GLfloat vertexArray[4 * 2];
   qt_add_rect_to_array(target, vertexArray);

   QOpenGLFunctions_1_1 *gl1funcs = qgl1_functions();
   gl1funcs->glVertexPointer(2, GL_FLOAT, 0, vertexArray);
   gl1funcs->glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

   gl1funcs->glEnableClientState(GL_VERTEX_ARRAY);
   gl1funcs->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   funcs->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

   gl1funcs->glDisableClientState(GL_VERTEX_ARRAY);
   gl1funcs->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

#endif // !QT_OPENGL_ES_2

void QGLContext::drawTexture(const QRectF &target, GLuint textureId, GLenum textureTarget)
{
#if !defined(QT_OPENGL_ES) || defined(QT_OPENGL_ES_2)

   if (d_ptr->active_engine &&
      d_ptr->active_engine->type() == QPaintEngine::OpenGL2) {

      QGL2PaintEngineEx *eng = static_cast<QGL2PaintEngineEx *>(d_ptr->active_engine);

      if (!eng->isNativePaintingActive()) {
         QRectF src(0, 0, target.width(), target.height());
         QSize size(target.width(), target.height());
         if (eng->drawTexture(target, textureId, size, src)) {
            return;
         }
      }
   }
#endif


#ifndef QT_OPENGL_ES_2
   QOpenGLFunctions *funcs = qgl_functions();
   if (!contextHandle()->isOpenGLES()) {

#ifdef QT_OPENGL_ES
      if (textureTarget != GL_TEXTURE_2D) {
         qWarning("QGLContext::drawTexture(): texture target must be GL_TEXTURE_2D on OpenGL ES");
         return;
      }
#else
      const bool wasEnabled = funcs->glIsEnabled(GL_TEXTURE_2D);
      GLint oldTexture;
      funcs->glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
#endif

      funcs->glEnable(textureTarget);
      funcs->glBindTexture(textureTarget, textureId);

      qDrawTextureRect(target, -1, -1, textureTarget);

#ifdef QT_OPENGL_ES
      funcs->glDisable(textureTarget);
#else
      if (! wasEnabled) {
         funcs->glDisable(textureTarget);
      }

      funcs->glBindTexture(textureTarget, oldTexture);
#endif
      return;
   }

#else
   (void) target;
   (void) textureId;
   (void) textureTarget;
#endif

   qWarning("drawTexture() with OpenGL ES 2.0 requires an active OpenGL2 paint engine");
}

void QGLContext::drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget)
{
#ifdef QT_OPENGL_ES
   (void) point;
   (void) textureId;
   (void) textureTarget;

#else
   if (! contextHandle()->isOpenGLES()) {
      QOpenGLFunctions *funcs = qgl_functions();
      const bool wasEnabled = funcs->glIsEnabled(GL_TEXTURE_2D);
      GLint oldTexture;
      funcs->glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);

      funcs->glEnable(textureTarget);
      funcs->glBindTexture(textureTarget, textureId);

      GLint textureWidth;
      GLint textureHeight;

      QOpenGLFunctions_1_1 *gl1funcs = qgl1_functions();
      gl1funcs->glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_WIDTH, &textureWidth);
      gl1funcs->glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_HEIGHT, &textureHeight);

      if (d_ptr->active_engine &&
         d_ptr->active_engine->type() == QPaintEngine::OpenGL2) {
         QGL2PaintEngineEx *eng = static_cast<QGL2PaintEngineEx *>(d_ptr->active_engine);

         if (!eng->isNativePaintingActive()) {
            QRectF dest(point, QSizeF(textureWidth, textureHeight));
            QRectF src(0, 0, textureWidth, textureHeight);
            QSize size(textureWidth, textureHeight);
            if (eng->drawTexture(dest, textureId, size, src)) {
               return;
            }
         }
      }

      qDrawTextureRect(QRectF(point, QSizeF(textureWidth, textureHeight)), textureWidth, textureHeight, textureTarget);

      if (!wasEnabled) {
         funcs->glDisable(textureTarget);
      }
      funcs->glBindTexture(textureTarget, oldTexture);
      return;
   }
#endif

   qWarning("drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget) not supported with OpenGL ES, use rect version instead");
}

void QGLContext::setTextureCacheLimit(int size)
{
   QGLTextureCache::instance()->setMaxCost(size);
}

int QGLContext::textureCacheLimit()
{
   return QGLTextureCache::instance()->maxCost();
}

void QGLContext::setFormat(const QGLFormat &format)
{
   Q_D(QGLContext);
   reset();
   d->glFormat = d->reqFormat = format;
}

// internal
void QGLContext::setDevice(QPaintDevice *pDev)
{
   Q_D(QGLContext);

   d->paintDevice = pDev;
   if (d->paintDevice && (d->paintDevice->devType() != QInternal::Widget
         && d->paintDevice->devType() != QInternal::Pixmap
         && d->paintDevice->devType() != QInternal::Pbuffer)) {
      qWarning("QGLContext: Unsupported paint device type");
   }
}

bool QGLContext::areSharing(const QGLContext *context1, const QGLContext *context2)
{
   if (!context1 || !context2) {
      return false;
   }
   return context1->d_ptr->group == context2->d_ptr->group;
}

uint QGLContext::colorIndex(const QColor &) const
{
   return 0;
}

QColor QGLContext::overlayTransparentColor() const
{
   return QColor(); // Invalid color
}

bool QGLContext::create(const QGLContext *shareContext)
{
   Q_D(QGLContext);

   if (!d->paintDevice && !d->guiGlContext) {
      return false;
   }

   reset();
   d->valid = chooseContext(shareContext);

   if (d->valid && d->paintDevice && d->paintDevice->devType() == QInternal::Widget) {
      QWidgetPrivate *wd = qt_widget_private(static_cast<QWidget *>(d->paintDevice));
      wd->usesDoubleBufferedGLContext = d->glFormat.doubleBuffer();
   }

   return d->valid;
}

bool QGLContext::isValid() const
{
   Q_D(const QGLContext);
   return d->valid;
}

void QGLContext::setValid(bool valid)
{
   Q_D(QGLContext);
   d->valid = valid;
}

bool QGLContext::isSharing() const
{
   Q_D(const QGLContext);
   return d->group->isSharing();
}

QGLFormat QGLContext::format() const
{
   Q_D(const QGLContext);
   return d->glFormat;
}

QGLFormat QGLContext::requestedFormat() const
{
   Q_D(const QGLContext);
   return d->reqFormat;
}

QPaintDevice *QGLContext::device() const
{
   Q_D(const QGLContext);
   return d->paintDevice;
}

bool QGLContext::deviceIsPixmap() const
{
   Q_D(const QGLContext);
   return !d->readback_target_size.isEmpty();
}

bool QGLContext::windowCreated() const
{
   Q_D(const QGLContext);
   return d->crWin;
}

void QGLContext::setWindowCreated(bool on)
{
   Q_D(QGLContext);
   d->crWin = on;
}

bool QGLContext::initialized() const
{
   Q_D(const QGLContext);
   return d->initDone;
}

void QGLContext::setInitialized(bool on)
{
   Q_D(QGLContext);
   d->initDone = on;
}

const QGLContext *QGLContext::currentContext()
{
   if (const QOpenGLContext *threadContext = QOpenGLContext::currentContext()) {
      return QGLContext::fromOpenGLContext(const_cast<QOpenGLContext *>(threadContext));
   }

   return nullptr;
}

void QGLContextPrivate::setCurrentContext(QGLContext *context)
{
   (void) context;
}

void QGLContext::moveToThread(QThread *thread)
{
   Q_D(QGLContext);
   if (d->guiGlContext) {
      d->guiGlContext->moveToThread(thread);
   }
}

bool QGLContext::chooseContext(const QGLContext *shareContext)
{
   Q_D(QGLContext);
   if (!d->paintDevice || d->paintDevice->devType() != QInternal::Widget) {
      // Only possible target is a widget backed by an OpenGL-based QWindow.
      // Pixmaps in particular are not supported as paint devices

      d->valid = false;

   } else {
      QWidget *widget = static_cast<QWidget *>(d->paintDevice);
      QGLFormat glformat = format();
      QSurfaceFormat winFormat = QGLFormat::toSurfaceFormat(glformat);

   if (widget->testAttribute(Qt::WA_TranslucentBackground)) {
         winFormat.setAlphaBufferSize(qMax(winFormat.alphaBufferSize(), 8));
      }

      QWindow *window = widget->windowHandle();
      if (!window->handle()
         || window->surfaceType() != QWindow::OpenGLSurface
         || window->requestedFormat() != winFormat) {
         window->setSurfaceType(QWindow::OpenGLSurface);
         window->setFormat(winFormat);
         window->destroy();
         window->create();
      }

      if (d->ownContext) {
         delete d->guiGlContext;
      }

      d->ownContext = true;
      QOpenGLContext *shareGlContext = shareContext ? shareContext->d_func()->guiGlContext : nullptr;
      d->guiGlContext = new QOpenGLContext;
      d->guiGlContext->setFormat(winFormat);
      d->guiGlContext->setShareContext(shareGlContext);
      d->valid = d->guiGlContext->create();

      if (d->valid) {
         d->guiGlContext->setQGLContextHandle(this, nullptr);
      }

      d->glFormat = QGLFormat::fromSurfaceFormat(d->guiGlContext->format());
      d->setupSharing();
   }

   return d->valid;
}

void QGLContext::reset()
{
   Q_D(QGLContext);
   if (! d->valid) {
      return;
   }

   d->cleanup();

   d->crWin    = false;
   d->sharing  = false;
   d->valid    = false;
   d->transpColor = QColor();
   d->initDone = false;

   QGLContextGroup::removeShare(this);

   if (d->guiGlContext) {
      if (QOpenGLContext::currentContext() == d->guiGlContext) {
         doneCurrent();
      }

      if (d->ownContext) {
         if (d->guiGlContext->thread() == QThread::currentThread()) {
            delete d->guiGlContext;
         } else {
            d->guiGlContext->deleteLater();
         }

      } else {
         d->guiGlContext->setQGLContextHandle(nullptr, nullptr);
      }

      d->guiGlContext = nullptr;
   }

   d->ownContext = false;
}

void QGLContext::makeCurrent()
{
   Q_D(QGLContext);
   if (! d->paintDevice || d->paintDevice->devType() != QInternal::Widget) {
      return;
   }

   QWidget *widget = static_cast<QWidget *>(d->paintDevice);
   if (! widget->windowHandle()) {
      return;
   }

   if (d->guiGlContext->makeCurrent(widget->windowHandle())) {
      if (! d->workaroundsCached) {
         d->workaroundsCached = true;
         const char *renderer = reinterpret_cast<const char *>(d->guiGlContext->functions()->glGetString(GL_RENDERER));
         if (renderer && strstr(renderer, "Mali")) {
            d->workaround_brokenFBOReadBack = true;
         }
      }
   }
}

void QGLContext::swapBuffers() const
{
   Q_D(const QGLContext);
   if (!d->paintDevice || d->paintDevice->devType() != QInternal::Widget) {
      return;
   }

   QWidget *widget = static_cast<QWidget *>(d->paintDevice);
   if (!widget->windowHandle()) {
      return;
   }

   d->guiGlContext->swapBuffers(widget->windowHandle());
}

void QGLContext::doneCurrent()
{
   Q_D(QGLContext);
   d->guiGlContext->doneCurrent();
}

QGLWidget::QGLWidget(QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags flags)
   : QWidget(*(new QGLWidgetPrivate), parent, flags | Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(new QGLContext(QGLFormat::defaultFormat(), this), shareWidget);
}

QGLWidget::QGLWidget(QGLWidgetPrivate &dd, const QGLFormat &format, QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags flags)
   : QWidget(dd, parent, flags| Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(new QGLContext(format, this), shareWidget);
}

QGLWidget::QGLWidget(const QGLFormat &format, QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags flags)
   : QWidget(*(new QGLWidgetPrivate), parent, flags | Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(new QGLContext(format, this), shareWidget);
}

QGLWidget::QGLWidget(QGLContext *context, QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags flags)
   : QWidget(*(new QGLWidgetPrivate), parent, flags | Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(context, shareWidget);
}

QGLWidget::~QGLWidget()
{
   Q_D(QGLWidget);

   delete d->glcx;
   d->glcx = nullptr;
   d->cleanupColormaps();
}

QGLContext::FP_Void QGLContext::getProcAddress(const QString &procName) const
{
   Q_D(const QGLContext);
   return d->guiGlContext->getProcAddress(procName.toLatin1());
}

bool QGLWidget::isValid() const
{
   Q_D(const QGLWidget);
   return d->glcx && d->glcx->isValid();
}

bool QGLWidget::isSharing() const
{
   Q_D(const QGLWidget);
   return d->glcx->isSharing();
}

void QGLWidget::makeCurrent()
{
   Q_D(QGLWidget);
   d->glcx->makeCurrent();
}

void QGLWidget::doneCurrent()
{
   Q_D(QGLWidget);
   d->glcx->doneCurrent();
}

void QGLWidget::swapBuffers()
{
   Q_D(QGLWidget);
   d->glcx->swapBuffers();
}

const QGLContext *QGLWidget::overlayContext() const
{
   return nullptr;
}

void QGLWidget::makeOverlayCurrent()
{
}

void QGLWidget::setFormat(const QGLFormat &format)
{
   setContext(new QGLContext(format, this));
}

void QGLWidget::setContext(QGLContext *context, const QGLContext *shareContext, bool deleteOldContext)
{
   Q_D(QGLWidget);

   if (context == nullptr) {
      qWarning("QGLWidget::setContext: Unable to set null context");
      return;
   }

   if (context->device() == nullptr) {
      //context may refer to more than 1 window.
      //it is better to point to 1 of them than none of them.
      context->setDevice(this);
   }

   QGLContext *oldcx = d->glcx;
   d->glcx = context;

   if (! d->glcx->isValid()) {
      d->glcx->create(shareContext ? shareContext : oldcx);
   }

   if (deleteOldContext) {
      delete oldcx;
   }
}

void QGLWidget::updateGL()
{
   Q_D(QGLWidget);
   const bool targetIsOffscreen = !d->glcx->d_ptr->readback_target_size.isEmpty();

   if (updatesEnabled() && (testAttribute(Qt::WA_Mapped) || targetIsOffscreen)) {
      glDraw();
   }

}

void QGLWidget::updateOverlayGL()
{
}

void QGLWidget::initializeGL()
{
}

void QGLWidget::paintGL()
{
   qgl_functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void QGLWidget::resizeGL(int, int)
{
}

void QGLWidget::initializeOverlayGL()
{
}

void QGLWidget::paintOverlayGL()
{
}

void QGLWidget::resizeOverlayGL(int, int)
{
}

bool QGLWidget::event(QEvent *e)
{
   Q_D(QGLWidget);

   // A re-parent will destroy the window and re-create it. We should not reset the context while it happens.
   if (e->type() == QEvent::ParentAboutToChange) {
      d->parent_changing = true;
   }

   if (e->type() == QEvent::ParentChange) {
      d->parent_changing = false;
   }


   return QWidget::event(e);
}

void QGLWidget::paintEvent(QPaintEvent *)
{
   if (updatesEnabled()) {
      glDraw();
      updateOverlayGL();
   }
}

void QGLWidget::resizeEvent(QResizeEvent *e)
{
   Q_D(QGLWidget);

   QWidget::resizeEvent(e);
   if (!isValid()) {
      return;
   }
   makeCurrent();
   if (!d->glcx->initialized()) {
      glInit();
   }
   const qreal scaleFactor = (window() && window()->windowHandle()) ?
      window()->windowHandle()->devicePixelRatio() : 1.0;

   resizeGL(width() * scaleFactor, height() * scaleFactor);
}

QPixmap QGLWidget::renderPixmap(int w, int h, bool useContext)
{
   (void) useContext;

   Q_D(QGLWidget);

   QSize sz = size();
   if ((w > 0) && (h > 0)) {
      sz = QSize(w, h);
   }

   QPixmap pm;

   if (d->glcx->isValid()) {
      d->glcx->makeCurrent();
      QGLFramebufferObject fbo(sz, QGLFramebufferObject::CombinedDepthStencil);
      fbo.bind();
      d->glcx->setInitialized(false);
      uint prevDefaultFbo = d->glcx->d_ptr->default_fbo;
      d->glcx->d_ptr->default_fbo = fbo.handle();
      d->glcx->d_ptr->readback_target_size = sz;
      updateGL();
      fbo.release();

      pm = QPixmap::fromImage(fbo.toImage());
      d->glcx->d_ptr->default_fbo = prevDefaultFbo;
      d->glcx->setInitialized(false);
      d->glcx->d_ptr->readback_target_size = QSize();
   }
   return pm;
}

QImage QGLWidget::grabFrameBuffer(bool withAlpha)
{
   makeCurrent();
   QImage res;
   qreal pixelRatio = devicePixelRatioF();
   int w = pixelRatio * width();
   int h = pixelRatio * height();

   if (format().rgba()) {
      res = cs_glRead_frameBuffer(QSize(w, h), format().alpha(), withAlpha);
   }

   res.setDevicePixelRatio(pixelRatio);

   return res;
}

void QGLWidget::glInit()
{
   Q_D(QGLWidget);
   if (!isValid()) {
      return;
   }

   makeCurrent();
   initializeGL();
   d->glcx->setInitialized(true);
}


void QGLWidget::glDraw()
{
   Q_D(QGLWidget);

   if (!isValid()) {
      return;
   }

   makeCurrent();

#ifndef QT_OPENGL_ES
   if (d->glcx->deviceIsPixmap() && !d->glcx->contextHandle()->isOpenGLES()) {
      qgl1_functions()->glDrawBuffer(GL_FRONT);
   }
#endif
   QSize readback_target_size = d->glcx->d_ptr->readback_target_size;
   if (!d->glcx->initialized()) {
      glInit();
      const qreal scaleFactor = (window() && window()->windowHandle()) ?
         window()->windowHandle()->devicePixelRatio() : 1.0;
      int w, h;
      if (readback_target_size.isEmpty()) {
         w = d->glcx->device()->width() * scaleFactor;
         h = d->glcx->device()->height() * scaleFactor;
      } else {
         w = readback_target_size.width();
         h = readback_target_size.height();
      }
      resizeGL(w, h); // New context needs this "resize"
   }
   paintGL();
   if (doubleBuffer() && readback_target_size.isEmpty()) {
      if (d->autoSwap) {
         swapBuffers();
      }
   } else {
      qgl_functions()->glFlush();
   }
}

void QGLWidget::qglColor(const QColor &c) const
{
#if ! defined(QT_OPENGL_ES_2)

#ifdef QT_OPENGL_ES
   qgl_functions()->glColor4f(c.redF(), c.greenF(), c.blueF(), c.alphaF());

#else
   Q_D(const QGLWidget);
   const QGLContext *ctx = QGLContext::currentContext();

   if (ctx && !ctx->contextHandle()->isOpenGLES()) {
      if (ctx->format().rgba()) {
         qgl1_functions()->glColor4f(c.redF(), c.greenF(), c.blueF(), c.alphaF());
      } else if (!d->cmap.isEmpty()) { // QGLColormap in use?
         int i = d->cmap.find(c.rgb());
         if (i < 0) {
            i = d->cmap.findNearest(c.rgb());
         }
         qgl1_functions()->glIndexi(i);
      } else {
         qgl1_functions()->glIndexi(ctx->colorIndex(c));
      }
   }

#endif

#else
   (void) c;

#endif //QT_OPENGL_ES_2
}

void QGLWidget::qglClearColor(const QColor &c) const
{
#ifdef QT_OPENGL_ES
   qgl_functions()->glClearColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());

#else
   Q_D(const QGLWidget);
   const QGLContext *ctx = QGLContext::currentContext();

   if (ctx && !ctx->contextHandle()->isOpenGLES()) {
      if (ctx->format().rgba()) {
         qgl_functions()->glClearColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
      } else if (!d->cmap.isEmpty()) { // QGLColormap in use?
         int i = d->cmap.find(c.rgb());
         if (i < 0) {
            i = d->cmap.findNearest(c.rgb());
         }
         qgl1_functions()->glClearIndex(i);
      } else {
         qgl1_functions()->glClearIndex(ctx->colorIndex(c));
      }
   } else {
      qgl_functions()->glClearColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
   }
#endif
}

QImage QGLWidget::convertToGLFormat(const QImage &img)
{
   QImage res(img.size(), QImage::Format_ARGB32);
   convertToGLFormatHelper(res, img.convertToFormat(QImage::Format_ARGB32), GL_RGBA);
   return res;
}

const QGLColormap &QGLWidget::colormap() const
{
   Q_D(const QGLWidget);
   return d->cmap;
}

void QGLWidget::setColormap(const QGLColormap &c)
{
   (void) c;
}

#ifndef QT_OPENGL_ES

static void qt_save_gl_state()
{
   QOpenGLFunctions *funcs = qgl_functions();
   QOpenGLFunctions_1_1 *gl1funcs = qgl1_functions();
   gl1funcs->glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
   gl1funcs->glPushAttrib(GL_ALL_ATTRIB_BITS);
   gl1funcs->glMatrixMode(GL_TEXTURE);
   gl1funcs->glPushMatrix();
   gl1funcs->glLoadIdentity();
   gl1funcs->glMatrixMode(GL_PROJECTION);
   gl1funcs->glPushMatrix();
   gl1funcs->glMatrixMode(GL_MODELVIEW);
   gl1funcs->glPushMatrix();

   gl1funcs->glShadeModel(GL_FLAT);
   funcs->glDisable(GL_CULL_FACE);
   funcs->glDisable(GL_LIGHTING);
   funcs->glDisable(GL_STENCIL_TEST);
   funcs->glDisable(GL_DEPTH_TEST);
   funcs->glEnable(GL_BLEND);
   funcs->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

static void qt_restore_gl_state()
{
   QOpenGLFunctions_1_1 *gl1funcs = qgl1_functions();
   gl1funcs->glMatrixMode(GL_TEXTURE);
   gl1funcs->glPopMatrix();
   gl1funcs->glMatrixMode(GL_PROJECTION);
   gl1funcs->glPopMatrix();
   gl1funcs->glMatrixMode(GL_MODELVIEW);
   gl1funcs->glPopMatrix();
   gl1funcs->glPopAttrib();
   gl1funcs->glPopClientAttrib();
}

static void qt_gl_draw_text(QPainter *p, int x, int y, const QString &str,
   const QFont &font)
{
   GLfloat color[4];
   qgl_functions()->glGetFloatv(GL_CURRENT_COLOR, &color[0]);

   QColor col;
   col.setRgbF(color[0], color[1], color[2], color[3]);
   QPen old_pen = p->pen();
   QFont old_font = p->font();

   p->setPen(col);
   p->setFont(font);
   p->drawText(x, y, str);

   p->setPen(old_pen);
   p->setFont(old_font);
}

#endif // ! QT_OPENGL_ES


void QGLWidget::renderText(int x, int y, const QString &str, const QFont &font)
{
#ifndef QT_OPENGL_ES
   Q_D(QGLWidget);

   if (! d->glcx->contextHandle()->isOpenGLES()) {

      if (str.isEmpty() || ! isValid()) {
         return;
      }

      QOpenGLFunctions *funcs = qgl_functions();
      GLint view[4];
      bool use_scissor_testing = funcs->glIsEnabled(GL_SCISSOR_TEST);

      if (!use_scissor_testing) {
         funcs->glGetIntegerv(GL_VIEWPORT, &view[0]);
      }
      int width = d->glcx->device()->width();
      int height = d->glcx->device()->height();
      bool auto_swap = autoBufferSwap();

      QPaintEngine *engine = paintEngine();

      qt_save_gl_state();

      // this changes what paintEngine() returns
      QPainter *p;
      bool reuse_painter = false;
      if (engine->isActive()) {
         reuse_painter = true;
         p = engine->painter();

         funcs->glDisable(GL_DEPTH_TEST);
         funcs->glViewport(0, 0, width, height);


      } else {
         setAutoBufferSwap(false);
         // disable glClear() as a result of QPainter::begin()
         d->disable_clear_on_painter_begin = true;
         p = new QPainter(this);
      }

      QRect viewport(view[0], view[1], view[2], view[3]);
      if (!use_scissor_testing && viewport != rect()) {
         // if the user hasn't set a scissor box, we set one that
         // covers the current viewport
         funcs->glScissor(view[0], view[1], view[2], view[3]);
         funcs->glEnable(GL_SCISSOR_TEST);
      } else if (use_scissor_testing) {
         // use the scissor box set by the user
         funcs->glEnable(GL_SCISSOR_TEST);
      }

      qt_gl_draw_text(p, x, y, str, font);

      if (!reuse_painter) {
         p->end();
         delete p;
         setAutoBufferSwap(auto_swap);
         d->disable_clear_on_painter_begin = false;
      }

      qt_restore_gl_state();

      return;
   }

#else
   (void) x;
   (void) y;
   (void) str;
   (void) font;

#endif

   qWarning("QGLWidget::renderText is not supported under OpenGL/ES");
}

void QGLWidget::renderText(double x, double y, double z, const QString &str, const QFont &font)
{
#ifndef QT_OPENGL_ES
   Q_D(QGLWidget);

   if (! d->glcx->contextHandle()->isOpenGLES()) {

      if (str.isEmpty() || !isValid()) {
         return;
      }

      QOpenGLFunctions *funcs = qgl_functions();
      bool auto_swap = autoBufferSwap();

      int width = d->glcx->device()->width();
      int height = d->glcx->device()->height();
      GLdouble model[4 * 4], proj[4 * 4];
      GLint view[4];
      QOpenGLFunctions_1_1 *gl1funcs = qgl1_functions();
      gl1funcs->glGetDoublev(GL_MODELVIEW_MATRIX, &model[0]);
      gl1funcs->glGetDoublev(GL_PROJECTION_MATRIX, &proj[0]);
      funcs->glGetIntegerv(GL_VIEWPORT, &view[0]);
      GLdouble win_x = 0, win_y = 0, win_z = 0;
      qgluProject(x, y, z, &model[0], &proj[0], &view[0],
         &win_x, &win_y, &win_z);
      const int dpr = d->glcx->device()->devicePixelRatioF();
      win_x /= dpr;
      win_y /= dpr;
      win_y = height - win_y; // y is inverted

      QPaintEngine *engine = paintEngine();

      QPainter *p;
      bool reuse_painter = false;
      bool use_depth_testing = funcs->glIsEnabled(GL_DEPTH_TEST);
      bool use_scissor_testing = funcs->glIsEnabled(GL_SCISSOR_TEST);

      qt_save_gl_state();
      if (engine->isActive()) {
         reuse_painter = true;
         p = engine->painter();

      } else {
         setAutoBufferSwap(false);
         // disable glClear() as a result of QPainter::begin()
         d->disable_clear_on_painter_begin = true;
         p = new QPainter(this);
      }

      QRect viewport(view[0], view[1], view[2], view[3]);
      if (!use_scissor_testing && viewport != rect()) {
         funcs->glScissor(view[0], view[1], view[2], view[3]);
         funcs->glEnable(GL_SCISSOR_TEST);
      } else if (use_scissor_testing) {
         funcs->glEnable(GL_SCISSOR_TEST);
      }
      funcs->glViewport(0, 0, width * dpr, height * dpr);
      gl1funcs->glAlphaFunc(GL_GREATER, 0.0);
      funcs->glEnable(GL_ALPHA_TEST);
      if (use_depth_testing) {
         funcs->glEnable(GL_DEPTH_TEST);
      }

      // The only option is the shader-based OpenGL 2 paint engine.
      // Setting fixed pipeline transformations is futile. Instead, pass the
      // extra values directly and let the engine figure the matrices out.
      static_cast<QGL2PaintEngineEx *>(p->paintEngine())->setTranslateZ(-win_z);

      qt_gl_draw_text(p, qRound(win_x), qRound(win_y), str, font);

      static_cast<QGL2PaintEngineEx *>(p->paintEngine())->setTranslateZ(0);

      if (!reuse_painter) {
         p->end();
         delete p;
         setAutoBufferSwap(auto_swap);
         d->disable_clear_on_painter_begin = false;
      }

      qt_restore_gl_state();

      return;
   }

#else
   (void) x;
   (void) y;
   (void) z;
   (void) str;
   (void) font;
#endif

   qWarning("QGLWidget::renderText is not supported under OpenGL/ES");
}

QGLFormat QGLWidget::format() const
{
   Q_D(const QGLWidget);
   return d->glcx->format();
}

QGLContext *QGLWidget::context() const
{
   Q_D(const QGLWidget);
   return d->glcx;
}

bool QGLWidget::doubleBuffer() const
{
   Q_D(const QGLWidget);
   return d->glcx->d_ptr->glFormat.testOption(QGL::DoubleBuffer);
}

void QGLWidget::setAutoBufferSwap(bool on)
{
   Q_D(QGLWidget);
   d->autoSwap = on;
}

bool QGLWidget::autoBufferSwap() const
{
   Q_D(const QGLWidget);
   return d->autoSwap;
}

GLuint QGLWidget::bindTexture(const QImage &image, GLenum target, GLint format)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLWidget);
   return d->glcx->bindTexture(image, target, format, QGLContext::DefaultBindOption);
}

GLuint QGLWidget::bindTexture(const QImage &image, GLenum target, GLint format, QGLContext::BindOptions options)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLWidget);
   return d->glcx->bindTexture(image, target, format, options);
}

GLuint QGLWidget::bindTexture(const QPixmap &pixmap, GLenum target, GLint format)
{
   if (pixmap.isNull()) {
      return 0;
   }

   Q_D(QGLWidget);
   return d->glcx->bindTexture(pixmap, target, format, QGLContext::DefaultBindOption);
}

GLuint QGLWidget::bindTexture(const QPixmap &pixmap, GLenum target, GLint format,
   QGLContext::BindOptions options)
{
   Q_D(QGLWidget);
   return d->glcx->bindTexture(pixmap, target, format, options);
}

GLuint QGLWidget::bindTexture(const QString &fileName)
{
   Q_D(QGLWidget);
   return d->glcx->bindTexture(fileName);
}

void QGLWidget::deleteTexture(GLuint id)
{
   Q_D(QGLWidget);
   d->glcx->deleteTexture(id);
}

void QGLWidget::drawTexture(const QRectF &target, GLuint textureId, GLenum textureTarget)
{
   Q_D(QGLWidget);
   d->glcx->drawTexture(target, textureId, textureTarget);
}

void QGLWidget::drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget)
{
   Q_D(QGLWidget);
   d->glcx->drawTexture(point, textureId, textureTarget);
}

static QGLEngineThreadStorage<QGL2PaintEngineEx> *qt_gl_2_engine()
{
   static QGLEngineThreadStorage<QGL2PaintEngineEx> retval;
   return &retval;
}

Q_OPENGL_EXPORT QPaintEngine *qt_qgl_paint_engine()
{
   return qt_gl_2_engine()->engine();
}

QPaintEngine *QGLWidget::paintEngine() const
{
   return qt_qgl_paint_engine();
}

void QGLWidgetPrivate::init(QGLContext *context, const QGLWidget *shareWidget)
{
   initContext(context, shareWidget);
}

void QGLWidgetPrivate::initContext(QGLContext *context, const QGLWidget *shareWidget)
{
   Q_Q(QGLWidget);

   glDevice.setWidget(q);

   glcx = nullptr;
   autoSwap = true;

   if (context && !context->device()) {
      context->setDevice(q);
   }
   q->setContext(context, shareWidget ? shareWidget->context() : nullptr);

   if (! glcx) {
      glcx = new QGLContext(QGLFormat::defaultFormat(), q);
   }
}

bool QGLWidgetPrivate::renderCxPm(QPixmap *)
{
   return false;
}

void QGLWidgetPrivate::cleanupColormaps()
{
}

static QString *qt_gl_lib_name()
{
   static QString retval;
   return &retval;
}

void qt_set_gl_library_name(const QString &name)
{
   qt_gl_lib_name()->operator=(name);
}

const QString qt_gl_library_name()
{
   if (qt_gl_lib_name()->isEmpty()) {


#if defined(QT_OPENGL_ES_2)
      return QLatin1String("GLESv2");
# else
      return QLatin1String("GL");
# endif


   }
   return *qt_gl_lib_name();
}


void QGLContextGroup::addShare(const QGLContext *context, const QGLContext *share)
{
   Q_ASSERT(context && share);
   if (context->d_ptr->group == share->d_ptr->group) {
      return;
   }

   // Make sure 'context' is not already shared with another group of contexts.
   Q_ASSERT(context->d_ptr->group->m_refs.load() == 1);

   // Free 'context' group resources and make it use the same resources as 'share'.
   QGLContextGroup *group = share->d_ptr->group;
   delete context->d_ptr->group;
   context->d_ptr->group = group;
   group->m_refs.ref();

   // Maintain a list of all the contexts in each group of sharing contexts.
   // The list is empty if the "share" context wasn't sharing already.
   if (group->m_shares.isEmpty()) {
      group->m_shares.append(share);
   }
   group->m_shares.append(context);
}

void QGLContextGroup::removeShare(const QGLContext *context)
{
   // Remove the context from the group.
   QGLContextGroup *group = context->d_ptr->group;
   if (group->m_shares.isEmpty()) {
      return;
   }
   group->m_shares.removeAll(context);

   // Update context group representative.
   Q_ASSERT(group->m_shares.size() != 0);
   if (group->m_context == context) {
      group->m_context = group->m_shares[0];
   }

   // If there is only one context left, then make the list empty.
   if (group->m_shares.size() == 1) {
      group->m_shares.clear();
   }
}

QSize QGLTexture::bindCompressedTexture
(const QString &fileName, const char *format)
{
   QFile file(fileName);
   if (!file.open(QIODevice::ReadOnly)) {
      return QSize();
   }
   QByteArray contents = file.readAll();
   file.close();
   return bindCompressedTexture
      (contents.constData(), contents.size(), format);
}

// PVR header format for container files that store textures compressed
// with the ETC1, PVRTC2, and PVRTC4 encodings.  Format information from the
// PowerVR SDK at http://www.imgtec.com/powervr/insider/powervr-sdk.asp
// "PVRTexTool Reference Manual, version 1.11f".
struct PvrHeader {
   quint32 headerSize;
   quint32 height;
   quint32 width;
   quint32 mipMapCount;
   quint32 flags;
   quint32 dataSize;
   quint32 bitsPerPixel;
   quint32 redMask;
   quint32 greenMask;
   quint32 blueMask;
   quint32 alphaMask;
   quint32 magic;
   quint32 surfaceCount;
};

#define PVR_MAGIC               0x21525650      // "PVR!" in little-endian

#define PVR_FORMAT_MASK         0x000000FF
#define PVR_FORMAT_PVRTC2       0x00000018
#define PVR_FORMAT_PVRTC4       0x00000019
#define PVR_FORMAT_ETC1         0x00000036

#define PVR_HAS_MIPMAPS         0x00000100
#define PVR_TWIDDLED            0x00000200
#define PVR_NORMAL_MAP          0x00000400
#define PVR_BORDER_ADDED        0x00000800
#define PVR_CUBE_MAP            0x00001000
#define PVR_FALSE_COLOR_MIPMAPS 0x00002000
#define PVR_VOLUME_TEXTURE      0x00004000
#define PVR_ALPHA_IN_TEXTURE    0x00008000
#define PVR_VERTICAL_FLIP       0x00010000

#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG     0x8C03
#endif

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES                        0x8D64
#endif

bool QGLTexture::canBindCompressedTexture
(const char *buf, int len, const char *format, bool *hasAlpha)
{
   if constexpr (QSysInfo::ByteOrder != QSysInfo::LittleEndian) {
      // Compressed texture loading only supported on little-endian
      // systems such as x86 and ARM at the moment.
      return false;
   }
   if (!format) {
      // Auto-detect the format from the header.
      if (len >= 4 && !qstrncmp(buf, "DDS ", 4)) {
         *hasAlpha = true;
         return true;
      } else if (len >= 52 && !qstrncmp(buf + 44, "PVR!", 4)) {
         const PvrHeader *pvrHeader =
            reinterpret_cast<const PvrHeader *>(buf);
         *hasAlpha = (pvrHeader->alphaMask != 0);
         return true;
      }
   } else {
      // Validate the format against the header.
      if (!qstricmp(format, "DDS")) {
         if (len >= 4 && !qstrncmp(buf, "DDS ", 4)) {
            *hasAlpha = true;
            return true;
         }
      } else if (!qstricmp(format, "PVR") || !qstricmp(format, "ETC1")) {
         if (len >= 52 && !qstrncmp(buf + 44, "PVR!", 4)) {
            const PvrHeader *pvrHeader =
               reinterpret_cast<const PvrHeader *>(buf);
            *hasAlpha = (pvrHeader->alphaMask != 0);
            return true;
         }
      }
   }
   return false;
}

#define ctx QGLContext::currentContext()

QSize QGLTexture::bindCompressedTexture(const char *buf, int len, const char *format)
{
   if constexpr (QSysInfo::ByteOrder != QSysInfo::LittleEndian) {
      // Compressed texture loading only supported on little-endian
      // systems such as x86 and ARM at the moment.
      return QSize();
   }

   if (!format) {
      // Auto-detect the format from the header.
      if (len >= 4 && !qstrncmp(buf, "DDS ", 4)) {
         return bindCompressedTextureDDS(buf, len);
      } else if (len >= 52 && !qstrncmp(buf + 44, "PVR!", 4)) {
         return bindCompressedTexturePVR(buf, len);
      }
   } else {
      // Validate the format against the header.
      if (!qstricmp(format, "DDS")) {
         if (len >= 4 && !qstrncmp(buf, "DDS ", 4)) {
            return bindCompressedTextureDDS(buf, len);
         }
      } else if (!qstricmp(format, "PVR") || !qstricmp(format, "ETC1")) {
         if (len >= 52 && !qstrncmp(buf + 44, "PVR!", 4)) {
            return bindCompressedTexturePVR(buf, len);
         }
      }
   }
   return QSize();
}

QSize QGLTexture::bindCompressedTextureDDS(const char *buf, int len)
{
   // only support 2D texture loading at present
   if (target != GL_TEXTURE_2D) {
      return QSize();
   }

   // Bail out if the necessary extension is not present.
   if (!qgl_extensions()->hasOpenGLExtension(QOpenGLExtensions::DDSTextureCompression)) {
      qWarning("QGLContext::bindTexture(): DDS texture compression is not supported.");
      return QSize();
   }

   const DDSFormat *ddsHeader = reinterpret_cast<const DDSFormat *>(buf + 4);
   if (!ddsHeader->dwLinearSize) {
      qWarning("QGLContext::bindTexture(): DDS image size is not valid.");
      return QSize();
   }

   int blockSize = 16;
   GLenum format;

   switch (ddsHeader->ddsPixelFormat.dwFourCC) {
      case FOURCC_DXT1:
         format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
         blockSize = 8;
         break;
      case FOURCC_DXT3:
         format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
         break;
      case FOURCC_DXT5:
         format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
         break;
      default:
         qWarning("QGLContext::bindTexture(): DDS image format not supported.");
         return QSize();
   }

   const GLubyte *pixels =
      reinterpret_cast<const GLubyte *>(buf + ddsHeader->dwSize + 4);

   QOpenGLFunctions *funcs = qgl_functions();
   funcs->glGenTextures(1, &id);
   funcs->glBindTexture(GL_TEXTURE_2D, id);
   funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   int size;
   int offset = 0;
   int available = len - int(ddsHeader->dwSize + 4);
   int w = ddsHeader->dwWidth;
   int h = ddsHeader->dwHeight;

   // load mip-maps
   for (int i = 0; i < (int) ddsHeader->dwMipMapCount; ++i) {
      if (w == 0) {
         w = 1;
      }
      if (h == 0) {
         h = 1;
      }

      size = ((w + 3) / 4) * ((h + 3) / 4) * blockSize;
      if (size > available) {
         break;
      }

      qgl_extensions()->glCompressedTexImage2D(GL_TEXTURE_2D, i, format, w, h, 0,
         size, pixels + offset);
      offset += size;
      available -= size;

      // half size for each mip-map level
      w = w / 2;
      h = h / 2;
   }

   // DDS images are not inverted.
   options &= ~QGLContext::InvertedYBindOption;

   return QSize(ddsHeader->dwWidth, ddsHeader->dwHeight);
}

QSize QGLTexture::bindCompressedTexturePVR(const char *buf, int len)
{
   // We only support 2D texture loading at present.  Cube maps later.
   if (target != GL_TEXTURE_2D) {
      return QSize();
   }

   // Determine which texture format we will be loading.
   const PvrHeader *pvrHeader = reinterpret_cast<const PvrHeader *>(buf);
   GLenum textureFormat;
   quint32 minWidth, minHeight;

   switch (pvrHeader->flags & PVR_FORMAT_MASK) {
      case PVR_FORMAT_PVRTC2:
         if (pvrHeader->alphaMask) {
            textureFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
         } else {
            textureFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
         }
         minWidth = 16;
         minHeight = 8;
         break;

      case PVR_FORMAT_PVRTC4:
         if (pvrHeader->alphaMask) {
            textureFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
         } else {
            textureFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
         }
         minWidth = 8;
         minHeight = 8;
         break;

      case PVR_FORMAT_ETC1:
         textureFormat = GL_ETC1_RGB8_OES;
         minWidth = 4;
         minHeight = 4;
         break;

      default:
         qWarning("QGLContext::bindTexture(): PVR image format 0x%x not supported.", int(pvrHeader->flags & PVR_FORMAT_MASK));
         return QSize();
   }

   // Bail out if the necessary extension is not present.
   if (textureFormat == GL_ETC1_RGB8_OES) {

      if (! qgl_extensions()->hasOpenGLExtension(QOpenGLExtensions::ETC1TextureCompression)) {
         qWarning("QGLContext::bindTexture(): ETC1 texture compression is not supported.");
         return QSize();
      }
   } else {

      if (!qgl_extensions()->hasOpenGLExtension(QOpenGLExtensions::PVRTCTextureCompression)) {
         qWarning("QGLContext::bindTexture(): PVRTC texture compression is not supported.");
         return QSize();
      }
   }

   // Boundary check on the buffer size.
   quint32 bufferSize = pvrHeader->headerSize + pvrHeader->dataSize;
   if (bufferSize > quint32(len)) {
      qWarning("QGLContext::bindTexture(): PVR image size is not valid.");
      return QSize();
   }

   // Create the texture.
   QOpenGLFunctions *funcs = qgl_functions();
   funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   funcs->glGenTextures(1, &id);
   funcs->glBindTexture(GL_TEXTURE_2D, id);

   if (pvrHeader->mipMapCount) {
      if ((options & QGLContext::LinearFilteringBindOption) != 0) {
         funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      } else {
         funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      }
   } else if ((options & QGLContext::LinearFilteringBindOption) != 0) {
      funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   } else {
      funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   }

   // Load the compressed mipmap levels.
   const GLubyte *buffer =
      reinterpret_cast<const GLubyte *>(buf + pvrHeader->headerSize);
   bufferSize = pvrHeader->dataSize;
   quint32 level = 0;
   quint32 width = pvrHeader->width;
   quint32 height = pvrHeader->height;
   while (bufferSize > 0 && level <= pvrHeader->mipMapCount) {
      quint32 size =
         (qMax(width, minWidth) * qMax(height, minHeight) *
            pvrHeader->bitsPerPixel) / 8;
      if (size > bufferSize) {
         break;
      }

      qgl_extensions()->glCompressedTexImage2D(GL_TEXTURE_2D, GLint(level), textureFormat,
         GLsizei(width), GLsizei(height), 0,
         GLsizei(size), buffer);
      width /= 2;
      height /= 2;
      buffer += size;
      ++level;
   }

   // Restore the default pixel alignment for later texture uploads.
   funcs->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

   // Set the invert flag for the texture.  The "vertical flip"
   // flag in PVR is the opposite sense to our sense of inversion.
   if ((pvrHeader->flags & PVR_VERTICAL_FLIP) != 0) {
      options &= ~QGLContext::InvertedYBindOption;
   } else {
      options |= QGLContext::InvertedYBindOption;
   }

   return QSize(pvrHeader->width, pvrHeader->height);
}

QString cs_glGetString(GLenum data)
{
   const GLubyte *begin = glGetString(data);

   if (begin == nullptr) {
      return QString();

   } else {
      const GLubyte *end = begin;

      while (*end) {
         ++end;
      }

      return QString(begin, end);
   }
}

using qt_glGetStringi = const GLubyte * (QOPENGLF_APIENTRYP)(GLenum, GLuint);

QString cs_glGetStringI(GLenum data, GLuint index)
{
   static qt_glGetStringi funcPtr = nullptr;

   if (funcPtr == nullptr) {
      const QGLContext *tmp = QGLContext::currentContext();

      if (tmp == nullptr) {
         return QString();
      }

      funcPtr = (qt_glGetStringi)ctx->getProcAddress("glGetStringi");
   }

   const GLubyte *begin = funcPtr(data, index);

   if (begin == nullptr) {
      return QString();

   } else {
      const GLubyte *end = begin;

      while (*end) {
         ++end;
      }

      return QString(begin, end);
   }
}

#undef ctx
