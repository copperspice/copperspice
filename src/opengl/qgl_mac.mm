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

#include "qgl.h"

#if defined(Q_OS_MAC)

#include <qcocoaview_mac_p.h>
#include <OpenGL/gl.h>
#include <CoreServices/CoreServices.h>
#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qgl_p.h>
#include <qpaintengine_opengl_p.h>
#include <qt_mac_p.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qstack.h>
#include <qdesktopwidget.h>
#include <qdebug.h>

QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QWidgetPrivate)
QT_FORWARD_DECLARE_CLASS(QGLWidgetPrivate)

QT_BEGIN_NAMESPACE

void *qt_current_nsopengl_context()
{
   return [NSOpenGLContext currentContext];
}

static GLint attribValue(NSOpenGLPixelFormat *fmt, NSOpenGLPixelFormatAttribute attrib)
{
   GLint res;
   [fmt getValues: &res forAttribute: attrib forVirtualScreen: 0];
   return res;
}

static int def(int val, int defVal)
{
   return val != -1 ? val : defVal;
}

extern quint32 *qt_mac_pixmap_get_base(const QPixmap *);
extern int qt_mac_pixmap_get_bytes_per_line(const QPixmap *);
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle); //qregion_mac.cpp

/*
    QGLTemporaryContext implementation
*/

class QGLTemporaryContextPrivate
{
 public:
   NSOpenGLContext *ctx;

};

QGLTemporaryContext::QGLTemporaryContext(bool, QWidget *)
   : d(new QGLTemporaryContextPrivate)
{
   d->ctx = 0;

   QMacCocoaAutoReleasePool pool;
   NSOpenGLPixelFormatAttribute attribs[] = { 0 };
   NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];
   if (!fmt) {
      qWarning("QGLTemporaryContext: Cannot find any visuals");
      return;
   }

   d->ctx = [[NSOpenGLContext alloc] initWithFormat: fmt shareContext: 0];
   if (!d->ctx) {
      qWarning("QGLTemporaryContext: Cannot create context");
   } else {
      [d->ctx makeCurrentContext];
   }

   [fmt release];
}

QGLTemporaryContext::~QGLTemporaryContext()
{
   if (d->ctx) {
      [NSOpenGLContext clearCurrentContext];
      [d->ctx release];
   }
}

bool QGLFormat::hasOpenGL()
{
   return true;
}

bool QGLFormat::hasOpenGLOverlays()
{
   return false;
}

bool QGLContext::chooseContext(const QGLContext *shareContext)
{
   QMacCocoaAutoReleasePool pool;
   Q_D(QGLContext);
   d->cx = 0;
   d->vi = chooseMacVisual(0);
   if (!d->vi) {
      return false;
   }


   NSOpenGLPixelFormat *fmt = static_cast<NSOpenGLPixelFormat *>(d->vi);

   d->glFormat = QGLFormat();

   // ### make sure to reset other options
   d->glFormat.setDoubleBuffer(attribValue(fmt, NSOpenGLPFADoubleBuffer));

   int depthSize = attribValue(fmt, NSOpenGLPFADepthSize);
   d->glFormat.setDepth(depthSize > 0);
   if (depthSize > 0) {
      d->glFormat.setDepthBufferSize(depthSize);
   }

   int alphaSize = attribValue(fmt, NSOpenGLPFAAlphaSize);
   d->glFormat.setAlpha(alphaSize > 0);
   if (alphaSize > 0) {
      d->glFormat.setAlphaBufferSize(alphaSize);
   }

   int accumSize = attribValue(fmt, NSOpenGLPFAAccumSize);
   d->glFormat.setAccum(accumSize > 0);
   if (accumSize > 0) {
      d->glFormat.setAccumBufferSize(accumSize);
   }

   int stencilSize = attribValue(fmt, NSOpenGLPFAStencilSize);
   d->glFormat.setStencil(stencilSize > 0);
   if (stencilSize > 0) {
      d->glFormat.setStencilBufferSize(stencilSize);
   }

   d->glFormat.setStereo(attribValue(fmt, NSOpenGLPFAStereo));

   int sampleBuffers = attribValue(fmt, NSOpenGLPFASampleBuffers);
   d->glFormat.setSampleBuffers(sampleBuffers);
   if (sampleBuffers > 0) {
      d->glFormat.setSamples(attribValue(fmt, NSOpenGLPFASamples));
   }

   if (shareContext && (!shareContext->isValid() || !shareContext->d_func()->cx)) {
      qWarning("QGLContext::chooseContext: Cannot share with invalid context");
      shareContext = 0;
   }

   // sharing between rgba and color-index will give wrong colors
   if (shareContext && (format().rgba() != shareContext->format().rgba())) {
      shareContext = 0;
   }


   NSOpenGLContext *ctx = [[NSOpenGLContext alloc] initWithFormat: fmt
                           shareContext: (shareContext ? static_cast<NSOpenGLContext *>(shareContext->d_func()->cx)
                                          : 0)];

   if (!ctx) {

      if (shareContext) {
         ctx = [[NSOpenGLContext alloc] initWithFormat: fmt shareContext: 0];
         if (ctx) {
            qWarning("QGLContext::chooseContext: Context sharing mismatch");
            shareContext = 0;
         }
      }

      if (!ctx) {
         qWarning("QGLContext::chooseContext: Unable to create QGLContext");
         return false;
      }
   }
   d->cx = ctx;
   if (shareContext && shareContext->d_func()->cx) {
      QGLContext *share = const_cast<QGLContext *>(shareContext);
      d->sharing = true;
      share->d_func()->sharing = true;
   }
   if (deviceIsPixmap()) {
      updatePaintDevice();
   }

   // vblank syncing
   GLint interval = d->reqFormat.swapInterval();
   if (interval != -1) {

      [ctx setValues: &interval forParameter: NSOpenGLCPSwapInterval];

   }

   [ctx getValues: &interval forParameter: NSOpenGLCPSwapInterval];

   d->glFormat.setSwapInterval(interval);
   return true;
}

