/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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
   QGLDefaultExtensions() : extensions(0) {
      QGLTemporaryContext tempContext;
      Q_ASSERT(QOpenGLContext::currentContext());
      QOpenGLExtensions *ext = qgl_extensions();
      Q_ASSERT(ext);
      extensions = ext->openGLExtensions();
      features = ext->openGLFeatures();
   }

   QOpenGLFunctions::OpenGLFeatures features;
   QOpenGLExtensions::OpenGLExtensions extensions;
};

Q_GLOBAL_STATIC(QGLDefaultExtensions, qtDefaultExtensions)

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
   return 0;
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

Q_GLOBAL_STATIC(QGLFormat, qgl_default_format)

class QGLDefaultOverlayFormat: public QGLFormat
{
 public:
   inline QGLDefaultOverlayFormat() {
      setOption(QGL::FormatOption(0xffff << 16)); // turn off all options
      setOption(QGL::DirectRendering);
      setPlane(1);
   }
};

Q_GLOBAL_STATIC(QGLDefaultOverlayFormat, defaultOverlayFormatInstance)
Q_GLOBAL_STATIC(QGLSignalProxy, theSignalProxy)

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
   out[0] =
      M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
   out[1] =
      M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
   out[2] =
      M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
   out[3] =
      M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

static inline GLint qgluProject(GLdouble objx, GLdouble objy, GLdouble objz,
   const GLdouble model[16], const GLdouble proj[16],
   const GLint viewport[4],
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

/*!
    \internal
*/
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
   QOpenGLContextPrivate *guiGlContextPrivate =
      guiGlContext ? QOpenGLContextPrivate::get(guiGlContext) : 0;

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

   QOpenGLContextPrivate *guiGlContextPrivate =
      guiGlContext ? QOpenGLContextPrivate::get(guiGlContext) : 0;

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

/*!
    \since 4.2

    Returns the currently set swap interval. -1 is returned if setting
    the swap interval isn't supported in the system GL implementation.
*/
int QGLFormat::swapInterval() const
{
   return d->swapInterval;
}

/*!
    \fn bool QGLFormat::hasOverlay() const

    Returns true if overlay plane is enabled; otherwise returns false.

    Overlay is disabled by default.

    \sa setOverlay()
*/

/*!
    If \a enable is true enables an overlay plane; otherwise disables
    the overlay plane.

    Enabling the overlay plane will cause QGLWidget to create an
    additional context in an overlay plane. See the QGLWidget
    documentation for further information.

    \sa hasOverlay()
*/

void QGLFormat::setOverlay(bool enable)
{
   setOption(enable ? QGL::HasOverlay : QGL::NoOverlay);
}

/*!
    Returns the plane of this format. The default for normal formats
    is 0, which means the normal plane. The default for overlay
    formats is 1, which is the first overlay plane.

    \sa setPlane(), defaultOverlayFormat()
*/
int QGLFormat::plane() const
{
   return d->pln;
}

/*!
    Sets the requested plane to \a plane. 0 is the normal plane, 1 is
    the first overlay plane, 2 is the second overlay plane, etc.; -1,
    -2, etc. are underlay planes.

    Note that in contrast to other format specifications, the plane
    specifications will be matched exactly. This means that if you
    specify a plane that the underlying OpenGL system cannot provide,
    an \link QGLWidget::isValid() invalid\endlink QGLWidget will be
    created.

    \sa plane()
*/
void QGLFormat::setPlane(int plane)
{
   detach();
   d->pln = plane;
}

/*!
    Sets the format option to \a opt.

    \sa testOption()
*/

void QGLFormat::setOption(QGL::FormatOptions opt)
{
   detach();
   if (opt & 0xffff) {
      d->opts |= opt;
   } else {
      d->opts &= ~(opt >> 16);
   }
}



/*!
    Returns true if format option \a opt is set; otherwise returns false.

    \sa setOption()
*/

bool QGLFormat::testOption(QGL::FormatOptions opt) const
{
   if (opt & 0xffff) {
      return (d->opts & opt) != 0;
   } else {
      return (d->opts & (opt >> 16)) == 0;
   }
}

/*!
    Set the minimum depth buffer size to \a size.

    \sa depthBufferSize(), setDepth(), depth()
*/
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

/*!
    Returns the depth buffer size.

    \sa depth(), setDepth(), setDepthBufferSize()
*/
int QGLFormat::depthBufferSize() const
{
   return d->depthSize;
}

/*!
    \since 4.2

    Set the preferred red buffer size to \a size.

    \sa setGreenBufferSize(), setBlueBufferSize(), setAlphaBufferSize()
*/
void QGLFormat::setRedBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setRedBufferSize: Cannot set negative red buffer size %d", size);
      return;
   }
   d->redSize = size;
}

