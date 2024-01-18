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

#include <qsurfaceformat.h>

#include <qatomic.h>
#include <qdebug.h>
#include <QOpenGLContext>
#include <qguiapplication.h>

#ifdef major
#undef major
#endif

#ifdef minor
#undef minor
#endif

class QSurfaceFormatPrivate
{
 public:
   explicit QSurfaceFormatPrivate(QSurfaceFormat::FormatOptions _opts = Qt::EmptyFlag)
      : ref(1)
      , opts(_opts)
      , redBufferSize(-1)
      , greenBufferSize(-1)
      , blueBufferSize(-1)
      , alphaBufferSize(-1)
      , depthSize(-1)
      , stencilSize(-1)
      , swapBehavior(QSurfaceFormat::DefaultSwapBehavior)
      , numSamples(-1)
      , renderableType(QSurfaceFormat::DefaultRenderableType)
      , profile(QSurfaceFormat::NoProfile)
      , major(2)
      , minor(0)
      , swapInterval(1) { // default to vsync
   }

   QSurfaceFormatPrivate(const QSurfaceFormatPrivate *other)
      : ref(1),
        opts(other->opts),
        redBufferSize(other->redBufferSize),
        greenBufferSize(other->greenBufferSize),
        blueBufferSize(other->blueBufferSize),
        alphaBufferSize(other->alphaBufferSize),
        depthSize(other->depthSize),
        stencilSize(other->stencilSize),
        swapBehavior(other->swapBehavior),
        numSamples(other->numSamples),
        renderableType(other->renderableType),
        profile(other->profile),
        major(other->major),
        minor(other->minor),
        swapInterval(other->swapInterval) {
   }

   QAtomicInt ref;
   QSurfaceFormat::FormatOptions opts;
   int redBufferSize;
   int greenBufferSize;
   int blueBufferSize;
   int alphaBufferSize;
   int depthSize;
   int stencilSize;
   QSurfaceFormat::SwapBehavior swapBehavior;
   int numSamples;
   QSurfaceFormat::RenderableType renderableType;
   QSurfaceFormat::OpenGLContextProfile profile;
   int major;
   int minor;
   int swapInterval;
};

QSurfaceFormat::QSurfaceFormat() : d(new QSurfaceFormatPrivate)
{
}


QSurfaceFormat::QSurfaceFormat(QSurfaceFormat::FormatOptions options) :
   d(new QSurfaceFormatPrivate(options))
{
}

/*!
    \internal
*/
void QSurfaceFormat::detach()
{
   if (d->ref.load() != 1) {
      QSurfaceFormatPrivate *newd = new QSurfaceFormatPrivate(d);
      if (!d->ref.deref()) {
         delete d;
      }
      d = newd;
   }
}

QSurfaceFormat::QSurfaceFormat(const QSurfaceFormat &other)
{
   d = other.d;
   d->ref.ref();
}

QSurfaceFormat &QSurfaceFormat::operator=(const QSurfaceFormat &other)
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

QSurfaceFormat::~QSurfaceFormat()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

void QSurfaceFormat::setStereo(bool enable)
{
   QSurfaceFormat::FormatOptions newOptions = d->opts;
   if (enable) {
      newOptions |= QSurfaceFormat::StereoBuffers;
   } else {
      newOptions &= ~QSurfaceFormat::StereoBuffers;
   }
   if (int(newOptions) != int(d->opts)) {
      detach();
      d->opts = newOptions;
   }
}

int QSurfaceFormat::samples() const
{
   return d->numSamples;
}

void QSurfaceFormat::setSamples(int numSamples)
{
   if (d->numSamples != numSamples) {
      detach();
      d->numSamples = numSamples;
   }
}

void QSurfaceFormat::setOptions(QSurfaceFormat::FormatOptions options)
{
   if (int(d->opts) != int(options)) {
      detach();
      d->opts = options;
   }
}

void QSurfaceFormat::setOption(QSurfaceFormat::FormatOption option, bool on)
{
   if (testOption(option) == on) {
      return;
   }
   detach();
   if (on) {
      d->opts |= option;
   } else {
      d->opts &= ~option;
   }
}

bool QSurfaceFormat::testOption(QSurfaceFormat::FormatOption option) const
{
   return d->opts & option;
}

QSurfaceFormat::FormatOptions QSurfaceFormat::options() const
{
   return d->opts;
}

void QSurfaceFormat::setDepthBufferSize(int size)
{
   if (d->depthSize != size) {
      detach();
      d->depthSize = size;
   }
}

int QSurfaceFormat::depthBufferSize() const
{
   return d->depthSize;
}

void QSurfaceFormat::setSwapBehavior(SwapBehavior behavior)
{
   if (d->swapBehavior != behavior) {
      detach();
      d->swapBehavior = behavior;
   }
}

QSurfaceFormat::SwapBehavior QSurfaceFormat::swapBehavior() const
{
   return d->swapBehavior;
}

bool QSurfaceFormat::hasAlpha() const
{
   return d->alphaBufferSize > 0;
}

void QSurfaceFormat::setStencilBufferSize(int size)
{
   if (d->stencilSize != size) {
      detach();
      d->stencilSize = size;
   }
}

int QSurfaceFormat::stencilBufferSize() const
{
   return d->stencilSize;
}

int QSurfaceFormat::redBufferSize() const
{
   return d->redBufferSize;
}

int QSurfaceFormat::greenBufferSize() const
{
   return d->greenBufferSize;
}

