/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qapplication.h"
#include "qplatformdefs.h"
#include <qgl.h>
#include <qdebug.h>

#if defined(Q_WS_X11)
#include "qt_x11_p.h"
#include "qpixmap_x11_p.h"

#define INT32 dummy_INT32
#define INT8 dummy_INT8

#ifdef QT_NO_EGL
# include <GL/glx.h>
#endif

#undef INT32
#undef INT8

#include "qx11info_x11.h"

#elif defined(Q_OS_MAC)
# include <qt_mac_p.h>
#endif

#include <qdatetime.h>
#include <stdlib.h>

#include "qpixmap.h"
#include "qimage.h"
#include "qgl_p.h"

#if !defined(QT_OPENGL_ES_1)
#include "gl2paintengineex/qpaintengineex_opengl2_p.h"
#include <qwindowsurface_gl_p.h>
#endif

#ifndef QT_OPENGL_ES_2
#include <qpaintengine_opengl_p.h>
#endif

#ifdef Q_WS_QWS
#include <qglwindowsurface_qws_p.h>
#endif

#ifdef Q_WS_QPA
#include <QtGui/QPlatformGLContext>
#endif

#include <qglpixelbuffer.h>
#include <qglframebufferobject.h>

#include <qimage_p.h>
#include <qpixmapdata_p.h>
#include <qpixmapdata_gl_p.h>
#include <qglpixelbuffer_p.h>
#include <qimagepixmapcleanuphooks_p.h>
#include <qcolormap.h>
#include <qfile.h>
#include <qlibrary.h>
#include <qmutex.h>

#if defined(QT_OPENGL_ES) && !defined(QT_NO_EGL)
#include <EGL/egl.h>
#endif

using qt_glGetStringi = const GLubyte * (APIENTRY *)(GLenum, GLuint);

#ifndef GL_NUM_EXTENSIONS
#define GL_NUM_EXTENSIONS 0x821D
#endif

typedef void (*_qt_pixmap_cleanup_hook_64)(qint64);
typedef void (*_qt_image_cleanup_hook_64)(qint64);

extern Q_GUI_EXPORT _qt_pixmap_cleanup_hook_64 qt_pixmap_cleanup_hook_64;
extern Q_GUI_EXPORT _qt_image_cleanup_hook_64 qt_image_cleanup_hook_64;

Q_GLOBAL_STATIC(QGLTextureCache, qt_gl_texture_cache)

#if defined(Q_WS_X11) || defined(Q_OS_MAC) || defined(Q_WS_QWS) || defined(Q_WS_QPA)
QGLExtensionFuncs QGLContextPrivate::qt_extensionFuncs;
#endif

#ifdef Q_WS_X11
extern const QX11Info *qt_x11Info(const QPaintDevice *pd);
#endif

struct QGLThreadContext {

   ~QGLThreadContext() {
      if (context) {
         context->doneCurrent();
      }
   }

   QGLContext *context;
};

#ifndef Q_WS_QPA
static QThreadStorage<QGLThreadContext *> qgl_context_storage;
#endif

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
   if (proxy && proxy->thread() != qApp->thread()) {
      if (proxy->thread() == QThread::currentThread()) {
         proxy->moveToThread(qApp->thread());
      }
   }
   return proxy;
}


class QGLEngineSelector
{
 public:
   QGLEngineSelector() : engineType(QPaintEngine::MaxUser) {
   }

   void setPreferredPaintEngine(QPaintEngine::Type type) {
      if (type == QPaintEngine::OpenGL || type == QPaintEngine::OpenGL2) {
         engineType = type;
      }
   }

   QPaintEngine::Type preferredPaintEngine() {
#ifdef Q_OS_MAC
      // The ATI X1600 driver for Mac OS X does not support return
      // values from functions in GLSL. Since working around this in
      // the GL2 engine would require a big, ugly rewrite, we're
      // falling back to the GL 1 engine..
      static bool mac_x1600_check_done = false;
      if (!mac_x1600_check_done) {
         QGLTemporaryContext *tmp = 0;
         if (!QGLContext::currentContext()) {
            tmp = new QGLTemporaryContext();
         }
         if (strstr((char *) glGetString(GL_RENDERER), "X1600")) {
            engineType = QPaintEngine::OpenGL;
         }
         if (tmp) {
            delete tmp;
         }
         mac_x1600_check_done = true;
      }
#endif
      if (engineType == QPaintEngine::MaxUser) {
         // No user-set engine - use the defaults
#if defined(QT_OPENGL_ES_2)
         engineType = QPaintEngine::OpenGL2;
#else
         // We can't do this in the constructor for this object because it
         // needs to be called *before* the QApplication constructor.
         // Also check for the FragmentShader extension in conjunction with
         // the 2.0 version flag, to cover the case where we export the display
         // from an old GL 1.1 server to a GL 2.x client. In that case we can't
         // use GL 2.0.
         if ((QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0)
               && (QGLExtensions::glExtensions() & QGLExtensions::FragmentShader)
               && qgetenv("QT_GL_USE_OPENGL1ENGINE").isEmpty()) {
            engineType = QPaintEngine::OpenGL2;
         } else {
            engineType = QPaintEngine::OpenGL;
         }
#endif
      }
      return engineType;
   }

 private:
   QPaintEngine::Type engineType;
};

Q_GLOBAL_STATIC(QGLEngineSelector, qgl_engine_selector)


bool qt_gl_preferGL2Engine()
{
   return qgl_engine_selector()->preferredPaintEngine() == QPaintEngine::OpenGL2;
}


/*!
    \namespace QGL
    \inmodule QtOpenGL

    \brief The QGL namespace specifies miscellaneous identifiers used
    in the Qt OpenGL module.

    \ingroup painting-3D
*/

/*!
    \enum QGL::FormatOption

    This enum specifies the format options that can be used to configure an OpenGL
    context. These are set using QGLFormat::setOption().

    \value DoubleBuffer      Specifies the use of double buffering.
    \value DepthBuffer       Enables the use of a depth buffer.
    \value Rgba              Specifies that the context should use RGBA as its pixel format.
    \value AlphaChannel      Enables the use of an alpha channel.
    \value AccumBuffer       Enables the use of an accumulation buffer.
    \value StencilBuffer     Enables the use of a stencil buffer.
    \value StereoBuffers     Enables the use of a stereo buffers for use with visualization hardware.
    \value DirectRendering   Specifies that the context is used for direct rendering to a display.
    \value HasOverlay        Enables the use of an overlay.
    \value SampleBuffers     Enables the use of sample buffers.
    \value DeprecatedFunctions      Enables the use of deprecated functionality for OpenGL 3.x
                                    contexts. A context with deprecated functionality enabled is
                                    called a full context in the OpenGL specification.
    \value SingleBuffer      Specifies the use of a single buffer, as opposed to double buffers.
    \value NoDepthBuffer     Disables the use of a depth buffer.
    \value ColorIndex        Specifies that the context should use a color index as its pixel format.
    \value NoAlphaChannel    Disables the use of an alpha channel.
    \value NoAccumBuffer     Disables the use of an accumulation buffer.
    \value NoStencilBuffer   Disables the use of a stencil buffer.
    \value NoStereoBuffers   Disables the use of stereo buffers.
    \value IndirectRendering Specifies that the context is used for indirect rendering to a buffer.
    \value NoOverlay         Disables the use of an overlay.
    \value NoSampleBuffers   Disables the use of sample buffers.
    \value NoDeprecatedFunctions    Disables the use of deprecated functionality for OpenGL 3.x
                                    contexts. A context with deprecated functionality disabled is
                                    called a forward compatible context in the OpenGL specification.

    \sa {Sample Buffers Example}
*/

/*!
   \fn void QGL::setPreferredPaintEngine(QPaintEngine::Type engineType)

   \since 4.6

   Sets the preferred OpenGL paint engine that is used to draw onto
   QGLWidget, QGLPixelBuffer and QGLFramebufferObject targets with QPainter
   in Qt.

   The \a engineType parameter specifies which of the GL engines to
   use. Only \c QPaintEngine::OpenGL and \c QPaintEngine::OpenGL2 are
   valid parameters to this function. All other values are ignored.

   By default, the \c QPaintEngine::OpenGL2 engine is used if GL/GLES
   version 2.0 is available, otherwise \c QPaintEngine::OpenGL is
   used.

   \warning This function must be called before the QApplication
   constructor is called.
*/
void QGL::setPreferredPaintEngine(QPaintEngine::Type engineType)
{
   qgl_engine_selector()->setPreferredPaintEngine(engineType);
}


/*****************************************************************************
  QGLFormat implementation
 *****************************************************************************/


/*!
    \class QGLFormat
    \brief The QGLFormat class specifies the display format of an OpenGL
    rendering context.

    \ingroup painting-3D

    A display format has several characteristics:
    \list
    \i \link setDoubleBuffer() Double or single buffering.\endlink
    \i \link setDepth() Depth buffer.\endlink
    \i \link setRgba() RGBA or color index mode.\endlink
    \i \link setAlpha() Alpha channel.\endlink
    \i \link setAccum() Accumulation buffer.\endlink
    \i \link setStencil() Stencil buffer.\endlink
    \i \link setStereo() Stereo buffers.\endlink
    \i \link setDirectRendering() Direct rendering.\endlink
    \i \link setOverlay() Presence of an overlay.\endlink
    \i \link setPlane() Plane of an overlay.\endlink
    \i \link setSampleBuffers() Multisample buffers.\endlink
    \endlist

    You can also specify preferred bit depths for the color buffer,
    depth buffer, alpha buffer, accumulation buffer and the stencil
    buffer with the functions: setRedBufferSize(), setGreenBufferSize(),
    setBlueBufferSize(), setDepthBufferSize(), setAlphaBufferSize(),
    setAccumBufferSize() and setStencilBufferSize().

    Note that even if you specify that you prefer a 32 bit depth
    buffer (e.g. with setDepthBufferSize(32)), the format that is
    chosen may not have a 32 bit depth buffer, even if there is a
    format available with a 32 bit depth buffer. The main reason for
    this is how the system dependant picking algorithms work on the
    different platforms, and some format options may have higher
    precedence than others.

    You create and tell a QGLFormat object what rendering options you
    want from an OpenGL rendering context.

    OpenGL drivers or accelerated hardware may or may not support
    advanced features such as alpha channel or stereographic viewing.
    If you request some features that the driver/hardware does not
    provide when you create a QGLWidget, you will get a rendering
    context with the nearest subset of features.

    There are different ways to define the display characteristics of
    a rendering context. One is to create a QGLFormat and make it the
    default for the entire application:
    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 0

    Or you can specify the desired format when creating an object of
    your QGLWidget subclass:
    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 1

    After the widget has been created, you can find out which of the
    requested features the system was able to provide:
    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 2

    \legalese
        OpenGL is a trademark of Silicon Graphics, Inc. in the
        United States and other countries.
    \endlegalese

    \sa QGLContext, QGLWidget
*/

#ifndef QT_OPENGL_ES