/*!
    \since 4.2

    Returns the red buffer size.

    \sa setRedBufferSize()
*/
int QGLFormat::redBufferSize() const
{
   return d->redSize;
}

/*!
    \since 4.2

    Set the preferred green buffer size to \a size.

    \sa setRedBufferSize(), setBlueBufferSize(), setAlphaBufferSize()
*/
void QGLFormat::setGreenBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setGreenBufferSize: Cannot set negative green buffer size %d", size);
      return;
   }
   d->greenSize = size;
}

/*!
    \since 4.2

    Returns the green buffer size.

    \sa setGreenBufferSize()
*/
int QGLFormat::greenBufferSize() const
{
   return d->greenSize;
}

/*!
    \since 4.2

    Set the preferred blue buffer size to \a size.

    \sa setRedBufferSize(), setGreenBufferSize(), setAlphaBufferSize()
*/
void QGLFormat::setBlueBufferSize(int size)
{
   detach();
   if (size < 0) {
      qWarning("QGLFormat::setBlueBufferSize: Cannot set negative blue buffer size %d", size);
      return;
   }
   d->blueSize = size;
}

/*!
    \since 4.2

    Returns the blue buffer size.

    \sa setBlueBufferSize()
*/
int QGLFormat::blueBufferSize() const
{
   return d->blueSize;
}

/*!
    Set the preferred alpha buffer size to \a size.
    This function implicitly enables the alpha channel.

    \sa setRedBufferSize(), setGreenBufferSize(), alphaBufferSize()
*/
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

/*!
    Returns the alpha buffer size.

    \sa alpha(), setAlpha(), setAlphaBufferSize()
*/
int QGLFormat::alphaBufferSize() const
{
   return d->alphaSize;
}

/*!
    Set the preferred accumulation buffer size, where \a size is the
    bit depth for each RGBA component.

    \sa accum(), setAccum(), accumBufferSize()
*/
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

/*!
    Returns the accumulation buffer size.

    \sa setAccumBufferSize(), accum(), setAccum()
*/
int QGLFormat::accumBufferSize() const
{
   return d->accumSize;
}

/*!
    Set the preferred stencil buffer size to \a size.

    \sa stencilBufferSize(), setStencil(), stencil()
*/
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

/*!
    Returns the stencil buffer size.

    \sa stencil(), setStencil(), setStencilBufferSize()
*/
int QGLFormat::stencilBufferSize() const
{
   return d->stencilSize;
}

/*!
    \since 4.7

    Set the OpenGL version to the \a major and \a minor numbers. If a
    context compatible with the requested OpenGL version cannot be
    created, a context compatible with version 1.x is created instead.

    \sa majorVersion(), minorVersion()
*/
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

/*!
    \since 4.7

    Returns the OpenGL major version.

    \sa setVersion(), minorVersion()
*/
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
   QGLTemporaryContext *tmpContext = 0;

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

/*!
    Returns false if all the options of the two QGLFormat objects
    \a a and \a b are equal; otherwise returns true.

    \relates QGLFormat
*/

bool operator!=(const QGLFormat &a, const QGLFormat &b)
{
   return !(a == b);
}

struct QGLContextGroupList {
   QGLContextGroupList()
      : m_mutex(QMutex::Recursive) {
   }
   void append(QGLContextGroup *group) {
      QMutexLocker locker(&m_mutex);
      m_list.append(group);
   }

   void remove(QGLContextGroup *group) {
      QMutexLocker locker(&m_mutex);
      m_list.removeOne(group);
   }

   QList<QGLContextGroup *> m_list;
   QMutex m_mutex;
};

Q_GLOBAL_STATIC(QGLContextGroupList, qt_context_groups)

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
   if (!ctx) {
      return 0;
   }

   QList<const QGLContext *> shares
   (QGLContextPrivate::contextGroup(ctx)->shares());

   if (shares.size() >= 2) {
      return (ctx == shares.at(0)) ? shares.at(1) : shares.at(0);
   } else {
      return 0;
   }
}