void *QGLContextPrivate::tryFormat(const QGLFormat &format)
{
   static const int Max = 40;

   NSOpenGLPixelFormatAttribute attribs[Max];
   int cnt = 0;
   int devType = paintDevice->devType();
   bool device_is_pixmap = (devType == QInternal::Pixmap);
   int depth = device_is_pixmap ? static_cast<QPixmap *>(paintDevice)->depth() : 32;

   attribs[cnt++] = NSOpenGLPFAColorSize;
   attribs[cnt++] = depth;

   if (device_is_pixmap) {
      attribs[cnt++] = NSOpenGLPFAOffScreen;
   } else {
      if (format.doubleBuffer()) {
         attribs[cnt++] = NSOpenGLPFADoubleBuffer;
      }
   }
   if (glFormat.stereo()) {
      attribs[cnt++] = NSOpenGLPFAStereo;
   }
   if (device_is_pixmap || format.alpha()) {
      attribs[cnt++] = NSOpenGLPFAAlphaSize;
      attribs[cnt++] = def(format.alphaBufferSize(), 8);
   }
   if (format.stencil()) {
      attribs[cnt++] = NSOpenGLPFAStencilSize;
      attribs[cnt++] = def(format.stencilBufferSize(), 8);
   }
   if (format.depth()) {
      attribs[cnt++] = NSOpenGLPFADepthSize;
      attribs[cnt++] = def(format.depthBufferSize(), 32);
   }
   if (format.accum()) {
      attribs[cnt++] = NSOpenGLPFAAccumSize;
      attribs[cnt++] = def(format.accumBufferSize(), 1);
   }
   if (format.sampleBuffers()) {
      attribs[cnt++] = NSOpenGLPFASampleBuffers;
      attribs[cnt++] = 1;
      attribs[cnt++] = NSOpenGLPFASamples;
      attribs[cnt++] = def(format.samples(), 4);
   }

   if (devType == QInternal::Pbuffer) {
      attribs[cnt++] = NSOpenGLPFAPixelBuffer;
   }

   attribs[cnt] = 0;
   Q_ASSERT(cnt < Max);
   return [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

}

void QGLContextPrivate::clearDrawable()
{
   [static_cast<NSOpenGLContext *>(cx) clearDrawable];
}

void *QGLContext::chooseMacVisual(GDHandle /* handle */)
{
   Q_D(QGLContext);

   void *fmt = d->tryFormat(d->glFormat);
   if (!fmt && d->glFormat.stereo()) {
      d->glFormat.setStereo(false);
      fmt = d->tryFormat(d->glFormat);
   }
   if (!fmt && d->glFormat.sampleBuffers()) {
      d->glFormat.setSampleBuffers(false);
      fmt = d->tryFormat(d->glFormat);
   }
   if (!fmt) {
      qWarning("QGLContext::chooseMacVisual: Unable to choose a pixel format");
   }
   return fmt;
}

void QGLContext::reset()
{
   Q_D(QGLContext);
   if (!d->valid) {
      return;
   }
   d->cleanup();
   doneCurrent();

   QMacCocoaAutoReleasePool pool;
   [static_cast<NSOpenGLContext *>(d->cx) release];

   d->cx = 0;
   [static_cast<NSOpenGLPixelFormat *>(d->vi) release];

   d->vi = 0;
   d->crWin = false;
   d->sharing = false;
   d->valid = false;
   d->transpColor = QColor();
   d->initDone = false;
   QGLContextGroup::removeShare(this);
}

void QGLContext::makeCurrent()
{
   Q_D(QGLContext);

   if (!d->valid) {
      qWarning("QGLContext::makeCurrent: Cannot make invalid context current");
      return;
   }

   [static_cast<NSOpenGLContext *>(d->cx) makeCurrentContext];

   QGLContextPrivate::setCurrentContext(this);
}

// internal
void QGLContext::updatePaintDevice()
{
   Q_D(QGLContext);

   QMacCocoaAutoReleasePool pool;

   if (d->paintDevice->devType() == QInternal::Widget) {
      //get control information
      QWidget *w = (QWidget *)d->paintDevice;
      NSView *view = qt_mac_nativeview_for(w);

      // Trying to attach the GL context to the NSView will fail with
      // "invalid drawable" if done too soon, but we have to make sure
      // the connection is made before the first paint event. Using
      // the NSView do to this check fails as the NSView is visible
      // before it's safe to connect, and using the NSWindow fails as
      // the NSWindow will become visible after the first paint event.
      // This leaves us with the QWidget, who's visible state seems
      // to match the point in time when it's safe to connect.
      if (!w || !w->isVisible()) {
         return;   // Not safe to attach GL context to view yet
      }

      if ([static_cast<NSOpenGLContext *>(d->cx) view] != view && ![view isHidden]) {
         [static_cast<NSOpenGLContext *>(d->cx) setView: view];
      }
   } else if (d->paintDevice->devType() == QInternal::Pixmap) {
      const QPixmap *pm = static_cast<const QPixmap *>(d->paintDevice);
      [static_cast<NSOpenGLContext *>(d->cx) setOffScreen: qt_mac_pixmap_get_base(pm)
       width: pm->width()
       height: pm->height()
       rowbytes: qt_mac_pixmap_get_bytes_per_line(pm)];
   } else {
      qWarning("QGLContext::updatePaintDevice: Not sure how to render OpenGL on this device");
   }

   [static_cast<NSOpenGLContext *>(d->cx) update];

}

void QGLContext::doneCurrent()
{
   if ( [NSOpenGLContext currentContext] != d_func()->cx) {
      return;
   }

   QGLContextPrivate::setCurrentContext(0);
   [NSOpenGLContext clearCurrentContext];
}

void QGLContext::swapBuffers() const
{
   Q_D(const QGLContext);

   if (!d->valid) {
      return;
   }

   [static_cast<NSOpenGLContext *>(d->cx) flushBuffer];

}

QColor QGLContext::overlayTransparentColor() const
{
   return QColor(0, 0, 0);                // Invalid color
}


uint QGLContext::colorIndex(const QColor &c) const
{
   Q_UNUSED(c);
   return 0;
}

void QGLContext::generateFontDisplayLists(const QFont & /* fnt */, int /* listBase */)
{
}

static CFBundleRef qt_getOpenGLBundle()
{
   CFBundleRef bundle = 0;
   CFStringRef urlString = QCFString::toCFStringRef(QLatin1String("/System/Library/Frameworks/OpenGL.framework"));
   QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                           urlString, kCFURLPOSIXPathStyle, false);
   if (url) {
      bundle = CFBundleCreate(kCFAllocatorDefault, url);
   }
   CFRelease(urlString);
   return bundle;
}