static inline void transform_point(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#if defined(Q_CC_GNU) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Warray-bounds"
#endif
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
#if defined(Q_CC_GNU) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
# pragma GCC diagnostic pop
#endif
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

/*!
    Constructs a QGLFormat object with the following default settings:
    \list
    \i \link setDoubleBuffer() Double buffer:\endlink Enabled.
    \i \link setDepth() Depth buffer:\endlink Enabled.
    \i \link setRgba() RGBA:\endlink Enabled (i.e., color index disabled).
    \i \link setAlpha() Alpha channel:\endlink Disabled.
    \i \link setAccum() Accumulator buffer:\endlink Disabled.
    \i \link setStencil() Stencil buffer:\endlink Enabled.
    \i \link setStereo() Stereo:\endlink Disabled.
    \i \link setDirectRendering() Direct rendering:\endlink Enabled.
    \i \link setOverlay() Overlay:\endlink Disabled.
    \i \link setPlane() Plane:\endlink 0 (i.e., normal plane).
    \i \link setSampleBuffers() Multisample buffers:\endlink Disabled.
    \endlist
*/

QGLFormat::QGLFormat()
{
   d = new QGLFormatPrivate;
}


/*!
    Creates a QGLFormat object that is a copy of the current
    defaultFormat().

    If \a options is not 0, the default format is modified by the
    specified format options. The \a options parameter should be
    QGL::FormatOption values OR'ed together.

    This constructor makes it easy to specify a certain desired format
    in classes derived from QGLWidget, for example:
    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 3

    Note that there are QGL::FormatOption values to turn format settings
    both on and off, e.g. QGL::DepthBuffer and QGL::NoDepthBuffer,
    QGL::DirectRendering and QGL::IndirectRendering, etc.

    The \a plane parameter defaults to 0 and is the plane which this
    format should be associated with. Not all OpenGL implementations
    supports overlay/underlay rendering planes.

    \sa defaultFormat(), setOption(), setPlane()
*/

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

/*!
    Constructs a copy of \a other.
*/

QGLFormat::QGLFormat(const QGLFormat &other)
{
   d = other.d;
   d->ref.ref();
}

/*!
    Assigns \a other to this object.
*/

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

/*!
    Destroys the QGLFormat.
*/
QGLFormat::~QGLFormat()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

/*!
    \fn bool QGLFormat::doubleBuffer() const

    Returns true if double buffering is enabled; otherwise returns
    false. Double buffering is enabled by default.

    \sa setDoubleBuffer()
*/

/*!
    If \a enable is true sets double buffering; otherwise sets single
    buffering.

    Double buffering is enabled by default.

    Double buffering is a technique where graphics are rendered on an
    off-screen buffer and not directly to the screen. When the drawing
    has been completed, the program calls a swapBuffers() function to
    exchange the screen contents with the buffer. The result is
    flicker-free drawing and often better performance.

    Note that single buffered contexts are currently not supported
    with EGL.

    \sa doubleBuffer(), QGLContext::swapBuffers(),
    QGLWidget::swapBuffers()
*/

void QGLFormat::setDoubleBuffer(bool enable)
{
   setOption(enable ? QGL::DoubleBuffer : QGL::SingleBuffer);
}


/*!
    \fn bool QGLFormat::depth() const

    Returns true if the depth buffer is enabled; otherwise returns
    false. The depth buffer is enabled by default.

    \sa setDepth(), setDepthBufferSize()
*/

/*!
    If \a enable is true enables the depth buffer; otherwise disables
    the depth buffer.

    The depth buffer is enabled by default.

    The purpose of a depth buffer (or Z-buffering) is to remove hidden
    surfaces. Pixels are assigned Z values based on the distance to
    the viewer. A pixel with a high Z value is closer to the viewer
    than a pixel with a low Z value. This information is used to
    decide whether to draw a pixel or not.

    \sa depth(), setDepthBufferSize()
*/

void QGLFormat::setDepth(bool enable)
{
   setOption(enable ? QGL::DepthBuffer : QGL::NoDepthBuffer);
}


/*!
    \fn bool QGLFormat::rgba() const

    Returns true if RGBA color mode is set. Returns false if color
    index mode is set. The default color mode is RGBA.

    \sa setRgba()
*/

/*!
    If \a enable is true sets RGBA mode. If \a enable is false sets
    color index mode.

    The default color mode is RGBA.

    RGBA is the preferred mode for most OpenGL applications. In RGBA
    color mode you specify colors as red + green + blue + alpha
    quadruplets.

    In color index mode you specify an index into a color lookup
    table.

    \sa rgba()
*/

void QGLFormat::setRgba(bool enable)
{
   setOption(enable ? QGL::Rgba : QGL::ColorIndex);
}


/*!
    \fn bool QGLFormat::alpha() const

    Returns true if the alpha buffer in the framebuffer is enabled;
    otherwise returns false. The alpha buffer is disabled by default.

    \sa setAlpha(), setAlphaBufferSize()
*/

/*!
    If \a enable is true enables the alpha buffer; otherwise disables
    the alpha buffer.

    The alpha buffer is disabled by default.

    The alpha buffer is typically used for implementing transparency
    or translucency. The A in RGBA specifies the transparency of a
    pixel.

    \sa alpha(), setAlphaBufferSize()
*/

void QGLFormat::setAlpha(bool enable)
{
   setOption(enable ? QGL::AlphaChannel : QGL::NoAlphaChannel);
}


/*!
    \fn bool QGLFormat::accum() const

    Returns true if the accumulation buffer is enabled; otherwise
    returns false. The accumulation buffer is disabled by default.

    \sa setAccum(), setAccumBufferSize()
*/

/*!
    If \a enable is true enables the accumulation buffer; otherwise
    disables the accumulation buffer.

    The accumulation buffer is disabled by default.

    The accumulation buffer is used to create blur effects and
    multiple exposures.

    \sa accum(), setAccumBufferSize()
*/

void QGLFormat::setAccum(bool enable)
{
   setOption(enable ? QGL::AccumBuffer : QGL::NoAccumBuffer);
}


/*!
    \fn bool QGLFormat::stencil() const

    Returns true if the stencil buffer is enabled; otherwise returns
    false. The stencil buffer is enabled by default.

    \sa setStencil(), setStencilBufferSize()
*/

/*!
    If \a enable is true enables the stencil buffer; otherwise
    disables the stencil buffer.

    The stencil buffer is enabled by default.

    The stencil buffer masks certain parts of the drawing area so that
    masked parts are not drawn on.

    \sa stencil(), setStencilBufferSize()
*/

void QGLFormat::setStencil(bool enable)
{
   setOption(enable ? QGL::StencilBuffer : QGL::NoStencilBuffer);
}


/*!
    \fn bool QGLFormat::stereo() const

    Returns true if stereo buffering is enabled; otherwise returns
    false. Stereo buffering is disabled by default.

    \sa setStereo()
*/

/*!
    If \a enable is true enables stereo buffering; otherwise disables
    stereo buffering.

    Stereo buffering is disabled by default.

    Stereo buffering provides extra color buffers to generate left-eye
    and right-eye images.

    \sa stereo()
*/

void QGLFormat::setStereo(bool enable)
{
   setOption(enable ? QGL::StereoBuffers : QGL::NoStereoBuffers);
}


/*!
    \fn bool QGLFormat::directRendering() const

    Returns true if direct rendering is enabled; otherwise returns
    false.

    Direct rendering is enabled by default.

    \sa setDirectRendering()
*/

/*!
    If \a enable is true enables direct rendering; otherwise disables
    direct rendering.

    Direct rendering is enabled by default.

    Enabling this option will make OpenGL bypass the underlying window
    system and render directly from hardware to the screen, if this is
    supported by the system.

    \sa directRendering()
*/

void QGLFormat::setDirectRendering(bool enable)
{
   setOption(enable ? QGL::DirectRendering : QGL::IndirectRendering);
}

/*!
    \fn bool QGLFormat::sampleBuffers() const

    Returns true if multisample buffer support is enabled; otherwise
    returns false.

    The multisample buffer is disabled by default.

    \sa setSampleBuffers()
*/

/*!
    If \a enable is true, a GL context with multisample buffer support
    is picked; otherwise ignored.

    \sa sampleBuffers(), setSamples(), samples()
*/
void QGLFormat::setSampleBuffers(bool enable)
{
   setOption(enable ? QGL::SampleBuffers : QGL::NoSampleBuffers);
}

/*!
    Returns the number of samples per pixel when multisampling is
    enabled. By default, the highest number of samples that is
    available is used.

    \sa setSampleBuffers(), sampleBuffers(), setSamples()
*/
int QGLFormat::samples() const
{
   return d->numSamples;
}

/*!
    Set the preferred number of samples per pixel when multisampling
    is enabled to \a numSamples. By default, the highest number of
    samples available is used.

    \sa setSampleBuffers(), sampleBuffers(), samples()
*/
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

/*!
    \since 4.2

    Set the preferred swap interval. This can be used to sync the GL
    drawing into a system window to the vertical refresh of the screen.
    Setting an \a interval value of 0 will turn the vertical refresh syncing
    off, any value higher than 0 will turn the vertical syncing on.

    Under Windows and under X11, where the \c{WGL_EXT_swap_control}
    and \c{GLX_SGI_video_sync} extensions are used, the \a interval
    parameter can be used to set the minimum number of video frames
    that are displayed before a buffer swap will occur. In effect,
    setting the \a interval to 10, means there will be 10 vertical
    retraces between every buffer swap.

    Under Windows the \c{WGL_EXT_swap_control} extension has to be present,
    and under X11 the \c{GLX_SGI_video_sync} extension has to be present.
*/
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


QGLFormat::OpenGLVersionFlags qOpenGLVersionFlagsFromString(const QString &versionString)
{
   QGLFormat::OpenGLVersionFlags versionFlags = QGLFormat::OpenGL_Version_None;

   if (versionString.startsWith(QLatin1String("OpenGL ES"))) {
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
            case '4':
               versionFlags |= QGLFormat::OpenGL_Version_1_4;
            case '3':
               versionFlags |= QGLFormat::OpenGL_Version_1_3;
            case '2':
               versionFlags |= QGLFormat::OpenGL_Version_1_2;
            case '1':
               versionFlags |= QGLFormat::OpenGL_Version_1_1;
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
            case '2':
               versionFlags |= QGLFormat::OpenGL_Version_3_2;
            case '1':
               versionFlags |= QGLFormat::OpenGL_Version_3_1;
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
                         QGLFormat::OpenGL_Version_4_0;
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


/*!
    Returns the default QGLFormat for the application. All QGLWidget
    objects that are created use this format unless another format is
    specified, e.g. when they are constructed.

    If no special default format has been set using
    setDefaultFormat(), the default format is the same as that created
    with QGLFormat().

    \sa setDefaultFormat()
*/

QGLFormat QGLFormat::defaultFormat()
{
   return *qgl_default_format();
}

/*!
    Sets a new default QGLFormat for the application to \a f. For
    example, to set single buffering as the default instead of double
    buffering, your main() might contain code like this:
    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 4

    \sa defaultFormat()
*/

void QGLFormat::setDefaultFormat(const QGLFormat &f)
{
   *qgl_default_format() = f;
}


/*!
    Returns the default QGLFormat for overlay contexts.

    The default overlay format is:
    \list
    \i \link setDoubleBuffer() Double buffer:\endlink Disabled.
    \i \link setDepth() Depth buffer:\endlink Disabled.
    \i \link setRgba() RGBA:\endlink Disabled (i.e., color index enabled).
    \i \link setAlpha() Alpha channel:\endlink Disabled.
    \i \link setAccum() Accumulator buffer:\endlink Disabled.
    \i \link setStencil() Stencil buffer:\endlink Disabled.
    \i \link setStereo() Stereo:\endlink Disabled.
    \i \link setDirectRendering() Direct rendering:\endlink Enabled.
    \i \link setOverlay() Overlay:\endlink Disabled.
    \i \link setSampleBuffers() Multisample buffers:\endlink Disabled.
    \i \link setPlane() Plane:\endlink 1 (i.e., first overlay plane).
    \endlist

    \sa setDefaultFormat()
*/

QGLFormat QGLFormat::defaultOverlayFormat()
{
   return *defaultOverlayFormatInstance();
}

/*!
    Sets a new default QGLFormat for overlay contexts to \a f. This
    format is used whenever a QGLWidget is created with a format that
    hasOverlay() enabled.

    For example, to get a double buffered overlay context (if
    available), use code like this:

    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 5

    As usual, you can find out after widget creation whether the
    underlying OpenGL system was able to provide the requested
    specification:

    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 6

    \sa defaultOverlayFormat()
*/

void QGLFormat::setDefaultOverlayFormat(const QGLFormat &f)
{
   QGLFormat *defaultFormat = defaultOverlayFormatInstance();
   *defaultFormat = f;
   // Make sure the user doesn't request that the overlays themselves
   // have overlays, since it is unlikely that the system supports
   // infinitely many planes...
   defaultFormat->setOverlay(false);
}


/*!
    Returns true if all the options of the two QGLFormat objects
    \a a and \a b are equal; otherwise returns false.

    \relates QGLFormat
*/

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

   return dbg.space();
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

/*****************************************************************************
  QGLContext implementation
 *****************************************************************************/

QGLContextGroup::QGLContextGroup(const QGLContext *context)
   : m_context(context), m_guards(0), m_refs(1)
{
   qt_context_groups()->append(this);
}

QGLContextGroup::~QGLContextGroup()
{
   // Clear any remaining QGLSharedResourceGuard objects on the group.
   QGLSharedResourceGuard *guard = m_guards;
   while (guard != 0) {
      guard->m_group = 0;
      guard->m_id = 0;
      guard = guard->m_next;
   }
   qt_context_groups()->remove(this);
}

void QGLContextGroup::addGuard(QGLSharedResourceGuard *guard)
{
   if (m_guards) {
      m_guards->m_prev = guard;
   }
   guard->m_next = m_guards;
   guard->m_prev = 0;
   m_guards = guard;
}

void QGLContextGroup::removeGuard(QGLSharedResourceGuard *guard)
{
   if (guard->m_next) {
      guard->m_next->m_prev = guard->m_prev;
   }
   if (guard->m_prev) {
      guard->m_prev->m_next = guard->m_next;
   } else {
      m_guards = guard->m_next;
   }
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
{
   group = new QGLContextGroup(context);
   texture_destroyer = new QGLTextureDestroyer;
   texture_destroyer->moveToThread(qApp->thread());
}

QGLContextPrivate::~QGLContextPrivate()
{
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

#if defined(Q_WS_X11)
   pbuf = 0;
   gpm = 0;
   vi = 0;
   screen = QX11Info::appScreen();
#endif

#if defined(Q_OS_WIN)
   dc = 0;
   win = 0;
   threadId = 0;
   pixelFormatId = 0;
   cmap = 0;
   hbitmap = 0;
   hbitmap_hdc = 0;
#endif

#if defined(Q_OS_MAC)
   vi = 0;
#endif

#if defined(Q_WS_QPA)
   platformContext = 0;
#endif

#if !defined(QT_NO_EGL)
   ownsEglContext = false;
   eglContext = 0;
   eglSurface = EGL_NO_SURFACE;
#endif
   fbo = 0;
   crWin = false;
   initDone = false;
   sharing = false;
   max_texture_size = -1;
   version_flags_cached = false;
   version_flags = QGLFormat::OpenGL_Version_None;
   extension_flags_cached = false;
   extension_flags = 0;
   current_fbo = 0;
   default_fbo = 0;
   active_engine = 0;
   workaround_needsFullClearOnEveryFrame = false;
   workaround_brokenFBOReadBack = false;
   workaround_brokenTexSubImage = false;
   workaroundsCached = false;

   workaround_brokenTextureFromPixmap = false;
   workaround_brokenTextureFromPixmap_init = false;

   workaround_brokenScissor = false;
   workaround_brokenAlphaTexSubImage = false;
   workaround_brokenAlphaTexSubImage_init = false;

   for (int i = 0; i < QT_GL_VERTEX_ARRAY_TRACKED_COUNT; ++i) {
      vertexAttributeArraysEnabledState[i] = false;
   }
}

QGLContext *QGLContext::currentCtx = 0;

/*
   Read back the contents of the currently bound framebuffer, used in
   QGLWidget::grabFrameBuffer(), QGLPixelbuffer::toImage() and
   QGLFramebufferObject::toImage()
*/

static void convertFromGLImage(QImage &img, int w, int h, bool alpha_format, bool include_alpha)
{
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

QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format, bool include_alpha)
{
   QImage img(size, (alpha_format && include_alpha) ? QImage::Format_ARGB32_Premultiplied
              : QImage::Format_RGB32);
   int w = size.width();
   int h = size.height();
   glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
   convertFromGLImage(img, w, h, alpha_format, include_alpha);
   return img;
}

QImage qt_gl_read_texture(const QSize &size, bool alpha_format, bool include_alpha)
{
   QImage img(size, alpha_format ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32);
   int w = size.width();
   int h = size.height();
#if !defined(QT_OPENGL_ES_2) && !defined(QT_OPENGL_ES_1)
   //### glGetTexImage not in GL ES 2.0, need to do something else here!
   glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
#endif
   convertFromGLImage(img, w, h, alpha_format, include_alpha);
   return img;
}

// returns the highest number closest to v, which is a power of 2
// NB! assumes 32 bit ints
int qt_next_power_of_two(int v)
{
   v--;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   ++v;
   return v;
}

QGLTextureCache::QGLTextureCache()
   : m_cache(64 * 1024) // cache ~64 MB worth of textures - this is not accurate though
{
   QImagePixmapCleanupHooks::instance()->addPixmapDataModificationHook(cleanupTexturesForPixampData);
   QImagePixmapCleanupHooks::instance()->addPixmapDataDestructionHook(cleanupBeforePixmapDestruction);
   QImagePixmapCleanupHooks::instance()->addImageHook(cleanupTexturesForCacheKey);
}

QGLTextureCache::~QGLTextureCache()
{
   QImagePixmapCleanupHooks::instance()->removePixmapDataModificationHook(cleanupTexturesForPixampData);
   QImagePixmapCleanupHooks::instance()->removePixmapDataDestructionHook(cleanupBeforePixmapDestruction);
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


void QGLTextureCache::cleanupTexturesForPixampData(QPixmapData *pmd)
{
   cleanupTexturesForCacheKey(pmd->cacheKey());
}

void QGLTextureCache::cleanupBeforePixmapDestruction(QPixmapData *pmd)
{
   // Remove any bound textures first:
   cleanupTexturesForPixampData(pmd);

#if defined(Q_WS_X11)
   if (pmd->classId() == QPixmapData::X11Class) {
      Q_ASSERT(pmd->ref.load() == 0); // Make sure reference counting isn't broken
      QGLContextPrivate::destroyGlSurfaceForPixmap(pmd);
   }
#endif
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
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif

#ifndef GL_GENERATE_MIPMAP_SGIS
#define GL_GENERATE_MIPMAP_SGIS       0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS  0x8192
#endif

/*!
    \class QGLContext
    \brief The QGLContext class encapsulates an OpenGL rendering context.

    \ingroup painting-3D

    An OpenGL rendering context is a complete set of OpenGL state
    variables. The rendering context's \l {QGL::FormatOption} {format}
    is set in the constructor, but it can also be set later with
    setFormat(). The format options that are actually set are returned
    by format(); the options you asked for are returned by
    requestedFormat(). Note that after a QGLContext object has been
    constructed, the actual OpenGL context must be created by
    explicitly calling the \link create() create()\endlink
    function. The makeCurrent() function makes this context the
    current rendering context. You can make \e no context current
    using doneCurrent(). The reset() function will reset the context
    and make it invalid.

    You can examine properties of the context with, e.g. isValid(),
    isSharing(), initialized(), windowCreated() and
    overlayTransparentColor().

    If you're using double buffering you can swap the screen contents
    with the off-screen buffer using swapBuffers().

    Please note that QGLContext is not \l{thread-safe}.
*/

/*!
    \enum QGLContext::BindOption
    \since 4.6

    A set of options to decide how to bind a texture using bindTexture().

    \value NoBindOption Don't do anything, pass the texture straight
    through.

    \value InvertedYBindOption Specifies that the texture should be flipped
    over the X axis so that the texture coordinate 0,0 corresponds to
    the top left corner. Inverting the texture implies a deep copy
    prior to upload.

    \value MipmapBindOption Specifies that bindTexture() should try
    to generate mipmaps.  If the GL implementation supports the \c
    GL_SGIS_generate_mipmap extension, mipmaps will be automatically
    generated for the texture. Mipmap generation is only supported for
    the \c GL_TEXTURE_2D target.

    \value PremultipliedAlphaBindOption Specifies that the image should be
    uploaded with premultiplied alpha and does a conversion accordingly.

    \value LinearFilteringBindOption Specifies that the texture filtering
    should be set to GL_LINEAR. Default is GL_NEAREST. If mipmap is
    also enabled, filtering will be set to GL_LINEAR_MIPMAP_LINEAR.

    \value DefaultBindOption In Qt 4.5 and earlier, bindTexture()
    would mirror the image and automatically generate mipmaps. This
    option helps preserve this default behavior.

    \omitvalue CanFlipNativePixmapBindOption Used by x11 from pixmap to choose
    whether or not it can bind the pixmap upside down or not.

    \omitvalue MemoryManagedBindOption Used by paint engines to
    indicate that the pixmap should be memory managed along side with
    the pixmap/image that it stems from, e.g. installing destruction
    hooks in them.

    \omitvalue InternalBindOption
*/

/*!
    \obsolete

    Constructs an OpenGL context for the given paint \a device, which
    can be a widget or a pixmap. The \a format specifies several
    display options for the context.

    If the underlying OpenGL/Window system cannot satisfy all the
    features requested in \a format, the nearest subset of features
    will be used. After creation, the format() method will return the
    actual format obtained.

    Note that after a QGLContext object has been constructed, \l
    create() must be called explicitly to create the actual OpenGL
    context. The context will be \l {isValid()}{invalid} if it was not
    possible to obtain a GL context at all.
*/

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

/*!
    Destroys the OpenGL context and frees its resources.
*/

QGLContext::~QGLContext()
{
   // remove any textures cached in this context
   QGLTextureCache::instance()->removeContextTextures(this);

   // clean up resources specific to this context
   d_ptr->cleanup();
   // clean up resources belonging to this context's group
   d_ptr->group->cleanupResources(this);

   QGLSignalProxy::instance()->emitAboutToDestroyContext(this);
   reset();
}

void QGLContextPrivate::cleanup()
{
   QHash<QGLContextResourceBase *, void *>::const_iterator it;

   for (it = m_resources.begin(); it != m_resources.end(); ++it) {
      it.key()->freeResource(it.value());
   }
   m_resources.clear();
}

#define ctx q_ptr
void QGLContextPrivate::setVertexAttribArrayEnabled(int arrayIndex, bool enabled)
{
   Q_ASSERT(arrayIndex < QT_GL_VERTEX_ARRAY_TRACKED_COUNT);

#ifdef glEnableVertexAttribArray
   Q_ASSERT(glEnableVertexAttribArray);
#endif

   if (vertexAttributeArraysEnabledState[arrayIndex] && !enabled) {
      glDisableVertexAttribArray(arrayIndex);
   }

   if (!vertexAttributeArraysEnabledState[arrayIndex] && enabled) {
      glEnableVertexAttribArray(arrayIndex);
   }

   vertexAttributeArraysEnabledState[arrayIndex] = enabled;
}

void QGLContextPrivate::syncGlState()
{
#ifdef glEnableVertexAttribArray
   Q_ASSERT(glEnableVertexAttribArray);
#endif
   for (int i = 0; i < QT_GL_VERTEX_ARRAY_TRACKED_COUNT; ++i) {
      if (vertexAttributeArraysEnabledState[i]) {
         glEnableVertexAttribArray(i);
      } else {
         glDisableVertexAttribArray(i);
      }
   }

}
#undef ctx

#ifdef QT_NO_EGL
void QGLContextPrivate::swapRegion(const QRegion &)
{
   Q_Q(QGLContext);
   q->swapBuffers();
}
#endif

/*!
    \overload

    Reads the compressed texture file \a fileName and generates a 2D GL
    texture from it.

    This function can load DirectDrawSurface (DDS) textures in the
    DXT1, DXT3 and DXT5 DDS formats if the \c GL_ARB_texture_compression
    and \c GL_EXT_texture_compression_s3tc extensions are supported.

    Since 4.6.1, textures in the ETC1 format can be loaded if the
    \c GL_OES_compressed_ETC1_RGB8_texture extension is supported
    and the ETC1 texture has been encapsulated in the PVR container format.
    Also, textures in the PVRTC2 and PVRTC4 formats can be loaded
    if the \c GL_IMG_texture_compression_pvrtc extension is supported.

    \sa deleteTexture()
*/

GLuint QGLContext::bindTexture(const QString &fileName)
{
   QGLTexture texture(this);
   QSize size = texture.bindCompressedTexture(fileName);
   if (!size.isValid()) {
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

QRgb qt_gl_convertToGLFormat(QRgb src_pixel, GLenum texture_format)
{
   return qt_gl_convertToGLFormatHelper(src_pixel, texture_format);
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
      uchar *srcPixels = (uchar *) img.scanLine(img.height() - 1);
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

#if defined(Q_WS_X11) || defined(Q_OS_MAC) || defined(Q_WS_QWS) || defined(Q_WS_QPA)
QGLExtensionFuncs &QGLContextPrivate::extensionFuncs(const QGLContext *)
{
   return qt_extensionFuncs;
}
#endif

QImage QGLContextPrivate::convertToGLFormat(const QImage &image, bool force_premul,
      GLenum texture_format)
{
   QImage::Format target_format = image.format();
   if (force_premul || image.format() != QImage::Format_ARGB32) {
      target_format = QImage::Format_ARGB32_Premultiplied;
   }

   QImage result(image.width(), image.height(), target_format);
   convertToGLFormatHelper(result, image.convertToFormat(target_format), texture_format);
   return result;
}

/*! \internal */
QGLTexture *QGLContextPrivate::bindTexture(const QImage &image, GLenum target, GLint format,
      QGLContext::BindOptions options)
{
   Q_Q(QGLContext);

   const qint64 key = image.cacheKey();
   QGLTexture *texture = textureCacheLookup(key, target);
   if (texture) {
      if (image.paintingActive()) {
         // A QPainter is active on the image - take the safe route and replace the texture.
         q->deleteTexture(texture->id);
         texture = 0;
      } else {
         glBindTexture(target, texture->id);
         return texture;
      }
   }

   if (!texture) {
      texture = bindTexture(image, target, format, key, options);
   }
   // NOTE: bindTexture(const QImage&, GLenum, GLint, const qint64, bool) should never return null
   Q_ASSERT(texture);

   // Enable the cleanup hooks for this image so that the texture cache entry is removed when the
   // image gets deleted:
   QImagePixmapCleanupHooks::enableCleanupHooks(image);

   return texture;
}

// #define QGL_BIND_TEXTURE_DEBUG

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

#ifdef QGL_BIND_TEXTURE_DEBUG
   printf("QGLContextPrivate::bindTexture(), imageSize=(%d,%d), internalFormat =0x%x, options=%x, key=%llx\n",
          image.width(), image.height(), internalFormat, int(options), key);
   QTime time;
   time.start();
#endif

#ifndef QT_NO_DEBUG
   // Reset the gl error stack...git
   while (glGetError() != GL_NO_ERROR) ;
#endif

   // Scale the pixmap if needed. GL textures needs to have the
   // dimensions 2^n+2(border) x 2^m+2(border), unless we're using GL
   // 2.0 or use the GL_TEXTURE_RECTANGLE texture target
   int tx_w = qt_next_power_of_two(image.width());
   int tx_h = qt_next_power_of_two(image.height());

   QImage img = image;

   if (!(QGLExtensions::glExtensions() & QGLExtensions::NPOTTextures)
         && !(QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Version_2_0)
         && (target == GL_TEXTURE_2D && (tx_w != image.width() || tx_h != image.height()))) {
      img = img.scaled(tx_w, tx_h);
#ifdef QGL_BIND_TEXTURE_DEBUG
      printf(" - upscaled to %dx%d (%d ms)\n", tx_w, tx_h, time.elapsed());

#endif
   }

   GLuint filtering = options & QGLContext::LinearFilteringBindOption ? GL_LINEAR : GL_NEAREST;

   GLuint tx_id;
   glGenTextures(1, &tx_id);
   glBindTexture(target, tx_id);
   glTexParameterf(target, GL_TEXTURE_MAG_FILTER, filtering);

#if defined(QT_OPENGL_ES_2)
   bool genMipmap = false;
#endif
   if (glFormat.directRendering()
         && (QGLExtensions::glExtensions() & QGLExtensions::GenerateMipmap)
         && target == GL_TEXTURE_2D
         && (options & QGLContext::MipmapBindOption)) {
#if !defined(QT_OPENGL_ES_2)
      glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
#ifndef QT_OPENGL_ES
      glTexParameteri(target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
#else
      glTexParameterf(target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
#endif
#else
      glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
      genMipmap = true;
#endif
      glTexParameterf(target, GL_TEXTURE_MIN_FILTER, options & QGLContext::LinearFilteringBindOption
                      ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
#ifdef QGL_BIND_TEXTURE_DEBUG
      printf(" - generating mipmaps (%d ms)\n", time.elapsed());
#endif
   } else {
      glTexParameterf(target, GL_TEXTURE_MIN_FILTER, filtering);
   }

   QImage::Format target_format = img.format();
   bool premul = options & QGLContext::PremultipliedAlphaBindOption;
   GLenum externalFormat;
   GLuint pixel_type;
   if (QGLExtensions::glExtensions() & QGLExtensions::BGRATextureFormat) {
      externalFormat = GL_BGRA;
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
      case QImage::Format_RGB16:
         pixel_type = GL_UNSIGNED_SHORT_5_6_5;
         externalFormat = GL_RGB;
         internalFormat = GL_RGB;
         break;
      case QImage::Format_RGB32:
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

   if (externalFormat == GL_RGBA) {
      // The only case where we end up with a depth different from
      // 32 in the switch above is for the RGB16 case, where we set
      // the format to GL_RGB
      Q_ASSERT(img.depth() == 32);
      qgl_byteSwapImage(img, pixel_type);
#ifdef QGL_BIND_TEXTURE_DEBUG
      printf(" - did byte swapping (%d ms)\n", time.elapsed());
#endif
   }
#ifdef QT_OPENGL_ES
   // OpenGL/ES requires that the internal and external formats be
   // identical.
   internalFormat = externalFormat;
#endif
#ifdef QGL_BIND_TEXTURE_DEBUG
   printf(" - uploading, image.format=%d, externalFormat=0x%x, internalFormat=0x%x, pixel_type=0x%x\n",
          img.format(), externalFormat, internalFormat, pixel_type);
#endif

   const QImage &constRef = img; // to avoid detach in bits()...

   glTexImage2D(target, 0, internalFormat, img.width(), img.height(), 0, externalFormat,
                pixel_type, constRef.bits());

#if defined(QT_OPENGL_ES_2)
   if (genMipmap) {
      glGenerateMipmap(target);
   }
#endif

#ifndef QT_NO_DEBUG
   GLenum error = glGetError();
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

/*! \internal */
QGLTexture *QGLContextPrivate::bindTexture(const QPixmap &pixmap, GLenum target, GLint format,
      QGLContext::BindOptions options)
{
   Q_Q(QGLContext);
   QPixmapData *pd = pixmap.pixmapData();
#if !defined(QT_OPENGL_ES_1)
   if (target == GL_TEXTURE_2D && pd->classId() == QPixmapData::OpenGLClass) {
      const QGLPixmapData *data = static_cast<const QGLPixmapData *>(pd);

      if (data->isValidContext(q)) {
         data->bind();
         return data->texture();
      }
   }
#else
   Q_UNUSED(pd);
#endif

   const qint64 key = pixmap.cacheKey();
   QGLTexture *texture = textureCacheLookup(key, target);
   if (texture) {
      if (pixmap.paintingActive()) {
         // A QPainter is active on the pixmap - take the safe route and replace the texture.
         q->deleteTexture(texture->id);
         texture = 0;
      } else {
         glBindTexture(target, texture->id);
         return texture;
      }
   }

#if defined(Q_WS_X11)
   // Try to use texture_from_pixmap
   const QX11Info *xinfo = qt_x11Info(paintDevice);
   if (pd->classId() == QPixmapData::X11Class && pd->pixelType() == QPixmapData::PixmapType
         && xinfo && xinfo->screen() == pixmap.x11Info().screen() && target == GL_TEXTURE_2D
         && QApplication::instance()->thread() == QThread::currentThread()) {

      if (! workaround_brokenTextureFromPixmap_init) {
         workaround_brokenTextureFromPixmap_init = true;

         const QString &versionStr = cs_glGetString(GL_VERSION);
         const int pos = versionStr.indexOf("NVIDIA ");

         if (pos >= 0) {
            const QString nvidiaVersionString = versionStr.mid(pos + strlen("NVIDIA "));

            if (nvidiaVersionString.startsWith("195") || nvidiaVersionString.startsWith("256")) {
               workaround_brokenTextureFromPixmap = true;
            }
         }
      }

      if (!workaround_brokenTextureFromPixmap) {
         texture = bindTextureFromNativePixmap(const_cast<QPixmap *>(&pixmap), key, options);

         if (texture) {
            texture->options |= QGLContext::MemoryManagedBindOption;
            texture->boundPixmap = pd;
            boundPixmaps.insert(pd, QPixmap(pixmap));
         }
      }
   }
#endif

   if (!texture) {

      QImage image = pixmap.toImage();
      // If the system depth is 16 and the pixmap doesn't have an alpha channel
      // then we convert it to RGB16 in the hope that it gets uploaded as a 16
      // bit texture which is much faster to access than a 32-bit one.
      if (pixmap.depth() == 16 && !image.hasAlphaChannel() ) {
         image = image.convertToFormat(QImage::Format_RGB16);
      }
      texture = bindTexture(image, target, format, key, options);

   }
   // NOTE: bindTexture(const QImage&, GLenum, GLint, const qint64, bool) should never return null
   Q_ASSERT(texture);

   if (texture->id > 0) {
      QImagePixmapCleanupHooks::enableCleanupHooks(pixmap);
   }

   return texture;
}

/*! \internal */
int QGLContextPrivate::maxTextureSize()
{
   if (max_texture_size != -1) {
      return max_texture_size;
   }

   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

#if defined(QT_OPENGL_ES)
   return max_texture_size;
#else
   GLenum proxy = GL_PROXY_TEXTURE_2D;

   GLint size;
   GLint next = 64;
   glTexImage2D(proxy, 0, GL_RGBA, next, next, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
   glGetTexLevelParameteriv(proxy, 0, GL_TEXTURE_WIDTH, &size);
   if (size == 0) {
      return max_texture_size;
   }
   do {
      size = next;
      next = size * 2;

      if (next > max_texture_size) {
         break;
      }
      glTexImage2D(proxy, 0, GL_RGBA, next, next, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
      glGetTexLevelParameteriv(proxy, 0, GL_TEXTURE_WIDTH, &next);
   } while (next > size);

   max_texture_size = size;
   return max_texture_size;
#endif
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

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
GLuint QGLContext::bindTexture(const QImage &image, QMacCompatGLenum target, QMacCompatGLint format)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(image, GLenum(target), GLint(format), DefaultBindOption);
   return texture->id;
}

/*! \internal */
GLuint QGLContext::bindTexture(const QImage &image, QMacCompatGLenum target, QMacCompatGLint format,
                               BindOptions options)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(image, GLenum(target), GLint(format), options);
   return texture->id;
}
#endif

/*! \overload

    Generates and binds a 2D GL texture based on \a pixmap.
*/
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

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
GLuint QGLContext::bindTexture(const QPixmap &pixmap, QMacCompatGLenum target, QMacCompatGLint format)
{
   if (pixmap.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(pixmap, GLenum(target), GLint(format), DefaultBindOption);
   return texture->id;
}
/*! \internal */
GLuint QGLContext::bindTexture(const QPixmap &pixmap, QMacCompatGLenum target, QMacCompatGLint format,
                               BindOptions options)
{
   if (pixmap.isNull()) {
      return 0;
   }

   Q_D(QGLContext);
   QGLTexture *texture = d->bindTexture(pixmap, GLenum(target), GLint(format), options);
   return texture->id;
}
#endif

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
   glDeleteTextures(1, &id);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLContext::deleteTexture(QMacCompatGLuint id)
{
   return deleteTexture(GLuint(id));
}
#endif

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
   GLfloat tx = 1.0f;
   GLfloat ty = 1.0f;

#ifdef QT_OPENGL_ES
   Q_UNUSED(textureWidth);
   Q_UNUSED(textureHeight);
   Q_UNUSED(textureTarget);
#else
   if (textureTarget != GL_TEXTURE_2D) {
      if (textureWidth == -1 || textureHeight == -1) {
         glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_WIDTH, &textureWidth);
         glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_HEIGHT, &textureHeight);
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

   glVertexPointer(2, GL_FLOAT, 0, vertexArray);
   glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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
#ifdef QT_OPENGL_ES
   if (textureTarget != GL_TEXTURE_2D) {
      qWarning("QGLContext::drawTexture(): texture target must be GL_TEXTURE_2D on OpenGL ES");
      return;
   }
#else
   const bool wasEnabled = glIsEnabled(GL_TEXTURE_2D);
   GLint oldTexture;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
#endif

   glEnable(textureTarget);
   glBindTexture(textureTarget, textureId);

   qDrawTextureRect(target, -1, -1, textureTarget);

#ifdef QT_OPENGL_ES
   glDisable(textureTarget);
#else
   if (!wasEnabled) {
      glDisable(textureTarget);
   }
   glBindTexture(textureTarget, oldTexture);
#endif
#else
   Q_UNUSED(target);
   Q_UNUSED(textureId);
   Q_UNUSED(textureTarget);
   qWarning("drawTexture() with OpenGL ES 2.0 requires an active OpenGL2 paint engine");
#endif
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLContext::drawTexture(const QRectF &target, QMacCompatGLuint textureId, QMacCompatGLenum textureTarget)
{
   drawTexture(target, GLuint(textureId), GLenum(textureTarget));
}
#endif

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
   Q_UNUSED(point);
   Q_UNUSED(textureId);
   Q_UNUSED(textureTarget);
   qWarning("drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget) not supported with OpenGL ES, use rect version instead");
#else

   const bool wasEnabled = glIsEnabled(GL_TEXTURE_2D);
   GLint oldTexture;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);

   glEnable(textureTarget);
   glBindTexture(textureTarget, textureId);

   GLint textureWidth;
   GLint textureHeight;

   glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_WIDTH, &textureWidth);
   glGetTexLevelParameteriv(textureTarget, 0, GL_TEXTURE_HEIGHT, &textureHeight);

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
      glDisable(textureTarget);
   }
   glBindTexture(textureTarget, oldTexture);
#endif
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLContext::drawTexture(const QPointF &point, QMacCompatGLuint textureId, QMacCompatGLenum textureTarget)
{
   drawTexture(point, GLuint(textureId), GLenum(textureTarget));
}
#endif


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
   if (isValid()) {
      reset();
   }
   d->paintDevice = pDev;
   if (d->paintDevice && (d->paintDevice->devType() != QInternal::Widget
                          && d->paintDevice->devType() != QInternal::Pixmap
                          && d->paintDevice->devType() != QInternal::Pbuffer)) {
      qWarning("QGLContext: Unsupported paint device type");
   }
}

/*!
    \fn bool QGLContext::isValid() const

    Returns true if a GL rendering context has been successfully
    created; otherwise returns false.
*/

/*!
    \fn void QGLContext::setValid(bool valid)
    \internal

    Forces the GL rendering context to be valid.
*/

/*!
    \fn bool QGLContext::isSharing() const

    Returns true if this context is sharing its GL context with
    another QGLContext, otherwise false is returned. Note that context
    sharing might not be supported between contexts with different
    formats.
*/

/*!
    Returns true if \a context1 and \a context2 are sharing their
    GL resources such as textures, shader programs, etc;
    otherwise returns false.

    \since 4.6
*/
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
#ifdef Q_WS_QPA
   if (!d->paintDevice && !d->platformContext)
#else
   if (!d->paintDevice)
#endif
      return false;

   reset();
   d->valid = chooseContext(shareContext);
   if (d->valid && d->paintDevice && d->paintDevice->devType() == QInternal::Widget) {
      QWidgetPrivate *wd = qt_widget_private(static_cast<QWidget *>(d->paintDevice));
      wd->usesDoubleBufferedGLContext = d->glFormat.doubleBuffer();
   }
#ifndef Q_WS_QPA //We do this in choose context->setupSharing()
   if (d->sharing) { // ok, we managed to share
      QGLContextGroup::addShare(this, shareContext);
   }
#endif
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
   return d->paintDevice->devType() == QInternal::Pixmap;
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
#ifdef Q_WS_QPA
   if (const QPlatformGLContext *threadContext = QPlatformGLContext::currentContext()) {
      return QGLContext::fromPlatformGLContext(const_cast<QPlatformGLContext *>(threadContext));
   }
   return 0;
#else
   QGLThreadContext *threadContext = qgl_context_storage.localData();
   if (threadContext) {
      return threadContext->context;
   }
   return 0;
#endif
}

void QGLContextPrivate::setCurrentContext(QGLContext *context)
{
#ifdef Q_WS_QPA
   Q_UNUSED(context);
#else
   QGLThreadContext *threadContext = qgl_context_storage.localData();
   if (!threadContext) {
      if (!QThread::currentThread()) {
         // We don't have a current QThread, so just set the static.
         QGLContext::currentCtx = context;
         return;
      }
      threadContext = new QGLThreadContext;
      qgl_context_storage.setLocalData(threadContext);
   }
   threadContext->context = context;
   QGLContext::currentCtx = context; // XXX: backwards-compat, not thread-safe
#endif
}

/*!
    \fn bool QGLContext::chooseContext(const QGLContext* shareContext = 0)

    This semi-internal function is called by create(). It creates a
    system-dependent OpenGL handle that matches the format() of \a
    shareContext as closely as possible, returning true if successful
    or false if a suitable handle could not be found.

    On Windows, it calls the virtual function choosePixelFormat(),
    which finds a matching pixel format identifier. On X11, it calls
    the virtual function chooseVisual() which finds an appropriate X
    visual. On other platforms it may work differently.
*/

/*! \fn int QGLContext::choosePixelFormat(void* dummyPfd, HDC pdc)

    \bold{Win32 only:} This virtual function chooses a pixel format
    that matches the OpenGL \link setFormat() format\endlink.
    Reimplement this function in a subclass if you need a custom
    context.

    \warning The \a dummyPfd pointer and \a pdc are used as a \c
    PIXELFORMATDESCRIPTOR*. We use \c void to avoid using
    Windows-specific types in our header files.

    \sa chooseContext()
*/

/*! \fn void *QGLContext::chooseVisual()

  \bold{X11 only:} This virtual function tries to find a visual that
  matches the format, reducing the demands if the original request
  cannot be met.

  The algorithm for reducing the demands of the format is quite
  simple-minded, so override this method in your subclass if your
  application has spcific requirements on visual selection.

  \sa chooseContext()
*/

/*! \fn void *QGLContext::tryVisual(const QGLFormat& f, int bufDepth)
  \internal

  \bold{X11 only:} This virtual function chooses a visual that matches
  the OpenGL \link format() format\endlink. Reimplement this function
  in a subclass if you need a custom visual.

  \sa chooseContext()
*/

/*!
    \fn void QGLContext::reset()

    Resets the context and makes it invalid.

    \sa create(), isValid()
*/


/*!
    \fn void QGLContext::makeCurrent()

    Makes this context the current OpenGL rendering context. All GL
    functions you call operate on this context until another context
    is made current.

    In some very rare cases the underlying call may fail. If this
    occurs an error message is output to stderr.
*/


/*!
    \fn void QGLContext::swapBuffers() const

    Swaps the screen contents with an off-screen buffer. Only works if
    the context is in double buffer mode.

    \sa QGLFormat::setDoubleBuffer()
*/


/*!
    \fn void QGLContext::doneCurrent()

    Makes no GL context the current context. Normally, you do not need
    to call this function; QGLContext calls it as necessary.
*/


/*!
    \fn QPaintDevice* QGLContext::device() const

    Returns the paint device set for this context.

    \sa QGLContext::QGLContext()
*/

/*!
    \obsolete
    \fn void QGLContext::generateFontDisplayLists(const QFont& font, int listBase)

    Generates a set of 256 display lists for the 256 first characters
    in the font \a font. The first list will start at index \a listBase.

    \sa QGLWidget::renderText()
*/



/*****************************************************************************
  QGLWidget implementation
 *****************************************************************************/


/*!
    \class QGLWidget
    \brief The QGLWidget class is a widget for rendering OpenGL graphics.

    \ingroup painting-3D


    QGLWidget provides functionality for displaying OpenGL graphics
    integrated into a Qt application. It is very simple to use. You
    inherit from it and use the subclass like any other QWidget,
    except that you have the choice between using QPainter and
    standard OpenGL rendering commands.

    QGLWidget provides three convenient virtual functions that you can
    reimplement in your subclass to perform the typical OpenGL tasks:

    \list
    \i paintGL() - Renders the OpenGL scene. Gets called whenever the widget
    needs to be updated.
    \i resizeGL() - Sets up the OpenGL viewport, projection, etc. Gets
    called whenever the widget has been resized (and also when it
    is shown for the first time because all newly created widgets get a
    resize event automatically).
    \i initializeGL() - Sets up the OpenGL rendering context, defines display
    lists, etc. Gets called once before the first time resizeGL() or
    paintGL() is called.
    \endlist

    Here is a rough outline of how a QGLWidget subclass might look:

    \snippet doc/src/snippets/code/src_opengl_qgl.cpp 8

    If you need to trigger a repaint from places other than paintGL()
    (a typical example is when using \link QTimer timers\endlink to
    animate scenes), you should call the widget's updateGL() function.

    Your widget's OpenGL rendering context is made current when
    paintGL(), resizeGL(), or initializeGL() is called. If you need to
    call the standard OpenGL API functions from other places (e.g. in
    your widget's constructor or in your own paint functions), you
    must call makeCurrent() first.

    QGLWidget provides functions for requesting a new display \link
    QGLFormat format\endlink and you can also create widgets with
    customized rendering \link QGLContext contexts\endlink.

    You can also share OpenGL display lists between QGLWidget objects (see
    the documentation of the QGLWidget constructors for details).

    Note that under Windows, the QGLContext belonging to a QGLWidget
    has to be recreated when the QGLWidget is reparented. This is
    necessary due to limitations on the Windows platform. This will
    most likely cause problems for users that have subclassed and
    installed their own QGLContext on a QGLWidget. It is possible to
    work around this issue by putting the QGLWidget inside a dummy
    widget and then reparenting the dummy widget, instead of the
    QGLWidget. This will side-step the issue altogether, and is what
    we recommend for users that need this kind of functionality.

    On Mac OS X, when Qt is built with Cocoa support, a QGLWidget
    can't have any sibling widgets placed ontop of itself. This is due
    to limitations in the Cocoa API and is not supported by Apple.

    \section1 Overlays

    The QGLWidget creates a GL overlay context in addition to the
    normal context if overlays are supported by the underlying system.

    If you want to use overlays, you specify it in the \link QGLFormat
    format\endlink. (Note: Overlay must be requested in the format
    passed to the QGLWidget constructor.) Your GL widget should also
    implement some or all of these virtual methods:

    \list
    \i paintOverlayGL()
    \i resizeOverlayGL()
    \i initializeOverlayGL()
    \endlist

    These methods work in the same way as the normal paintGL() etc.
    functions, except that they will be called when the overlay
    context is made current. You can explicitly make the overlay
    context current by using makeOverlayCurrent(), and you can access
    the overlay context directly (e.g. to ask for its transparent
    color) by calling overlayContext().

    On X servers in which the default visual is in an overlay plane,
    non-GL Qt windows can also be used for overlays.

    \section1 Painting Techniques

    As described above, subclass QGLWidget to render pure 3D content in the
    following way:

    \list
    \o Reimplement the QGLWidget::initializeGL() and QGLWidget::resizeGL() to
       set up the OpenGL state and provide a perspective transformation.
    \o Reimplement QGLWidget::paintGL() to paint the 3D scene, calling only
       OpenGL functions to draw on the widget.
    \endlist

    It is also possible to draw 2D graphics onto a QGLWidget subclass, it is necessary
    to reimplement QGLWidget::paintEvent() and do the following:

    \list
    \o Construct a QPainter object.
    \o Initialize it for use on the widget with the QPainter::begin() function.
    \o Draw primitives using QPainter's member functions.
    \o Call QPainter::end() to finish painting.
    \endlist

    Overpainting 2D content on top of 3D content takes a little more effort.
    One approach to doing this is shown in the
    \l{Overpainting Example}{Overpainting} example.

    \section1 Threading

    As of Qt version 4.8, support for doing threaded GL rendering has
    been improved. There are three scenarios that we currently support:
    \list
    \o 1. Buffer swapping in a thread.

    Swapping buffers in a double buffered context may be a
    synchronous, locking call that may be a costly operation in some
    GL implementations. Especially so on embedded devices. It's not
    optimal to have the CPU idling while the GPU is doing a buffer
    swap. In those cases it is possible to do the rendering in the
    main thread and do the actual buffer swap in a separate
    thread. This can be done with the following steps:

    1. Call doneCurrent() in the main thread when the rendering is
    finished.

    2. Notify the swapping thread that it can grab the context.

    3. Make the rendering context current in the swapping thread with
    makeCurrent() and then call swapBuffers().

    4. Call doneCurrent() in the swapping thread and notify the main
    thread that swapping is done.

    Doing this will free up the main thread so that it can continue
    with, for example, handling UI events or network requests. Even if
    there is a context swap involved, it may be preferable compared to
    having the main thread wait while the GPU finishes the swap
    operation. Note that this is highly implementation dependent.

    \o 2. Texture uploading in a thread.

    Doing texture uploads in a thread may be very useful for
    applications handling large amounts of images that needs to be
    displayed, like for instance a photo gallery application. This is
    supported in Qt through the existing bindTexture() API. A simple
    way of doing this is to create two sharing QGLWidgets. One is made
    current in the main GUI thread, while the other is made current in
    the texture upload thread. The widget in the uploading thread is
    never shown, it is only used for sharing textures with the main
    thread. For each texture that is bound via bindTexture(), notify
    the main thread so that it can start using the texture.

    \o 3. Using QPainter to draw into a QGLWidget in a thread.

    In Qt 4.8, it is possible to draw into a QGLWidget using a
    QPainter in a separate thread. Note that this is also possible for
    QGLPixelBuffers and QGLFramebufferObjects. Since this is only
    supported in the GL 2 paint engine, OpenGL 2.0 or OpenGL ES 2.0 is
    required.

    QGLWidgets can only be created in the main GUI thread. This means
    a call to doneCurrent() is necessary to release the GL context
    from the main thread, before the widget can be drawn into by
    another thread. Also, the main GUI thread will dispatch resize and
    paint events to a QGLWidget when the widget is resized, or parts
    of it becomes exposed or needs redrawing. It is therefore
    necessary to handle those events because the default
    implementations inside QGLWidget will try to make the QGLWidget's
    context current, which again will interfere with any threads
    rendering into the widget. Reimplement QGLWidget::paintEvent() and
    QGLWidget::resizeEvent() to notify the rendering thread that a
    resize or update is necessary, and be careful not to call the base
    class implementation. If you are rendering an animation, it might
    not be necessary to handle the paint event at all since the
    rendering thread is doing regular updates. Then it would be enough
    to reimplement QGLWidget::paintEvent() to do nothing.

    \endlist

    As a general rule when doing threaded rendering: be aware that
    binding and releasing contexts in different threads have to be
    synchronized by the user. A GL rendering context can only be
    current in one thread at any time. If you try to open a QPainter
    on a QGLWidget and the widget's rendering context is current in
    another thread, it will fail.

    Note that under X11 it is necessary to set the
    Qt::AA_X11InitThreads application attribute to make the X11
    library and GLX calls thread safe, otherwise the above scenarios
    will fail.

    In addition to this, rendering using raw GL calls in a separate
    thread is supported.

    \e{OpenGL is a trademark of Silicon Graphics, Inc. in the United States and other
    countries.}

    \sa QGLPixelBuffer, {Hello GL Example}, {2D Painting Example}, {Overpainting Example},
        {Grabber Example}
*/

/*!
    Constructs an OpenGL widget with a \a parent widget.

    The \link QGLFormat::defaultFormat() default format\endlink is
    used. The widget will be \link isValid() invalid\endlink if the
    system has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.

    The \a parent and widget flag, \a f, arguments are passed
    to the QWidget constructor.

    If \a shareWidget is a valid QGLWidget, this widget will share
    OpenGL display lists and texture objects with \a shareWidget. But
    if \a shareWidget and this widget have different \l {format()}
    {formats}, sharing might not be possible. You can check whether
    sharing is in effect by calling isSharing().

    The initialization of OpenGL rendering state, etc. should be done
    by overriding the initializeGL() function, rather than in the
    constructor of your QGLWidget subclass.

    \sa QGLFormat::defaultFormat(), {Textures Example}
*/

QGLWidget::QGLWidget(QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags f)
   : QWidget(*(new QGLWidgetPrivate), parent, f | Qt::MSWindowsOwnDC)
{
   Q_D(QGLWidget);
   setAttribute(Qt::WA_PaintOnScreen);
   setAttribute(Qt::WA_NoSystemBackground);
   setAutoFillBackground(true); // for compatibility
   d->init(new QGLContext(QGLFormat::defaultFormat(), this), shareWidget);
}


/*!
    Constructs an OpenGL widget with parent \a parent.

    The \a format argument specifies the desired \link QGLFormat
    rendering options \endlink. If the underlying OpenGL/Window system
    cannot satisfy all the features requested in \a format, the
    nearest subset of features will be used. After creation, the
    format() method will return the actual format obtained.

    The widget will be \link isValid() invalid\endlink if the system
    has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.

    The \a parent and widget flag, \a f, arguments are passed
    to the QWidget constructor.

    If \a shareWidget is a valid QGLWidget, this widget will share
    OpenGL display lists and texture objects with \a shareWidget. But
    if \a shareWidget and this widget have different \l {format()}
    {formats}, sharing might not be possible. You can check whether
    sharing is in effect by calling isSharing().

    The initialization of OpenGL rendering state, etc. should be done
    by overriding the initializeGL() function, rather than in the
    constructor of your QGLWidget subclass.

    \sa QGLFormat::defaultFormat(), isValid()
*/

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

/*!
    Constructs an OpenGL widget with parent \a parent.

    The \a context argument is a pointer to the QGLContext that
    you wish to be bound to this widget. This allows you to pass in
    your own QGLContext sub-classes.

    The widget will be \link isValid() invalid\endlink if the system
    has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.

    The \a parent and widget flag, \a f, arguments are passed
    to the QWidget constructor.

    If \a shareWidget is a valid QGLWidget, this widget will share
    OpenGL display lists and texture objects with \a shareWidget. But
    if \a shareWidget and this widget have different \l {format()}
    {formats}, sharing might not be possible. You can check whether
    sharing is in effect by calling isSharing().

    The initialization of OpenGL rendering state, etc. should be done
    by overriding the initializeGL() function, rather than in the
    constructor of your QGLWidget subclass.

    \sa QGLFormat::defaultFormat(), isValid()
*/
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
#if defined(GLX_MESA_release_buffers) && defined(QGL_USE_MESA_EXT)
   bool doRelease = (glcx && glcx->windowCreated());
#endif
   delete d->glcx;
   d->glcx = 0;
#if defined(Q_OS_WIN)
   delete d->olcx;
   d->olcx = 0;
#endif
#if defined(GLX_MESA_release_buffers) && defined(QGL_USE_MESA_EXT)
   if (doRelease) {
      glXReleaseBuffersMESA(x11Display(), winId());
   }
#endif
   d->cleanupColormaps();

#ifdef Q_OS_MAC
   QWidget *current = parentWidget();
   while (current) {
      qt_widget_private(current)->glWidgets.removeAll(QWidgetPrivate::GlWidgetInfo(this));
      if (current->isWindow()) {
         break;
      }
      current = current->parentWidget();
   };
#endif
}

/*!
    \fn QGLFormat QGLWidget::format() const

    Returns the format of the contained GL rendering context.
*/

/*!
    \fn bool QGLWidget::doubleBuffer() const

    Returns true if the contained GL rendering context has double
    buffering; otherwise returns false.

    \sa QGLFormat::doubleBuffer()
*/

/*!
    \fn void QGLWidget::setAutoBufferSwap(bool on)

    If \a on is true automatic GL buffer swapping is switched on;
    otherwise it is switched off.

    If \a on is true and the widget is using a double-buffered format,
    the background and foreground GL buffers will automatically be
    swapped after each paintGL() call.

    The buffer auto-swapping is on by default.

    \sa autoBufferSwap(), doubleBuffer(), swapBuffers()
*/

/*!
    \fn bool QGLWidget::autoBufferSwap() const

    Returns true if the widget is doing automatic GL buffer swapping;
    otherwise returns false.

    \sa setAutoBufferSwap()
*/

/*!
    \fn void *QGLContext::getProcAddress(const QString &proc) const

    Returns a function pointer to the GL extension function passed in
    \a proc. 0 is returned if a pointer to the function could not be
    obtained.
*/

/*!
    \fn bool QGLWidget::isValid() const

    Returns true if the widget has a valid GL rendering context;
    otherwise returns false. A widget will be invalid if the system
    has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.
*/

bool QGLWidget::isValid() const
{
   Q_D(const QGLWidget);
   return d->glcx && d->glcx->isValid();
}

/*!
    \fn bool QGLWidget::isSharing() const

    Returns true if this widget's GL context is shared with another GL
    context, otherwise false is returned. Context sharing might not be
    possible if the widgets use different formats.

    \sa format()
*/

bool QGLWidget::isSharing() const
{
   Q_D(const QGLWidget);
   return d->glcx->isSharing();
}

/*!
    \fn void QGLWidget::makeCurrent()

    Makes this widget the current widget for OpenGL operations, i.e.
    makes the widget's rendering context the current OpenGL rendering
    context.
*/

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



/*!
    \fn void QGLWidget::makeOverlayCurrent()

    Makes the overlay context of this widget current. Use this if you
    need to issue OpenGL commands to the overlay context outside of
    initializeOverlayGL(), resizeOverlayGL(), and paintOverlayGL().

    Does nothing if this widget has no overlay.

    \sa makeCurrent()
*/


/*!
  \obsolete

  Sets a new format for this widget.

  If the underlying OpenGL/Window system cannot satisfy all the
  features requested in \a format, the nearest subset of features will
  be used. After creation, the format() method will return the actual
  rendering context format obtained.

  The widget will be assigned a new QGLContext, and the initializeGL()
  function will be executed for this new context before the first
  resizeGL() or paintGL().

  This method will try to keep display list and texture object sharing
  in effect with other QGLWidget objects, but changing the format might make
  sharing impossible. Use isSharing() to see if sharing is still in
  effect.

  \sa format(), isSharing(), isValid()
*/

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



/*!
    \fn void QGLWidget::updateGL()

    Updates the widget by calling glDraw().
*/

void QGLWidget::updateGL()
{
   if (updatesEnabled()) {
      glDraw();
   }
}


/*!
    \fn void QGLWidget::updateOverlayGL()

    Updates the widget's overlay (if any). Will cause the virtual
    function paintOverlayGL() to be executed.

    The widget's rendering context will become the current context and
    initializeGL() will be called if it hasn't already been called.
*/


/*!
    This virtual function is called once before the first call to
    paintGL() or resizeGL(), and then once whenever the widget has
    been assigned a new QGLContext. Reimplement it in a subclass.

    This function should set up any required OpenGL context rendering
    flags, defining display lists, etc.

    There is no need to call makeCurrent() because this has already
    been done when this function is called.
*/

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

/*! \fn bool QGLWidget::event(QEvent *e)
  \reimp
*/
#if !defined(Q_WS_QWS) && !defined(Q_WS_QPA)
bool QGLWidget::event(QEvent *e)
{
   Q_D(QGLWidget);

   if (e->type() == QEvent::Paint) {
      QPoint offset;
      QPaintDevice *redirectedDevice = d->redirected(&offset);
      if (redirectedDevice && redirectedDevice->devType() == QInternal::Pixmap) {
         d->restoreRedirected();
         QPixmap pixmap = renderPixmap();
         d->setRedirected(redirectedDevice, offset);
         QPainter p(redirectedDevice);
         p.drawPixmap(-offset, pixmap);
         return true;
      }
   }

#if defined(Q_WS_X11)
   if (e->type() == QEvent::ParentChange) {
      // if we've reparented a window that has the current context
      // bound, we need to rebind that context to the new window id
      if (d->glcx == QGLContext::currentContext()) {
         makeCurrent();
      }

      if (d->glcx->d_func()->screen != d->xinfo.screen() || testAttribute(Qt::WA_TranslucentBackground)) {
         setContext(new QGLContext(d->glcx->requestedFormat(), this));
         // ### recreating the overlay isn't supported atm
      }
   }

#ifndef QT_NO_EGL
   // A re-parent is likely to destroy the X11 window and re-create it. It is important
   // that we free the EGL surface _before_ the winID changes - otherwise we can leak.
   if (e->type() == QEvent::ParentAboutToChange) {
      d->glcx->d_func()->destroyEglSurfaceForDevice();
   }

   if ((e->type() == QEvent::ParentChange) || (e->type() == QEvent::WindowStateChange)) {
      // The window may have been re-created during re-parent or state change - if so, the EGL
      // surface will need to be re-created.
      d->recreateEglSurface();
   }
#endif

#elif defined(Q_OS_WIN)
   if (e->type() == QEvent::ParentChange) {
      QGLContext *newContext = new QGLContext(d->glcx->requestedFormat(), this);
      setContext(newContext, d->glcx);

      // the overlay needs to be recreated as well
      delete d->olcx;
      if (isValid() && context()->format().hasOverlay()) {
         d->olcx = new QGLContext(QGLFormat::defaultOverlayFormat(), this);
         if (!d->olcx->create(isSharing() ? d->glcx : 0)) {
            delete d->olcx;
            d->olcx = 0;
            d->glcx->d_func()->glFormat.setOverlay(false);
         }
      } else {
         d->olcx = 0;
      }
   } else if (e->type() == QEvent::Show) {
      if (!format().rgba()) {
         d->updateColormap();
      }
   }
#elif defined(Q_OS_MAC)
   if (e->type() == QEvent::MacGLWindowChange) {
      if (d->needWindowChange) {
         d->needWindowChange = false;
         d->glcx->updatePaintDevice();
         update();
      }
      return true;

   } else if (e->type() == QEvent::MacGLClearDrawable) {
      d->glcx->d_ptr->clearDrawable();

   }

#endif

   return QWidget::event(e);
}
#endif

/*!
    \fn void QGLWidget::paintEvent(QPaintEvent *event)

    Handles paint events passed in the \a event parameter. Will cause
    the virtual paintGL() function to be called.

    The widget's rendering context will become the current context and
    initializeGL() will be called if it hasn't already been called.
*/

void QGLWidget::paintEvent(QPaintEvent *)
{
   if (updatesEnabled()) {
      glDraw();
      updateOverlayGL();
   }
}


/*!
    \fn void QGLWidget::resizeEvent(QResizeEvent *event)

    Handles resize events that are passed in the \a event parameter.
    Calls the virtual function resizeGL().
*/


/*!
    \fn void QGLWidget::setMouseTracking(bool enable)

    If \a enable is true then mouse tracking is enabled; otherwise it
    is disabled.
*/


/*!
    Renders the current scene on a pixmap and returns the pixmap.

    You can use this method on both visible and invisible QGLWidget objects.

    This method will create a pixmap and a temporary QGLContext to
    render on the pixmap. It will then call initializeGL(),
    resizeGL(), and paintGL() on this context. Finally, the widget's
    original GL context is restored.

    The size of the pixmap will be \a w pixels wide and \a h pixels
    high unless one of these parameters is 0 (the default), in which
    case the pixmap will have the same size as the widget.

    If \a useContext is true, this method will try to be more
    efficient by using the existing GL context to render the pixmap.
    The default is false. Only use true if you understand the risks.
    Note that under Windows a temporary context has to be created
    and usage of the \e useContext parameter is not supported.

    Overlays are not rendered onto the pixmap.

    If the GL rendering context and the desktop have different bit
    depths, the result will most likely look surprising.

    Note that the creation of display lists, modifications of the view
    frustum etc. should be done from within initializeGL(). If this is
    not done, the temporary QGLContext will not be initialized
    properly, and the rendered pixmap may be incomplete/corrupted.
*/

QPixmap QGLWidget::renderPixmap(int w, int h, bool useContext)
{
   Q_D(QGLWidget);
   QSize sz = size();
   if ((w > 0) && (h > 0)) {
      sz = QSize(w, h);
   }

#if defined(Q_WS_X11)
   extern int qt_x11_preferred_pixmap_depth;
   int old_depth = qt_x11_preferred_pixmap_depth;
   qt_x11_preferred_pixmap_depth = x11Info().depth();

   QPixmapData *data = new QX11PixmapData(QPixmapData::PixmapType);
   data->resize(sz.width(), sz.height());
   QPixmap pm(data);
   qt_x11_preferred_pixmap_depth = old_depth;
   QX11Info xinfo = x11Info();

   // make sure we use a pixmap with the same depth/visual as the widget
   if (xinfo.visual() != QX11Info::appVisual()) {
      QX11InfoData *xd = pm.x11Info().getX11Data(true);
      xd->depth = xinfo.depth();
      xd->visual = static_cast<Visual *>(xinfo.visual());
      const_cast<QX11Info &>(pm.x11Info()).setX11Data(xd);
   }

#else
   QPixmap pm(sz);
#endif

   d->glcx->doneCurrent();

   bool success = true;

   if (useContext && isValid() && d->renderCxPm(&pm)) {
      return pm;
   }

   QGLFormat fmt = d->glcx->requestedFormat();
   fmt.setDirectRendering(false);                // Direct is unlikely to work
   fmt.setDoubleBuffer(false);                // We don't need dbl buf
#ifdef Q_OS_MAC // crash prevention on the Mac - it's unlikely to work anyway
   fmt.setSampleBuffers(false);
#endif

   QGLContext *ocx = d->glcx;
   ocx->doneCurrent();
   d->glcx = new QGLContext(fmt, &pm);
   d->glcx->create();

   if (d->glcx->isValid()) {
      updateGL();
   } else {
      success = false;
   }

   delete d->glcx;
   d->glcx = ocx;

   ocx->makeCurrent();

   if (success) {
#if defined(Q_WS_X11)
      if (xinfo.visual() != QX11Info::appVisual()) {
         QImage image = pm.toImage();
         QPixmap p = QPixmap::fromImage(image);
         return p;
      }
#endif
      return pm;
   }
   return QPixmap();
}

/*!
    Returns an image of the frame buffer. If \a withAlpha is true the
    alpha channel is included.

    Depending on your hardware, you can explicitly select which color
    buffer to grab with a glReadBuffer() call before calling this
    function.
*/
QImage QGLWidget::grabFrameBuffer(bool withAlpha)
{
   makeCurrent();
   QImage res;
   int w = width();
   int h = height();
   if (format().rgba()) {
      res = qt_gl_read_framebuffer(QSize(w, h), format().alpha(), withAlpha);
   } else {
#if defined (Q_OS_WIN) && !defined(QT_OPENGL_ES)
      res = QImage(w, h, QImage::Format_Indexed8);
      glReadPixels(0, 0, w, h, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, res.bits());
      const QVector<QColor> pal = QColormap::instance().colormap();
      if (pal.size()) {
         res.setColorCount(pal.size());
         for (int i = 0; i < pal.size(); i++) {
            res.setColor(i, pal.at(i).rgb());
         }
      }
      res = res.mirrored();
#endif
   }

   return res;
}



/*!
    Initializes OpenGL for this widget's context. Calls the virtual
    function initializeGL().
*/

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


/*!
    Executes the virtual function paintGL().

    The widget's rendering context will become the current context and
    initializeGL() will be called if it hasn't already been called.
*/

void QGLWidget::glDraw()
{
   Q_D(QGLWidget);
   if (!isValid()) {
      return;
   }

   makeCurrent();

#ifndef QT_OPENGL_ES
   if (d->glcx->deviceIsPixmap()) {
      glDrawBuffer(GL_FRONT);
   }
#endif
   if (!d->glcx->initialized()) {
      glInit();
      resizeGL(d->glcx->device()->width(), d->glcx->device()->height()); // New context needs this "resize"
   }
   paintGL();
   if (doubleBuffer()) {
      if (d->autoSwap) {
         swapBuffers();
      }
   } else {
      glFlush();
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
#if !defined(QT_OPENGL_ES_2)
#ifdef QT_OPENGL_ES
   glColor4f(c.redF(), c.greenF(), c.blueF(), c.alphaF());
#else
   Q_D(const QGLWidget);
   const QGLContext *ctx = QGLContext::currentContext();
   if (ctx) {
      if (ctx->format().rgba()) {
         glColor4f(c.redF(), c.greenF(), c.blueF(), c.alphaF());
      } else if (!d->cmap.isEmpty()) { // QGLColormap in use?
         int i = d->cmap.find(c.rgb());
         if (i < 0) {
            i = d->cmap.findNearest(c.rgb());
         }
         glIndexi(i);
      } else {
         glIndexi(ctx->colorIndex(c));
      }
   }
#endif //QT_OPENGL_ES
#else
   Q_UNUSED(c);
#endif //QT_OPENGL_ES_2
}

/*!
    Convenience function for specifying the clearing color to OpenGL.
    Calls glClearColor (in RGBA mode) or glClearIndex (in color-index
    mode) with the color \a c. Applies to this widgets GL context.

    \sa qglColor(), QGLContext::currentContext(), QColor
*/

void QGLWidget::qglClearColor(const QColor &c) const
{
#ifdef QT_OPENGL_ES
   glClearColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
#else
   Q_D(const QGLWidget);
   const QGLContext *ctx = QGLContext::currentContext();
   if (ctx) {
      if (ctx->format().rgba()) {
         glClearColor(c.redF(), c.greenF(), c.blueF(), c.alphaF());
      } else if (!d->cmap.isEmpty()) { // QGLColormap in use?
         int i = d->cmap.find(c.rgb());
         if (i < 0) {
            i = d->cmap.findNearest(c.rgb());
         }
         glClearIndex(i);
      } else {
         glClearIndex(ctx->colorIndex(c));
      }
   }
#endif
}


/*!
    Converts the image \a img into the unnamed format expected by
    OpenGL functions such as glTexImage2D(). The returned image is not
    usable as a QImage, but QImage::width(), QImage::height() and
    QImage::bits() may be used with OpenGL. The GL format used is
    \c GL_RGBA.

    \omit ###

    \l opengl/texture example
    The following few lines are from the texture example. Most of the
    code is irrelevant, so we just quote the relevant bits:

    \quotefromfile opengl/texture/gltexobj.cpp
    \skipto tex1
    \printline tex1
    \printline gllogo.bmp

    We create \e tex1 (and another variable) for OpenGL, and load a real
    image into \e buf.

    \skipto convertToGLFormat
    \printline convertToGLFormat

    A few lines later, we convert \e buf into OpenGL format and store it
    in \e tex1.

    \skipto glTexImage2D
    \printline glTexImage2D
    \printline tex1.bits

    Note the dimension restrictions for texture images as described in
    the glTexImage2D() documentation. The width must be 2^m + 2*border
    and the height 2^n + 2*border where m and n are integers and
    border is either 0 or 1.

    Another function in the same example uses \e tex1 with OpenGL.

    \endomit
*/

QImage QGLWidget::convertToGLFormat(const QImage &img)
{
   QImage res(img.size(), QImage::Format_ARGB32);
   convertToGLFormatHelper(res, img.convertToFormat(QImage::Format_ARGB32), GL_RGBA);
   return res;
}


/*!
    \fn QGLColormap & QGLWidget::colormap() const

    Returns the colormap for this widget.

    Usually it is only top-level widgets that can have different
    colormaps installed. Asking for the colormap of a child widget
    will return the colormap for the child's top-level widget.

    If no colormap has been set for this widget, the QGLColormap
    returned will be empty.

    \sa setColormap(), QGLColormap::isEmpty()
*/

/*!
    \fn void QGLWidget::setColormap(const QGLColormap & cmap)

    Set the colormap for this widget to \a cmap. Usually it is only
    top-level widgets that can have colormaps installed.

    \sa colormap()
*/


/*!
    \obsolete

    Returns the value of the first display list that is generated for
    the characters in the given \a font. \a listBase indicates the base
    value used when generating the display lists for the font. The
    default value is 2000.

    \note This function is not supported on OpenGL/ES systems.
*/
int QGLWidget::fontDisplayListBase(const QFont &font, int listBase)
{
#ifndef QT_OPENGL_ES
   Q_D(QGLWidget);
   int base;

   if (! d->glcx) {
      // this can not happen unless we run out of mem
      return 0;
   }

   // always regenerate font disp. lists for pixmaps - hw accelerated
   // contexts can't handle this otherwise
   bool regenerate = d->glcx->deviceIsPixmap();

#ifndef QT_NO_FONTCONFIG
   // font color needs to be part of the font cache key when using antialiased fonts since one
   // set of glyphs needs to be generated for each font color

   QString color_key;

   if (font.styleStrategy() != QFont::NoAntialias) {
      GLfloat color[4];
      glGetFloatv(GL_CURRENT_COLOR, color);

      color_key = QString("%1_%2_%3").formatArg(color[0], 0, 'f').formatArg(color[1], 0, 'f').formatArg(color[2], 0, 'f');
   }

   QString key = font.key() + color_key + QString::number((int) regenerate);

#else
   QString key = font.key() + QString::number((int) regenerate);
#endif

   if (! regenerate && (d->displayListCache.find(key) != d->displayListCache.end())) {
      base = d->displayListCache[key];

   } else {
      int maxBase = listBase - 256;
      QMap<QString, int>::const_iterator it;

      for (it = d->displayListCache.constBegin(); it != d->displayListCache.constEnd(); ++it) {
         if (maxBase < it.value()) {
            maxBase = it.value();
         }
      }

      maxBase += 256;
      d->glcx->generateFontDisplayLists(font, maxBase);
      d->displayListCache[key] = maxBase;
      base = maxBase;
   }

   return base;
#else // QT_OPENGL_ES
   Q_UNUSED(font);
   Q_UNUSED(listBase);
   return 0;
#endif
}

#ifndef QT_OPENGL_ES

static void qt_save_gl_state()
{
   glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
   glPushAttrib(GL_ALL_ATTRIB_BITS);
   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   glShadeModel(GL_FLAT);
   glDisable(GL_CULL_FACE);
   glDisable(GL_LIGHTING);
   glDisable(GL_STENCIL_TEST);
   glDisable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

static void qt_restore_gl_state()
{
   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glPopAttrib();
   glPopClientAttrib();
}

static void qt_gl_draw_text(QPainter *p, int x, int y, const QString &str,
                            const QFont &font)
{
   GLfloat color[4];
   glGetFloatv(GL_CURRENT_COLOR, &color[0]);

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

#endif // !QT_OPENGL_ES

/*!
   Renders the string \a str into the GL context of this widget.

   \a x and \a y are specified in window coordinates, with the origin
   in the upper left-hand corner of the window. If \a font is not
   specified, the currently set application font will be used to
   render the string. To change the color of the rendered text you can
   use the glColor() call (or the qglColor() convenience function),
   just before the renderText() call.

   The \a listBase parameter is obsolete and will be removed in a
   future version of Qt.

   \note This function clears the stencil buffer.

   \note This function is not supported on OpenGL/ES systems.

   \note This function temporarily disables depth-testing when the
   text is drawn.

   \note This function can only be used inside a
   QPainter::beginNativePainting()/QPainter::endNativePainting() block
   if the default OpenGL paint engine is QPaintEngine::OpenGL. To make
   QPaintEngine::OpenGL the default GL engine, call
   QGL::setPreferredPaintEngine(QPaintEngine::OpenGL) before the
   QApplication constructor.

   \l{Overpainting Example}{Overpaint} with QPainter::drawText() instead.
*/

void QGLWidget::renderText(int x, int y, const QString &str, const QFont &font)
{
#ifndef QT_OPENGL_ES
   Q_D(QGLWidget);
   if (str.isEmpty() || !isValid()) {
      return;
   }

   GLint view[4];
   bool use_scissor_testing = glIsEnabled(GL_SCISSOR_TEST);
   if (!use_scissor_testing) {
      glGetIntegerv(GL_VIEWPORT, &view[0]);
   }
   int width = d->glcx->device()->width();
   int height = d->glcx->device()->height();
   bool auto_swap = autoBufferSwap();

   QPaintEngine::Type oldEngineType = qgl_engine_selector()->preferredPaintEngine();

   QPaintEngine *engine = paintEngine();
   if (engine && (oldEngineType == QPaintEngine::OpenGL2) && engine->isActive()) {
      qWarning("QGLWidget::renderText(): Calling renderText() while a GL 2 paint engine is"
               " active on the same device is not allowed.");
      return;
   }

   // this changes what paintEngine() returns
   qgl_engine_selector()->setPreferredPaintEngine(QPaintEngine::OpenGL);
   engine = paintEngine();
   QPainter *p;
   bool reuse_painter = false;
   if (engine->isActive()) {
      reuse_painter = true;
      p = engine->painter();
      qt_save_gl_state();

      glDisable(GL_DEPTH_TEST);
      glViewport(0, 0, width, height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, width, height, 0, 0, 1);
      glMatrixMode(GL_MODELVIEW);

      glLoadIdentity();
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
      glScissor(view[0], view[1], view[2], view[3]);
      glEnable(GL_SCISSOR_TEST);
   } else if (use_scissor_testing) {
      // use the scissor box set by the user
      glEnable(GL_SCISSOR_TEST);
   }

   qt_gl_draw_text(p, x, y, str, font);

   if (reuse_painter) {
      qt_restore_gl_state();
   } else {
      p->end();
      delete p;
      setAutoBufferSwap(auto_swap);
      d->disable_clear_on_painter_begin = false;
   }
   qgl_engine_selector()->setPreferredPaintEngine(oldEngineType);
#else // QT_OPENGL_ES
   Q_UNUSED(x);
   Q_UNUSED(y);
   Q_UNUSED(str);
   Q_UNUSED(font);
   qWarning("QGLWidget::renderText is not supported under OpenGL/ES");
#endif
}

/*! \overload

    \a x, \a y and \a z are specified in scene or object coordinates
    relative to the currently set projection and model matrices. This
    can be useful if you want to annotate models with text labels and
    have the labels move with the model as it is rotated etc.

    \note This function is not supported on OpenGL/ES systems.

    \note If depth testing is enabled before this function is called,
    then the drawn text will be depth-tested against the models that
    have already been drawn in the scene.  Use \c{glDisable(GL_DEPTH_TEST)}
    before calling this function to annotate the models without
    depth-testing the text.

    \l{Overpainting Example}{Overpaint} with QPainter::drawText() instead.
*/
void QGLWidget::renderText(double x, double y, double z, const QString &str, const QFont &font)
{
#ifndef QT_OPENGL_ES
   Q_D(QGLWidget);
   if (str.isEmpty() || !isValid()) {
      return;
   }

   bool auto_swap = autoBufferSwap();

   int width = d->glcx->device()->width();
   int height = d->glcx->device()->height();
   GLdouble model[4][4], proj[4][4];
   GLint view[4];
   glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
   glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
   glGetIntegerv(GL_VIEWPORT, &view[0]);
   GLdouble win_x = 0, win_y = 0, win_z = 0;
   qgluProject(x, y, z, &model[0][0], &proj[0][0], &view[0],
               &win_x, &win_y, &win_z);
   win_y = height - win_y; // y is inverted

   QPaintEngine::Type oldEngineType = qgl_engine_selector()->preferredPaintEngine();
   QPaintEngine *engine = paintEngine();

   if (engine && (oldEngineType == QPaintEngine::OpenGL2) && engine->isActive()) {
      qWarning("QGLWidget::renderText(): Calling renderText() while a GL 2 paint engine is"
               " active on the same device is not allowed.");
      return;
   }

   // this changes what paintEngine() returns
   qgl_engine_selector()->setPreferredPaintEngine(QPaintEngine::OpenGL);
   engine = paintEngine();
   QPainter *p;
   bool reuse_painter = false;
   bool use_depth_testing = glIsEnabled(GL_DEPTH_TEST);
   bool use_scissor_testing = glIsEnabled(GL_SCISSOR_TEST);

   if (engine->isActive()) {
      reuse_painter = true;
      p = engine->painter();
      qt_save_gl_state();
   } else {
      setAutoBufferSwap(false);
      // disable glClear() as a result of QPainter::begin()
      d->disable_clear_on_painter_begin = true;
      p = new QPainter(this);
   }

   QRect viewport(view[0], view[1], view[2], view[3]);
   if (!use_scissor_testing && viewport != rect()) {
      glScissor(view[0], view[1], view[2], view[3]);
      glEnable(GL_SCISSOR_TEST);
   } else if (use_scissor_testing) {
      glEnable(GL_SCISSOR_TEST);
   }
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glViewport(0, 0, width, height);
   glOrtho(0, width, height, 0, 0, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glAlphaFunc(GL_GREATER, 0.0);
   glEnable(GL_ALPHA_TEST);
   if (use_depth_testing) {
      glEnable(GL_DEPTH_TEST);
   }
   glTranslated(0, 0, -win_z);
   qt_gl_draw_text(p, qRound(win_x), qRound(win_y), str, font);

   if (reuse_painter) {
      qt_restore_gl_state();
   } else {
      p->end();
      delete p;
      setAutoBufferSwap(auto_swap);
      d->disable_clear_on_painter_begin = false;
   }
   qgl_engine_selector()->setPreferredPaintEngine(oldEngineType);
#else // QT_OPENGL_ES
   Q_UNUSED(x);
   Q_UNUSED(y);
   Q_UNUSED(z);
   Q_UNUSED(str);
   Q_UNUSED(font);
   qWarning("QGLWidget::renderText is not supported under OpenGL/ES");
#endif
}

QGLFormat QGLWidget::format() const
{
   Q_D(const QGLWidget);
   return d->glcx->format();
}

const QGLContext *QGLWidget::context() const
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

/*!
    Calls QGLContext:::bindTexture(\a image, \a target, \a format) on the currently
    set context.

    \sa deleteTexture()
*/
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


#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
GLuint QGLWidget::bindTexture(const QImage &image, QMacCompatGLenum target, QMacCompatGLint format)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLWidget);
   return d->glcx->bindTexture(image, GLenum(target), GLint(format), QGLContext::DefaultBindOption);
}

GLuint QGLWidget::bindTexture(const QImage &image, QMacCompatGLenum target, QMacCompatGLint format,
                              QGLContext::BindOptions options)
{
   if (image.isNull()) {
      return 0;
   }

   Q_D(QGLWidget);
   return d->glcx->bindTexture(image, GLenum(target), GLint(format), options);
}
#endif

/*!
    Calls QGLContext:::bindTexture(\a pixmap, \a target, \a format) on the currently
    set context.

    \sa deleteTexture()
*/
GLuint QGLWidget::bindTexture(const QPixmap &pixmap, GLenum target, GLint format)
{
   if (pixmap.isNull()) {
      return 0;
   }

   Q_D(QGLWidget);
   return d->glcx->bindTexture(pixmap, target, format, QGLContext::DefaultBindOption);
}

/*!
  \overload
  \since 4.6

  Generates and binds a 2D GL texture to the current context, based
  on \a pixmap. The generated texture id is returned and can be used in

  The binding \a options are a set of options used to decide how to
  bind the texture to the context.
 */
GLuint QGLWidget::bindTexture(const QPixmap &pixmap, GLenum target, GLint format,
                              QGLContext::BindOptions options)
{
   Q_D(QGLWidget);
   return d->glcx->bindTexture(pixmap, target, format, options);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
GLuint QGLWidget::bindTexture(const QPixmap &pixmap, QMacCompatGLenum target, QMacCompatGLint format)
{
   Q_D(QGLWidget);
   return d->glcx->bindTexture(pixmap, target, format, QGLContext::DefaultBindOption);
}

GLuint QGLWidget::bindTexture(const QPixmap &pixmap, QMacCompatGLenum target, QMacCompatGLint format,
                              QGLContext::BindOptions options)
{
   Q_D(QGLWidget);
   return d->glcx->bindTexture(pixmap, target, format, options);
}
#endif


/*! \overload

    Calls QGLContext::bindTexture(\a fileName) on the currently set context.

    \sa deleteTexture()
*/
GLuint QGLWidget::bindTexture(const QString &fileName)
{
   Q_D(QGLWidget);
   return d->glcx->bindTexture(fileName);
}

/*!
    Calls QGLContext::deleteTexture(\a id) on the currently set
    context.

    \sa bindTexture()
*/
void QGLWidget::deleteTexture(GLuint id)
{
   Q_D(QGLWidget);
   d->glcx->deleteTexture(id);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLWidget::deleteTexture(QMacCompatGLuint id)
{
   Q_D(QGLWidget);
   d->glcx->deleteTexture(GLuint(id));
}
#endif

/*!
    \since 4.4

    Calls the corresponding QGLContext::drawTexture() with
    \a target, \a textureId, and \a textureTarget for this
    widget's context.
*/
void QGLWidget::drawTexture(const QRectF &target, GLuint textureId, GLenum textureTarget)
{
   Q_D(QGLWidget);
   d->glcx->drawTexture(target, textureId, textureTarget);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLWidget::drawTexture(const QRectF &target, QMacCompatGLuint textureId, QMacCompatGLenum textureTarget)
{
   Q_D(QGLWidget);
   d->glcx->drawTexture(target, GLint(textureId), GLenum(textureTarget));
}
#endif

/*!
    \since 4.4

    Calls the corresponding QGLContext::drawTexture() with
    \a point, \a textureId, and \a textureTarget for this
    widget's context.
*/
void QGLWidget::drawTexture(const QPointF &point, GLuint textureId, GLenum textureTarget)
{
   Q_D(QGLWidget);
   d->glcx->drawTexture(point, textureId, textureTarget);
}

#ifdef Q_MAC_COMPAT_GL_FUNCTIONS
/*! \internal */
void QGLWidget::drawTexture(const QPointF &point, QMacCompatGLuint textureId, QMacCompatGLenum textureTarget)
{
   Q_D(QGLWidget);
   d->glcx->drawTexture(point, GLuint(textureId), GLenum(textureTarget));
}
#endif

#ifndef QT_OPENGL_ES_1
Q_GLOBAL_STATIC(QGLEngineThreadStorage<QGL2PaintEngineEx>, qt_gl_2_engine)
#endif

#ifndef QT_OPENGL_ES_2
Q_GLOBAL_STATIC(QGLEngineThreadStorage<QOpenGLPaintEngine>, qt_gl_engine)
#endif

Q_OPENGL_EXPORT QPaintEngine *qt_qgl_paint_engine()
{
#if defined(QT_OPENGL_ES_1)
   return qt_gl_engine()->engine();
#elif defined(QT_OPENGL_ES_2)
   return qt_gl_2_engine()->engine();
#else
   if (qt_gl_preferGL2Engine()) {
      return qt_gl_2_engine()->engine();
   } else {
      return qt_gl_engine()->engine();
   }
#endif
}

QPaintEngine *QGLWidget::paintEngine() const
{
   return qt_qgl_paint_engine();
}

QGLExtensionMatcher::QGLExtensionMatcher(const QString &str)
{
   init(str);
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

QString cs_glGetStringI(GLenum data, GLuint index)
{
   static qt_glGetStringi funcPtr = nullptr;

   if (funcPtr == nullptr) {
      const QGLContext *ctx = QGLContext::currentContext();

      if (ctx == nullptr) {
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

QGLExtensionMatcher::QGLExtensionMatcher()
{
   const QString &extensionStr = cs_glGetString(GL_EXTENSIONS);

   if (! extensionStr.isEmpty()) {
      init(extensionStr);

   } else {
      // clear error state
      while (glGetError()) {}

      GLint numExtensions;
      glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

      for (int i = 0; i < numExtensions; ++i) {
         const QString &extStr = cs_glGetStringI(GL_EXTENSIONS, i);

         if (! extStr.isEmpty()) {
            m_extensions.append(extStr);
         }
      }
   }
}

void QGLExtensionMatcher::init(const QString &str)
{
   m_extensions = str.split(' ');
}

QGLExtensions::Extensions QGLExtensions::currentContextExtensions()
{
   QGLExtensionMatcher extensions;
   Extensions glExtensions;

   if (extensions.match("GL_ARB_texture_rectangle")) {
      glExtensions |= TextureRectangle;
   }

   if (extensions.match("GL_ARB_multisample")) {
      glExtensions |= SampleBuffers;
   }

   if (extensions.match("GL_SGIS_generate_mipmap")) {
      glExtensions |= GenerateMipmap;
   }

   if (extensions.match("GL_ARB_texture_compression")) {
      glExtensions |= TextureCompression;
   }

   if (extensions.match("GL_EXT_texture_compression_s3tc")) {
      glExtensions |= DDSTextureCompression;
   }

   if (extensions.match("GL_OES_compressed_ETC1_RGB8_texture")) {
      glExtensions |= ETC1TextureCompression;
   }

   if (extensions.match("GL_IMG_texture_compression_pvrtc")) {
      glExtensions |= PVRTCTextureCompression;
   }

   if (extensions.match("GL_ARB_fragment_program")) {
      glExtensions |= FragmentProgram;
   }

   if (extensions.match("GL_ARB_fragment_shader")) {
      glExtensions |= FragmentShader;
   }

   if (extensions.match("GL_ARB_shader_objects")) {
      glExtensions |= FragmentShader;
   }

   if (extensions.match("GL_ARB_texture_mirrored_repeat")) {
      glExtensions |= MirroredRepeat;
   }

   if (extensions.match("GL_EXT_framebuffer_object")) {
      glExtensions |= FramebufferObject;
   }

   if (extensions.match("GL_EXT_stencil_two_side")) {
      glExtensions |= StencilTwoSide;
   }

   if (extensions.match("GL_EXT_stencil_wrap")) {
      glExtensions |= StencilWrap;
   }

   if (extensions.match("GL_EXT_packed_depth_stencil")) {
      glExtensions |= PackedDepthStencil;
   }

   if (extensions.match("GL_NV_float_buffer")) {
      glExtensions |= NVFloatBuffer;
   }

   if (extensions.match("GL_ARB_pixel_buffer_object")) {
      glExtensions |= PixelBufferObject;
   }

   if (extensions.match("GL_IMG_texture_format_BGRA8888")
         || extensions.match("GL_EXT_texture_format_BGRA8888")) {
      glExtensions |= BGRATextureFormat;
   }

#if defined(QT_OPENGL_ES_2)
   glExtensions |= FramebufferObject;
   glExtensions |= GenerateMipmap;
   glExtensions |= FragmentShader;
#endif

#if defined(QT_OPENGL_ES_1)
   if (extensions.match("GL_OES_framebuffer_object")) {
      glExtensions |= FramebufferObject;
   }
#endif

#if defined(QT_OPENGL_ES)
   if (extensions.match("GL_OES_packed_depth_stencil")) {
      glExtensions |= PackedDepthStencil;
   }
   if (extensions.match("GL_OES_element_index_uint")) {
      glExtensions |= ElementIndexUint;
   }
   if (extensions.match("GL_OES_depth24")) {
      glExtensions |= Depth24;
   }
#else
   glExtensions |= ElementIndexUint;
#endif

   if (extensions.match("GL_ARB_framebuffer_object")) {
      // ARB_framebuffer_object also includes EXT_framebuffer_blit.
      glExtensions |= FramebufferObject;
      glExtensions |= FramebufferBlit;
   }

   if (extensions.match("GL_EXT_framebuffer_blit")) {
      glExtensions |= FramebufferBlit;
   }

   if (extensions.match("GL_ARB_texture_non_power_of_two")) {
      glExtensions |= NPOTTextures;
   }

   if (extensions.match("GL_EXT_bgra")) {
      glExtensions |= BGRATextureFormat;
   }

   {
      GLboolean srgbCapableFramebuffers = false;
      glGetBooleanv(FRAMEBUFFER_SRGB_CAPABLE_EXT, &srgbCapableFramebuffers);
      if (srgbCapableFramebuffers) {
         glExtensions |= SRGBFrameBuffer;
      }
      // Clear possible error which is generated if
      // FRAMEBUFFER_SRGB_CAPABLE_EXT isn't supported.
      glGetError();
   }

   return glExtensions;
}


class QGLDefaultExtensions
{
 public:
   QGLDefaultExtensions() {
      QGLTemporaryContext tempContext;
      extensions = QGLExtensions::currentContextExtensions();
   }

   QGLExtensions::Extensions extensions;
};

Q_GLOBAL_STATIC(QGLDefaultExtensions, qtDefaultExtensions)

/*
    Returns the GL extensions for the current QGLContext. If there is no
    current QGLContext, a default context will be created and the extensions
    for that context will be returned instead.
*/
QGLExtensions::Extensions QGLExtensions::glExtensions()
{
   Extensions extensionFlags = 0;
   QGLContext *currentCtx = const_cast<QGLContext *>(QGLContext::currentContext());

   if (currentCtx && currentCtx->d_func()->extension_flags_cached) {
      return currentCtx->d_func()->extension_flags;
   }

   if (!currentCtx) {
      extensionFlags = qtDefaultExtensions()->extensions;
   } else {
      extensionFlags = currentContextExtensions();
      currentCtx->d_func()->extension_flags_cached = true;
      currentCtx->d_func()->extension_flags = extensionFlags;
   }
   return extensionFlags;
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

#if defined(Q_WS_X11) || defined(Q_OS_MAC) || defined(Q_WS_QWS) || defined(Q_WS_QPA)
Q_GLOBAL_STATIC(QString, qt_gl_lib_name)

Q_OPENGL_EXPORT void qt_set_gl_library_name(const QString &name)
{
   qt_gl_lib_name()->operator=(name);
}

Q_OPENGL_EXPORT const QString qt_gl_library_name()
{
   if (qt_gl_lib_name()->isEmpty()) {

#ifdef Q_OS_DARWIN
      // BROOM (can wait) consider changing the path
      return "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib";
#else

# if defined(QT_OPENGL_ES_1)
      return QLatin1String("GLES_CM");

# elif defined(QT_OPENGL_ES_2)
      return QLatin1String("GLESv2");

# else
      return QLatin1String("GL");
# endif

#endif

   }
   return *qt_gl_lib_name();
}
#endif

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

QGLContextGroupResourceBase::QGLContextGroupResourceBase()
   : active(0)
{
#ifdef QT_GL_CONTEXT_RESOURCE_DEBUG
   qDebug("Creating context group resource object %p.", this);
#endif
}

QGLContextGroupResourceBase::~QGLContextGroupResourceBase()
{
#ifdef QT_GL_CONTEXT_RESOURCE_DEBUG
   qDebug("Deleting context group resource %p. Group size: %d.", this, m_groups.size());
#endif
   for (int i = 0; i < m_groups.size(); ++i) {
      m_groups.at(i)->m_resources.remove(this);
      active.deref();
   }
#ifndef QT_NO_DEBUG
   if (active.load() != 0) {
      qWarning("QtOpenGL: Resources are still available at program shutdown.\n"
               "          This is possibly caused by a leaked QGLWidget, \n"
               "          QGLFramebufferObject or QGLPixelBuffer.");
   }
#endif
}

void QGLContextGroupResourceBase::insert(const QGLContext *context, void *value)
{
#ifdef QT_GL_CONTEXT_RESOURCE_DEBUG
   qDebug("Inserting context group resource %p for context %p, managed by %p.", value, context, this);
#endif
   QGLContextGroup *group = QGLContextPrivate::contextGroup(context);
   Q_ASSERT(!group->m_resources.contains(this));
   group->m_resources.insert(this, value);
   m_groups.append(group);
   active.ref();
}

void *QGLContextGroupResourceBase::value(const QGLContext *context)
{
   QGLContextGroup *group = QGLContextPrivate::contextGroup(context);
   return group->m_resources.value(this, 0);
}

void QGLContextGroupResourceBase::cleanup(const QGLContext *ctx)
{
   void *resource = value(ctx);

   if (resource != 0) {
      QGLShareContextScope scope(ctx);
      freeResource(resource);

      QGLContextGroup *group = QGLContextPrivate::contextGroup(ctx);
      group->m_resources.remove(this);
      m_groups.removeOne(group);
      active.deref();
   }
}

void QGLContextGroupResourceBase::contextDeleted(const QGLContext *ctx)
{
   Q_UNUSED(ctx);
}

void QGLContextGroupResourceBase::cleanup(const QGLContext *ctx, void *value)
{
#ifdef QT_GL_CONTEXT_RESOURCE_DEBUG
   qDebug("Cleaning up context group resource %p, for context %p in thread %p.", this, ctx, QThread::currentThread());
#endif
   QGLShareContextScope scope(ctx);
   freeResource(value);
   active.deref();

   QGLContextGroup *group = QGLContextPrivate::contextGroup(ctx);
   m_groups.removeOne(group);
}

void QGLContextGroup::cleanupResources(const QGLContext *context)
{
   // Notify all resources that a context has been deleted
   QHash<QGLContextGroupResourceBase *, void *>::const_iterator it;
   for (it = m_resources.begin(); it != m_resources.end(); ++it) {
      it.key()->contextDeleted(context);
   }

   // If there are still shares, then no cleanup to be done yet.
   if (m_shares.size() > 1) {
      return;
   }

   // Iterate over all resources and free each in turn.
   for (it = m_resources.begin(); it != m_resources.end(); ++it) {
      it.key()->cleanup(context, it.value());
   }
}

QGLSharedResourceGuard::~QGLSharedResourceGuard()
{
   if (m_group) {
      m_group->removeGuard(this);
   }
}

void QGLSharedResourceGuard::setContext(const QGLContext *context)
{
   if (m_group) {
      m_group->removeGuard(this);
   }
   if (context) {
      m_group = QGLContextPrivate::contextGroup(context);
      m_group->addGuard(this);
   } else {
      m_group = 0;
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

QSize QGLTexture::bindCompressedTexture
(const char *buf, int len, const char *format)
{
   if (QSysInfo::ByteOrder != QSysInfo::LittleEndian) {
      // Compressed texture loading only supported on little-endian
      // systems such as x86 and ARM at the moment.
      return QSize();
   }
#if !defined(QT_OPENGL_ES)
   if (!glCompressedTexImage2D) {
      if (!(QGLExtensions::glExtensions() & QGLExtensions::TextureCompression)) {
         qWarning("QGLContext::bindTexture(): The GL implementation does "
                  "not support texture compression extensions.");
         return QSize();
      }
      glCompressedTexImage2D = (_glCompressedTexImage2DARB) ctx->getProcAddress(QLatin1String("glCompressedTexImage2DARB"));
      if (!glCompressedTexImage2D) {
         qWarning("QGLContext::bindTexture(): could not resolve "
                  "glCompressedTexImage2DARB.");
         return QSize();
      }
   }
#endif
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
   // We only support 2D texture loading at present.
   if (target != GL_TEXTURE_2D) {
      return QSize();
   }

   // Bail out if the necessary extension is not present.
   if (!(QGLExtensions::glExtensions() & QGLExtensions::DDSTextureCompression)) {
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

   glGenTextures(1, &id);
   glBindTexture(GL_TEXTURE_2D, id);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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
      glCompressedTexImage2D(GL_TEXTURE_2D, i, format, w, h, 0,
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
      if (!(QGLExtensions::glExtensions() &
            QGLExtensions::ETC1TextureCompression)) {
         qWarning("QGLContext::bindTexture(): ETC1 texture compression is not supported.");
         return QSize();
      }
   } else {
      if (!(QGLExtensions::glExtensions() &
            QGLExtensions::PVRTCTextureCompression)) {
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
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glGenTextures(1, &id);
   glBindTexture(GL_TEXTURE_2D, id);
   if (pvrHeader->mipMapCount) {
      if ((options & QGLContext::LinearFilteringBindOption) != 0) {
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      } else {
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
      }
   } else if ((options & QGLContext::LinearFilteringBindOption) != 0) {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   } else {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
      glCompressedTexImage2D(GL_TEXTURE_2D, GLint(level), textureFormat,
                             GLsizei(width), GLsizei(height), 0,
                             GLsizei(size), buffer);
      width /= 2;
      height /= 2;
      buffer += size;
      ++level;
   }

   // Restore the default pixel alignment for later texture uploads.
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

   // Set the invert flag for the texture.  The "vertical flip"
   // flag in PVR is the opposite sense to our sense of inversion.
   if ((pvrHeader->flags & PVR_VERTICAL_FLIP) != 0) {
      options &= ~QGLContext::InvertedYBindOption;
   } else {
      options |= QGLContext::InvertedYBindOption;
   }

   return QSize(pvrHeader->width, pvrHeader->height);
}
#undef ctx

void QGLTextureDestroyer::freeTexture_slot(QGLContext *context, QPixmapData *boundPixmap, GLuint id)
{
   Q_UNUSED(boundPixmap);

#if defined(Q_WS_X11)
   if (boundPixmap) {
      QGLContext *oldContext = const_cast<QGLContext *>(QGLContext::currentContext());
      context->makeCurrent();

      // Although glXReleaseTexImage is a glX call, it must be called while there
      // is a current context - the context the pixmap was bound to a texture in.
      // Otherwise the release doesn't do anything and you get BadDrawable errors
      // when you come to delete the context.

      QGLContextPrivate::unbindPixmapFromTexture(boundPixmap);
      glDeleteTextures(1, &id);

      if (oldContext) {
         oldContext->makeCurrent();
      }
      return;
   }
#endif

   QGLShareContextScope scope(context);
   glDeleteTextures(1, &id);
}