QGLContextPrivate::QGLContextPrivate(QGLContext *context)
   : internal_context(false)
   , q_ptr(context)
   , texture_destroyer(0)
   , functions(0)
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

   guiGlContext = 0;
   ownContext = false;

   fbo = 0;
   crWin = false;
   initDone = false;
   sharing = false;
   max_texture_size = -1;
   version_flags_cached = false;
   version_flags = QGLFormat::OpenGL_Version_None;

   current_fbo = 0;
   default_fbo = 0;
   active_engine = 0;
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

QGLContext *QGLContext::currentCtx = 0;
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
#if !defined(QT_OPENGL_ES)
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
#endif // QT_OPENGL_ES
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

/*
   Read back the contents of the currently bound framebuffer, used in
   QGLWidget::grabFrameBuffer(), QGLPixelbuffer::toImage() and
   QGLFramebufferObject::toImage()
*/

static void convertFromGLImage(QImage &img, int w, int h, bool alpha_format, bool include_alpha)
{
   Q_ASSERT(!img.isNull());

   if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
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
   if (!QOpenGLContext::currentContext()->isOpenGLES()) {
      qgl1_functions()->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
   }
#endif
   convertFromGLImage(img, w, h, alpha_format, include_alpha);
   return img;
}

Q_GLOBAL_STATIC(QGLTextureCache, qt_gl_texture_cache)

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
   QMutexLocker groupLocker(&qt_context_groups()->m_mutex);
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

/*
  a hook that removes textures from the cache when a pixmap/image
  is deref'ed
*/
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