void *QGLContext::getProcAddress(const QString &proc) const
{
   CFStringRef procName = QCFString(proc).toCFStringRef(proc);
   void *result = CFBundleGetFunctionPointerForName(QCFType<CFBundleRef>(qt_getOpenGLBundle()),
                  procName);
   CFRelease(procName);
   return result;
}

void QGLWidget::setMouseTracking(bool enable)
{
   QWidget::setMouseTracking(enable);
}

void QGLWidget::resizeEvent(QResizeEvent *)
{
   Q_D(QGLWidget);

   if (!isValid()) {
      return;
   }

   makeCurrent();

   if (!d->glcx->initialized()) {
      glInit();
   }

   d->glcx->updatePaintDevice();

   resizeGL(width(), height());
}

const QGLContext *QGLWidget::overlayContext() const
{
   return 0;
}

void QGLWidget::makeOverlayCurrent()
{
}

void QGLWidget::updateOverlayGL()
{
}

void QGLWidget::setContext(QGLContext *context, const QGLContext *shareContext, bool deleteOldContext)
{
   Q_D(QGLWidget);
   if (context == 0) {
      qWarning("QGLWidget::setContext: Cannot set null context");
      return;
   }

   if (d->glcx) {
      d->glcx->doneCurrent();
   }
   QGLContext *oldcx = d->glcx;
   d->glcx = context;
   if (!d->glcx->isValid()) {
      d->glcx->create(shareContext ? shareContext : oldcx);
   }
   if (deleteOldContext && oldcx) {
      delete oldcx;
   }
}

void QGLWidgetPrivate::init(QGLContext *context, const QGLWidget *shareWidget)
{
   Q_Q(QGLWidget);

   initContext(context, shareWidget);

   QWidget *current = q;
   while (current) {
      qt_widget_private(current)->glWidgets.append(QWidgetPrivate::GlWidgetInfo(q));
      if (current->isWindow()) {
         break;
      }
      current = current->parentWidget();
   }
}

bool QGLWidgetPrivate::renderCxPm(QPixmap *)
{
   return false;
}

void QGLWidgetPrivate::cleanupColormaps()
{
}

const QGLColormap &QGLWidget::colormap() const
{
   return d_func()->cmap;
}

void QGLWidget::setColormap(const QGLColormap &)
{
}

void QGLWidgetPrivate::updatePaintDevice()
{
   Q_Q(QGLWidget);
   glcx->updatePaintDevice();
   q->update();
}

#endif

QT_END_NAMESPACE