int QSurfaceFormat::blueBufferSize() const
{
   return d->blueBufferSize;
}

int QSurfaceFormat::alphaBufferSize() const
{
   return d->alphaBufferSize;
}

void QSurfaceFormat::setRedBufferSize(int size)
{
   if (d->redBufferSize != size) {
      detach();
      d->redBufferSize = size;
   }
}

void QSurfaceFormat::setGreenBufferSize(int size)
{
   if (d->greenBufferSize != size) {
      detach();
      d->greenBufferSize = size;
   }
}

void QSurfaceFormat::setBlueBufferSize(int size)
{
   if (d->blueBufferSize != size) {
      detach();
      d->blueBufferSize = size;
   }
}

void QSurfaceFormat::setAlphaBufferSize(int size)
{
   if (d->alphaBufferSize != size) {
      detach();
      d->alphaBufferSize = size;
   }
}

void QSurfaceFormat::setRenderableType(RenderableType type)
{
   if (d->renderableType != type) {
      detach();
      d->renderableType = type;
   }
}

QSurfaceFormat::RenderableType QSurfaceFormat::renderableType() const
{
   return d->renderableType;
}

void QSurfaceFormat::setProfile(OpenGLContextProfile profile)
{
   if (d->profile != profile) {
      detach();
      d->profile = profile;
   }
}

QSurfaceFormat::OpenGLContextProfile QSurfaceFormat::profile() const
{
   return d->profile;
}

/*!
    Sets the desired \a major OpenGL version.
*/
void QSurfaceFormat::setMajorVersion(int major)
{
   if (d->major != major) {
      detach();
      d->major = major;
   }
}

/*!
    Returns the major OpenGL version.

    The default version is 2.0.
*/
int QSurfaceFormat::majorVersion() const
{
   return d->major;
}

/*!
    Sets the desired \a minor OpenGL version.

    The default version is 2.0.
*/
void QSurfaceFormat::setMinorVersion(int minor)
{
   if (d->minor != minor) {
      detach();
      d->minor = minor;
   }
}

/*!
    Returns the minor OpenGL version.
*/
int QSurfaceFormat::minorVersion() const
{
   return d->minor;
}

/*!
    Returns a QPair<int, int> representing the OpenGL version.

    Useful for version checks, for example format.version() >= qMakePair(3, 2)
*/
QPair<int, int> QSurfaceFormat::version() const
{
   return qMakePair(d->major, d->minor);
}

/*!
    Sets the desired \a major and \a minor OpenGL versions.

    The default version is 2.0.
*/
void QSurfaceFormat::setVersion(int major, int minor)
{
   if (d->minor != minor || d->major != major) {
      detach();
      d->minor = minor;
      d->major = major;
   }
}

void QSurfaceFormat::setSwapInterval(int interval)
{
   if (d->swapInterval != interval) {
      detach();
      d->swapInterval = interval;
   }
}

int QSurfaceFormat::swapInterval() const
{
   return d->swapInterval;
}

static QSurfaceFormat *qt_default_surface_format()
{
   static QSurfaceFormat retval;
   return &retval;
}

void QSurfaceFormat::setDefaultFormat(const QSurfaceFormat &format)
{
#ifndef QT_NO_OPENGL
   if (qApp) {
      QOpenGLContext *globalContext = QOpenGLContext::globalShareContext();

      if (globalContext && globalContext->isValid()) {
         qWarning("QSurfaceFormat::setDefaultFormat() Setting a new default format with a different version or profile, "
            "after the global shared context is created, may cause issues with context sharing");
      }
   }
#endif

   *qt_default_surface_format() = format;
}

QSurfaceFormat QSurfaceFormat::defaultFormat()
{
   return *qt_default_surface_format();
}

bool operator==(const QSurfaceFormat &a, const QSurfaceFormat &b)
{
   return (a.d == b.d) || ((int) a.d->opts == (int) b.d->opts
         && a.d->stencilSize == b.d->stencilSize
         && a.d->redBufferSize == b.d->redBufferSize
         && a.d->greenBufferSize == b.d->greenBufferSize
         && a.d->blueBufferSize == b.d->blueBufferSize
         && a.d->alphaBufferSize == b.d->alphaBufferSize
         && a.d->depthSize == b.d->depthSize
         && a.d->numSamples == b.d->numSamples
         && a.d->swapBehavior == b.d->swapBehavior
         && a.d->profile == b.d->profile
         && a.d->major == b.d->major
         && a.d->minor == b.d->minor
         && a.d->swapInterval == b.d->swapInterval);
}

bool operator!=(const QSurfaceFormat &a, const QSurfaceFormat &b)
{
   return !(a == b);
}

QDebug operator<<(QDebug dbg, const QSurfaceFormat &f)
{
   const QSurfaceFormatPrivate *const d = f.d;

   QDebugStateSaver saver(dbg);

   dbg.nospace() << "QSurfaceFormat("
      << "version " << d->major << '.' << d->minor
      << ", options " << d->opts
      << ", depthBufferSize " << d->depthSize
      << ", redBufferSize " << d->redBufferSize
      << ", greenBufferSize " << d->greenBufferSize
      << ", blueBufferSize " << d->blueBufferSize
      << ", alphaBufferSize " << d->alphaBufferSize
      << ", stencilBufferSize " << d->stencilSize
      << ", samples " << d->numSamples
      << ", swapBehavior " << d->swapBehavior
      << ", swapInterval " << d->swapInterval
      << ", profile  " << d->profile
      << ')';

   return dbg;
}