/*!
    Constructs an OpenGL context with the given \a format which
    specifies several display options for the context.

    If the underlying OpenGL/Window system cannot satisfy all the
    features requested in \a format, the nearest subset of features
    will be used. After creation, the format() method will return the
    actual format obtained.

    Note that after a QGLContext object has been constructed, \l
    create() must be called explicitly to create the actual OpenGL
    context. The context will be \l {isValid()}{invalid} if it was not
    possible to obtain a GL context at all.

    \sa format(), isValid()
*/
QGLContext::QGLContext(const QGLFormat &format)
   : d_ptr(new QGLContextPrivate(this))
{
   Q_D(QGLContext);
   d->init(0, format);
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
   d->init(0, QGLFormat::fromSurfaceFormat(context->format()));
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
      return 0;
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
      if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
         return ((src_pixel << 24) & 0xff000000)
            | ((src_pixel >> 24) & 0x000000ff)
            | ((src_pixel << 8) & 0x00ff0000)
            | ((src_pixel >> 8) & 0x0000ff00);
      } else {
         return src_pixel;
      }
   } else {  // GL_RGBA
      if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
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
         if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
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
         if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
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
         texture = 0;
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

// #define QGL_BIND_TEXTURE_DEBUG

#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367
#endif

// map from Qt's ARGB endianness-dependent format to GL's big-endian RGBA layout
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

#ifdef QGL_BIND_TEXTURE_DEBUG
   printf("QGLContextPrivate::bindTexture(), imageSize=(%d,%d), internalFormat =0x%x, options=%x, key=%llx\n",
      image.width(), image.height(), internalFormat, int(options), key);
   QTime time;
   time.start();
#endif

#ifndef QT_NO_DEBUG
   // Reset the gl error stack...git
   while (funcs->glGetError() != GL_NO_ERROR) ;
#endif

   // Scale the pixmap if needed. GL textures needs to have the
   // dimensions 2^n+2(border) x 2^m+2(border), unless we're using GL
   // 2.0 or use the GL_TEXTURE_RECTANGLE texture target
   int tx_w = qNextPowerOfTwo(image.width() - 1);
   int tx_h = qNextPowerOfTwo(image.height() - 1);

   QImage img = image;

   if (!qgl_extensions()->hasOpenGLFeature(QOpenGLFunctions::NPOTTextures)
      && !(QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Version_2_0)
      && (target == GL_TEXTURE_2D && (tx_w != image.width() || tx_h != image.height()))) {
      img = img.scaled(tx_w, tx_h);

#ifdef QGL_BIND_TEXTURE_DEBUG
      printf(" - upscaled to %dx%d (%d ms)\n", tx_w, tx_h, time.elapsed());

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

#ifdef QGL_BIND_TEXTURE_DEBUG
      printf(" - generating mipmaps (%d ms)\n", time.elapsed());
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
#ifdef QGL_BIND_TEXTURE_DEBUG
            printf(" - converted ARGB32 -> ARGB32_Premultiplied (%d ms) \n", time.elapsed());
#endif
         }
         break;
      case QImage::Format_ARGB32_Premultiplied:
         if (!premul) {
            img = img.convertToFormat(target_format = QImage::Format_ARGB32);
#ifdef QGL_BIND_TEXTURE_DEBUG
            printf(" - converted ARGB32_Premultiplied -> ARGB32 (%d ms)\n", time.elapsed());
#endif
         }
         break;

      case QImage::Format_RGBA8888:
         if (premul) {
            img = img.convertToFormat(target_format = QImage::Format_RGBA8888_Premultiplied);
#ifdef QGL_BIND_TEXTURE_DEBUG
            printf(" - converted RGBA8888 -> RGBA8888_Premultiplied (%d ms) \n", time.elapsed());
#endif
         }
         break;

      case QImage::Format_RGBA8888_Premultiplied:
         if (!premul) {
            img = img.convertToFormat(target_format = QImage::Format_RGBA8888);
#ifdef QGL_BIND_TEXTURE_DEBUG
            printf(" - converted RGBA8888_Premultiplied -> RGBA8888 (%d ms) \n", time.elapsed());
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
            img = img.convertToFormat(premul
                  ? QImage::Format_ARGB32_Premultiplied
                  : QImage::Format_ARGB32);
#ifdef QGL_BIND_TEXTURE_DEBUG
            printf(" - converted to 32-bit alpha format (%d ms)\n", time.elapsed());
#endif
         } else {
            img = img.convertToFormat(QImage::Format_RGB32);
#ifdef QGL_BIND_TEXTURE_DEBUG
            printf(" - converted to 32-bit (%d ms)\n", time.elapsed());
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
#ifdef QGL_BIND_TEXTURE_DEBUG
      printf(" - flipped bits over y (%d ms)\n", time.elapsed());
#endif
   }

   if (needsbyteswap) {
      // The only case where we end up with a depth different from
      // 32 in the switch above is for the RGB16 case, where we set
      // the format to GL_RGB
      Q_ASSERT(img.depth() == 32);
      qgl_byteSwapImage(img, pixel_type);

#ifdef QGL_BIND_TEXTURE_DEBUG
      printf(" - did byte swapping (%d ms)\n", time.elapsed());
#endif
   }

   if (ctx->isOpenGLES()) {
      // OpenGL/ES requires that the internal and external formats be
      // identical.
      internalFormat = externalFormat;
   }

#ifdef QGL_BIND_TEXTURE_DEBUG
   printf(" - uploading, image.format=%d, externalFormat=0x%x, internalFormat=0x%x, pixel_type=0x%x\n",
      img.format(), externalFormat, internalFormat, pixel_type);
#endif

   const QImage &constRef = img; // to avoid detach in bits()...

   funcs->glTexImage2D(target, 0, internalFormat, img.width(), img.height(), 0, externalFormat,
      pixel_type, constRef.bits());

   if (genMipmap && ctx->isOpenGLES()) {
      q->functions()->glGenerateMipmap(target);
   }

#ifndef QT_NO_DEBUG
   GLenum error = funcs->glGetError();
   if (error != GL_NO_ERROR) {
      qWarning(" - texture upload failed, error code 0x%x, enum: %d (%x)\n", error, target, target);
   }
#endif

#ifdef QGL_BIND_TEXTURE_DEBUG
   static int totalUploadTime = 0;
   totalUploadTime += time.elapsed();
   printf(" - upload done in %d ms, (accumulated: %d ms)\n", time.elapsed(), totalUploadTime);
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
   if (texture && texture->target == target
      && (texture->context == q || QGLContext::areSharing(q, texture->context))) {
      return texture;
   }
   return 0;
}

// internal
QGLTexture *QGLContextPrivate::bindTexture(const QPixmap &pixmap, GLenum target, GLint format, QGLContext::BindOptions options)
{
   Q_Q(QGLContext);

   const qint64 key = pixmap.cacheKey();
   QGLTexture *texture = textureCacheLookup(key, target);

   if (texture) {
      if (pixmap.paintingActive()) {
         // A QPainter is active on the pixmap - take the safe route and replace the texture.
         q->deleteTexture(texture->id);
         texture = 0;
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
         paintEngine->setPaintDevice(0);
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
      funcs->glTexImage2D(proxy, 0, GL_RGBA, next, next, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
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

         funcs->glTexImage2D(proxy, 0, GL_RGBA, next, next, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
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

/*!
  Generates and binds a 2D GL texture to the current context, based
  on \a image. The generated texture id is returned and can be used in
  later \c glBindTexture() calls.

  \overload
*/
GLuint QGLContext::bindTexture(const QImage &image, GLenum target, GLint format)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(image, target, format, DefaultBindOption);
   return texture->id;
}

/*!
    \since 4.6

    Generates and binds a 2D GL texture to the current context, based
    on \a image. The generated texture id is returned and can be used
    in later \c glBindTexture() calls.

    The \a target parameter specifies the texture target. The default
    target is \c GL_TEXTURE_2D.

    The \a format parameter sets the internal format for the
    texture. The default format is \c GL_RGBA.

    The binding \a options are a set of options used to decide how to
    bind the texture to the context.

    The texture that is generated is cached, so multiple calls to
    bindTexture() with the same QImage will return the same texture
    id.

    Note that we assume default values for the glPixelStore() and
    glPixelTransfer() parameters.

    \sa deleteTexture()
*/
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

/*!
  \overload
  \since 4.6

  Generates and binds a 2D GL texture to the current context, based
  on \a pixmap.
*/
GLuint QGLContext::bindTexture(const QPixmap &pixmap, GLenum target, GLint format, BindOptions options)
{
   if (pixmap.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(pixmap, target, format, options);
   return texture->id;
}

/*!
    Removes the texture identified by \a id from the texture cache,
    and calls glDeleteTextures() to delete the texture from the
    context.

    \sa bindTexture()
*/
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

#if !defined(QT_OPENGL_ES_2)

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

/*!
    \since 4.4

    This function supports the following use cases:

    \list
    \i On OpenGL and OpenGL ES 1.x it draws the given texture, \a textureId,
    to the given target rectangle, \a target, in OpenGL model space. The
    \a textureTarget should be a 2D texture target.
    \i On OpenGL and OpenGL ES 2.x, if a painter is active, not inside a
    beginNativePainting / endNativePainting block, and uses the
    engine with type QPaintEngine::OpenGL2, the function will draw the given
    texture, \a textureId, to the given target rectangle, \a target,
    respecting the current painter state. This will let you draw a texture
    with the clip, transform, render hints, and composition mode set by the
    painter. Note that the texture target needs to be GL_TEXTURE_2D for this
    use case, and that this is the only supported use case under OpenGL ES 2.x.
    \endlist

*/
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


/*!
    \since 4.4

    This function supports the following use cases:

    \list
    \i By default it draws the given texture, \a textureId,
    at the given \a point in OpenGL model space. The
    \a textureTarget should be a 2D texture target.
    \i If a painter is active, not inside a
    beginNativePainting / endNativePainting block, and uses the
    engine with type QPaintEngine::OpenGL2, the function will draw the given
    texture, \a textureId, at the given \a point,
    respecting the current painter state. This will let you draw a texture
    with the clip, transform, render hints, and composition mode set by the
    painter. Note that the texture target needs to be GL_TEXTURE_2D for this
    use case.
    \endlist

    \note This function is not supported under any version of OpenGL ES.
*/
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


/*!
    This function sets the limit for the texture cache to \a size,
    expressed in kilobytes.

    By default, the cache limit is approximately 64 MB.

    \sa textureCacheLimit()
*/
void QGLContext::setTextureCacheLimit(int size)
{
   QGLTextureCache::instance()->setMaxCost(size);
}

/*!
    Returns the current texture cache limit in kilobytes.

    \sa setTextureCacheLimit()
*/
int QGLContext::textureCacheLimit()
{
   return QGLTextureCache::instance()->maxCost();
}


/*!
    \fn QGLFormat QGLContext::format() const

    Returns the frame buffer format that was obtained (this may be a
    subset of what was requested).

    \sa requestedFormat()
*/

/*!
    \fn QGLFormat QGLContext::requestedFormat() const

    Returns the frame buffer format that was originally requested in
    the constructor or setFormat().

    \sa format()
*/

/*!
    Sets a \a format for this context. The context is \link reset()
    reset\endlink.

    Call create() to create a new GL context that tries to match the
    new format.

    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 7

    \sa format(), reset(), create()
*/

void QGLContext::setFormat(const QGLFormat &format)
{
   Q_D(QGLContext);
   reset();
   d->glFormat = d->reqFormat = format;
}

/*!
    \internal
*/
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

/*!
    \fn bool QGLContext::deviceIsPixmap() const

    Returns true if the paint device of this context is a pixmap;
    otherwise returns false.
*/

/*!
    \fn bool QGLContext::windowCreated() const

    Returns true if a window has been created for this context;
    otherwise returns false.

    \sa setWindowCreated()
*/

/*!
    \fn void QGLContext::setWindowCreated(bool on)

    If \a on is true the context has had a window created for it. If
    \a on is false no window has been created for the context.

    \sa windowCreated()
*/

/*!
    \fn uint QGLContext::colorIndex(const QColor& c) const

    \internal

    Returns a colormap index for the color c, in ColorIndex mode. Used
    by qglColor() and qglClearColor().
*/

uint QGLContext::colorIndex(const QColor &) const
{
   return 0;
}

/*!
    \fn bool QGLContext::initialized() const

    Returns true if this context has been initialized, i.e. if
    QGLWidget::initializeGL() has been performed on it; otherwise
    returns false.

    \sa setInitialized()
*/

/*!
    \fn void QGLContext::setInitialized(bool on)

    If \a on is true the context has been initialized, i.e.
    QGLContext::setInitialized() has been called on it. If \a on is
    false the context has not been initialized.

    \sa initialized()
*/

/*!
    \fn const QGLContext* QGLContext::currentContext()

    Returns the current context, i.e. the context to which any OpenGL
    commands will currently be directed. Returns 0 if no context is
    current.

    \sa makeCurrent()
*/

/*!
    \fn QColor QGLContext::overlayTransparentColor() const

    If this context is a valid context in an overlay plane, returns
    the plane's transparent color. Otherwise returns an
    \{QColor::isValid()}{invalid} color.

    The returned QColor object will generally work as expected only
    when passed as the argument to QGLWidget::qglColor() or
    QGLWidget::qglClearColor(). Under certain circumstances it can
    also be used to draw transparent graphics with a QPainter.
*/

QColor QGLContext::overlayTransparentColor() const
{
   return QColor(); // Invalid color
}

/*!
    Creates the GL context. Returns true if it was successful in
    creating a valid GL rendering context on the paint device
    specified in the constructor; otherwise returns false (i.e. the
    context is invalid).

    After successful creation, format() returns the set of features of
    the created GL rendering context.

    If \a shareContext points to a valid QGLContext, this method will
    try to establish OpenGL display list and texture object sharing
    between this context and the \a shareContext. Note that this may
    fail if the two contexts have different \l {format()} {formats}.
    Use isSharing() to see if sharing is in effect.

    \warning Implementation note: initialization of C++ class
    members usually takes place in the class constructor. QGLContext
    is an exception because it must be simple to customize. The
    virtual functions chooseContext() (and chooseVisual() for X11) can
    be reimplemented in a subclass to select a particular context. The
    problem is that virtual functions are not properly called during
    construction (even though this is correct C++) because C++
    constructs class hierarchies from the bottom up. For this reason
    we need a create() function.

    \sa chooseContext(), format(), isValid()
*/

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

   return 0;
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
      // Unlike in Qt 4, the only possible target is a widget backed by an OpenGL-based
      // QWindow. Pixmaps in particular are not supported anymore as paint devices since
      // starting from Qt 5 QPixmap is raster-backed on almost all platforms.
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
      QOpenGLContext *shareGlContext = shareContext ? shareContext->d_func()->guiGlContext : 0;
      d->guiGlContext = new QOpenGLContext;
      d->guiGlContext->setFormat(winFormat);
      d->guiGlContext->setShareContext(shareGlContext);
      d->valid = d->guiGlContext->create();

      if (d->valid) {
         d->guiGlContext->setQGLContextHandle(this, 0);
      }

      d->glFormat = QGLFormat::fromSurfaceFormat(d->guiGlContext->format());
      d->setupSharing();
   }


   return d->valid;
}


void QGLContext::reset()
{
   Q_D(QGLContext);
   if (!d->valid) {
      return;
   }
   d->cleanup();

   d->crWin = false;
   d->sharing = false;
   d->valid = false;
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
         d->guiGlContext->setQGLContextHandle(0, 0);
      }
      d->guiGlContext = 0;
   }
   d->ownContext = false;
}


void QGLContext::makeCurrent()
{
   Q_D(QGLContext);
   if (!d->paintDevice || d->paintDevice->devType() != QInternal::Widget) {
      return;
   }

   QWidget *widget = static_cast<QWidget *>(d->paintDevice);
   if (!widget->windowHandle()) {
      return;
   }

   if (d->guiGlContext->makeCurrent(widget->windowHandle())) {
      if (!d->workaroundsCached) {
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


QGLWidget::QGLWidget(QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags f)
   : QWidget(*(new QGLWidgetPrivate), parent, f | Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(new QGLContext(QGLFormat::defaultFormat(), this), shareWidget);
}

QGLWidget::QGLWidget(QGLWidgetPrivate &dd, const QGLFormat &format, QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags f)
   : QWidget(dd, parent, f | Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(new QGLContext(format, this), shareWidget);

}

QGLWidget::QGLWidget(const QGLFormat &format, QWidget *parent, const QGLWidget *shareWidget,
   Qt::WindowFlags f)
   : QWidget(*(new QGLWidgetPrivate), parent, f | Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(new QGLContext(format, this), shareWidget);
}


QGLWidget::QGLWidget(QGLContext *context, QWidget *parent, const QGLWidget *shareWidget,
   Qt::WindowFlags f)
   : QWidget(*(new QGLWidgetPrivate), parent, f | Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(context, shareWidget);
}

/*!
    Destroys the widget.
*/

QGLWidget::~QGLWidget()
{
   Q_D(QGLWidget);

   delete d->glcx;
   d->glcx = 0;
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

/*!
    \fn void QGLWidget::doneCurrent()

    Makes no GL context the current context. Normally, you do not need
    to call this function; QGLContext calls it as necessary. However,
    it may be useful in multithreaded environments.
*/

void QGLWidget::doneCurrent()
{
   Q_D(QGLWidget);
   d->glcx->doneCurrent();
}

/*!
    \fn void QGLWidget::swapBuffers()

    Swaps the screen contents with an off-screen buffer. This only
    works if the widget's format specifies double buffer mode.

    Normally, there is no need to explicitly call this function
    because it is done automatically after each widget repaint, i.e.
    each time after paintGL() has been executed.

    \sa doubleBuffer(), setAutoBufferSwap(), QGLFormat::setDoubleBuffer()
*/

void QGLWidget::swapBuffers()
{
   Q_D(QGLWidget);
   d->glcx->swapBuffers();
}


/*!
    \fn const QGLContext* QGLWidget::overlayContext() const

    Returns the overlay context of this widget, or 0 if this widget
    has no overlay.

    \sa context()
*/
const QGLContext *QGLWidget::overlayContext() const
{
   return 0;
}


void QGLWidget::makeOverlayCurrent()
{
}


void QGLWidget::setFormat(const QGLFormat &format)
{
   setContext(new QGLContext(format, this));
}




/*!
    \fn const QGLContext *QGLWidget::context() const

    Returns the context of this widget.

    It is possible that the context is not valid (see isValid()), for
    example, if the underlying hardware does not support the format
    attributes that were requested.
*/

/*
  \fn void QGLWidget::setContext(QGLContext *context,
                                 const QGLContext* shareContext,
                                 bool deleteOldContext)
  \obsolete

  Sets a new context for this widget. The QGLContext \a context must
  be created using \e new. QGLWidget will delete \a context when
  another context is set or when the widget is destroyed.

  If \a context is invalid, QGLContext::create() is performed on
  it. The initializeGL() function will then be executed for the new
  context before the first resizeGL() or paintGL().

  If \a context is invalid, this method will try to keep display list
  and texture object sharing in effect, or (if \a shareContext points
  to a valid context) start display list and texture object sharing
  with that context, but sharing might be impossible if the two
  contexts have different \l {format()} {formats}. Use isSharing() to
  see whether sharing is in effect.

  If \a deleteOldContext is true (the default), the existing context
  will be deleted. You may use false here if you have kept a pointer
  to the old context (as returned by context()), and want to restore
  that context later.

  \sa context(), isSharing()
*/

void QGLWidget::setContext(QGLContext *context,
   const QGLContext *shareContext,
   bool deleteOldContext)
{
   Q_D(QGLWidget);
   if (context == 0) {
      qWarning("QGLWidget::setContext: Cannot set null context");
      return;
   }

   if (context->device() == 0) { // a context may refere to more than 1 window.
      context->setDevice(this);   //but its better to point to 1 of them than none of them.
   }

   QGLContext *oldcx = d->glcx;
   d->glcx = context;

   if (!d->glcx->isValid()) {
      d->glcx->create(shareContext ? shareContext : oldcx);
   }
   if (deleteOldContext) {
      delete oldcx;
   }
}
/*!
    \fn void QGLWidget::updateGL()

    Updates the widget by calling glDraw().
*/

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


/*!
    This virtual function is called whenever the widget needs to be
    painted. Reimplement it in a subclass.

    There is no need to call makeCurrent() because this has already
    been done when this function is called.
*/

void QGLWidget::paintGL()
{
   qgl_functions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


/*!
    \fn void QGLWidget::resizeGL(int width , int height)

    This virtual function is called whenever the widget has been
    resized. The new size is passed in \a width and \a height.
    Reimplement it in a subclass.

    There is no need to call makeCurrent() because this has already
    been done when this function is called.
*/

void QGLWidget::resizeGL(int, int)
{
}



/*!
    This virtual function is used in the same manner as initializeGL()
    except that it operates on the widget's overlay context instead of
    the widget's main context. This means that initializeOverlayGL()
    is called once before the first call to paintOverlayGL() or
    resizeOverlayGL(). Reimplement it in a subclass.

    This function should set up any required OpenGL context rendering
    flags, defining display lists, etc. for the overlay context.

    There is no need to call makeOverlayCurrent() because this has
    already been done when this function is called.
*/

void QGLWidget::initializeOverlayGL()
{
}


/*!
    This virtual function is used in the same manner as paintGL()
    except that it operates on the widget's overlay context instead of
    the widget's main context. This means that paintOverlayGL() is
    called whenever the widget's overlay needs to be painted.
    Reimplement it in a subclass.

    There is no need to call makeOverlayCurrent() because this has
    already been done when this function is called.
*/

void QGLWidget::paintOverlayGL()
{
}


/*!
    \fn void QGLWidget::resizeOverlayGL(int width , int height)

    This virtual function is used in the same manner as paintGL()
    except that it operates on the widget's overlay context instead of
    the widget's main context. This means that resizeOverlayGL() is
    called whenever the widget has been resized. The new size is
    passed in \a width and \a height. Reimplement it in a subclass.

    There is no need to call makeOverlayCurrent() because this has
    already been done when this function is called.
*/

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

/*!
    Convenience function for specifying a drawing color to OpenGL.
    Calls glColor4 (in RGBA mode) or glIndex (in color-index mode)
    with the color \a c. Applies to this widgets GL context.

    \note This function is not supported on OpenGL/ES 2.0 systems.

    \sa qglClearColor(), QGLContext::currentContext(), QColor
*/

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
   if (!d->glcx->contextHandle()->isOpenGLES()) {
      Q_D(QGLWidget);
      if (str.isEmpty() || !isValid()) {
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
   if (!d->glcx->contextHandle()->isOpenGLES()) {
      Q_D(QGLWidget);
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

      // The only option in Qt 5 is the shader-based OpenGL 2 paint engine.
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

/*!
  \overload
  \since 4.6

  The binding \a options are a set of options used to decide how to
  bind the texture to the context.
 */
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

Q_GLOBAL_STATIC(QGLEngineThreadStorage<QGL2PaintEngineEx>, qt_gl_2_engine)

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

/*
  This is the shared initialization for all platforms. Called from QGLWidgetPrivate::init()
*/

void QGLWidgetPrivate::initContext(QGLContext *context, const QGLWidget *shareWidget)
{
   Q_Q(QGLWidget);

   glDevice.setWidget(q);

   glcx = 0;
   autoSwap = true;

   if (context && !context->device()) {
      context->setDevice(q);
   }
   q->setContext(context, shareWidget ? shareWidget->context() : 0);

   if (!glcx) {
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

Q_GLOBAL_STATIC(QString, qt_gl_lib_name)

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
   if (QSysInfo::ByteOrder != QSysInfo::LittleEndian) {
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
   if (QSysInfo::ByteOrder != QSysInfo::LittleEndian) {
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

