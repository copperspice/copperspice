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

#include <qscreenqnx_qws.h>
#include <qapplication.h>
#include <qregexp.h>

#include <gf/gf.h>

QT_BEGIN_NAMESPACE

// This struct holds all the pointers to QNX's internals
struct QQnxScreenContext {
   inline QQnxScreenContext()
      : device(0), display(0), layer(0), hwSurface(0), memSurface(0), context(0) {
   }
   inline ~QQnxScreenContext() {
      cleanup();
   }

   void cleanup();

   gf_dev_t device;
   gf_dev_info_t deviceInfo;
   gf_display_t display;
   gf_display_info_t displayInfo;
   gf_layer_t layer;
   gf_surface_t hwSurface;
   gf_surface_t memSurface;
   gf_surface_info_t memSurfaceInfo;
   gf_context_t context;
};

void QQnxScreenContext::cleanup()
{
   if (context) {
      gf_context_free(context);
      context = 0;
   }
   if (memSurface) {
      gf_surface_free(memSurface);
      memSurface = 0;
   }
   if (hwSurface) {
      gf_surface_free(hwSurface);
      hwSurface = 0;
   }
   if (layer) {
      gf_layer_detach(layer);
      layer = 0;
   }
   if (display) {
      gf_display_detach(display);
      display = 0;
   }
   if (device) {
      gf_dev_detach(device);
      device = 0;
   }
}


/*!
    \class QQnxScreen
    \preliminary
    \ingroup qws
    \since 4.6
    \internal

    \brief The QQnxScreen class implements a screen driver
    for QNX io-display based devices.

    Note - you never have to instanciate this class, the QScreenDriverFactory
    does that for us based on the \c{QWS_DISPLAY} environment variable.

    To activate this driver, set \c{QWS_DISPLAY} to \c{qnx}.

    Example:
    \c{QWS_DISPLAY=qnx; export QWS_DISPLAY}

    By default, the main layer of the first display of the first device is used.
    If you have multiple graphic cards, multiple displays or multiple layers and
    don't want to connect to the default, you can override that with setting
    the corresponding options \c{device}, \c{display} or \c{layer} in the \c{QWS_DISPLAY} variable:

    \c{QWS_DISPLAY=qnx:device=3:display=4:layer=5}

    In addition, it is suggested to set the physical width and height of the display.
    QQnxScreen will use that information to compute the dots per inch (DPI) in order to render
    fonts correctly. If this informaiton is omitted, QQnxScreen defaults to 72 dpi.

    \c{QWS_DISPLAY=qnx:mmWidth=120:mmHeight=80}

    \c{mmWidth} and \c{mmHeight} are the physical width/height of the screen in millimeters.

    \sa QScreen, QScreenDriverPlugin, {Running Qt for Embedded Linux Applications}{Running Applications}
*/

/*!
    Constructs a QQnxScreen object. The \a display_id argument
    identifies the Qt for Embedded Linux server to connect to.
*/
QQnxScreen::QQnxScreen(int display_id)
   : QScreen(display_id), d(new QQnxScreenContext)
{
}

/*!
    Destroys this QQnxScreen object.
*/
QQnxScreen::~QQnxScreen()
{
   delete d;
}

/*!
    \reimp
*/
bool QQnxScreen::initDevice()
{
#ifndef QT_NO_QWS_CURSOR
   QScreenCursor::initSoftwareCursor();
#endif

   return true;
}

/*!
    \internal
    Attaches to the named device \a name.
*/
static inline bool attachDevice(QQnxScreenContext *const d, const char *name)
{
   int ret = gf_dev_attach(&d->device, name, &d->deviceInfo);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_dev_attach(%s) failed with error code %d", name, ret);
      return false;
   }
   return true;
}

/*!
    \internal
    Attaches to the display at index \a displayIndex.
*/
static inline bool attachDisplay(QQnxScreenContext *const d, int displayIndex)
{
   int ret = gf_display_attach(&d->display, d->device, displayIndex, &d->displayInfo);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_display_attach(%d) failed with error code %d", displayIndex, ret);
      return false;
   }
   return true;
}

/*!
    \internal
    Attaches to the layer \a layerIndex.
*/
static inline bool attachLayer(QQnxScreenContext *const d, int layerIndex)
{
   unsigned flags = QApplication::type() != QApplication::GuiServer ? GF_LAYER_ATTACH_PASSIVE : 0;
   int ret = gf_layer_attach(&d->layer, d->display, layerIndex, flags);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_layer_attach(%d) failed with error code %d", layerIndex, ret);
      return false;
   }

   return true;
}

/*!
    \internal
    Creates a new hardware surface (usually on the Gfx card memory) with the dimensions \a w * \a h.
*/
static inline bool createHwSurface(QQnxScreenContext *const d, int w, int h)
{
   int ret = gf_surface_create_layer(&d->hwSurface, &d->layer, 1, 0,
                                     w, h, d->displayInfo.format, 0, 0);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_surface_create_layer(%dx%d) failed with error code %d", w, h, ret);
      return false;
   }

   gf_layer_set_surfaces(d->layer, &d->hwSurface, 1);

   gf_layer_enable(d->layer);

   ret = gf_layer_update(d->layer, 0);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_layer_update() failed with error code %d\n", ret);
      return false;
   }

   ret = gf_context_create(&d->context);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_context_create() failed with error code %d", ret);
      return false;
   }

   ret = gf_context_set_surface(d->context, d->hwSurface);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_context_set_surface() failed with error code %d", ret);
      return false;
   }

   return true;
}

/*!
    \internal
    Creates an in-memory, linear accessible surface of dimensions \a w * \a h.
    This is the main surface that QWS blits to.
*/
static inline bool createMemSurface(QQnxScreenContext *const d, int w, int h)
{
#ifndef QT_NO_QWS_MULTIPROCESS
   if (QApplication::type() != QApplication::GuiServer) {
      unsigned sidlist[64];
      int n = gf_surface_sidlist(d->device, sidlist); // undocumented API
      for (int i = 0; i < n; ++i) {
         int ret = gf_surface_attach_by_sid(&d->memSurface, d->device, sidlist[i]);
         if (ret == GF_ERR_OK) {
            gf_surface_get_info(d->memSurface, &d->memSurfaceInfo);
            if (d->memSurfaceInfo.sid != unsigned(GF_SID_INVALID)) {
               // can we use the surface's vaddr?
               unsigned flags = GF_SURFACE_CPU_LINEAR_READABLE | GF_SURFACE_CPU_LINEAR_WRITEABLE;
               if ((d->memSurfaceInfo.flags & flags) == flags) {
                  return true;
               }
            }

            gf_surface_free(d->memSurface);
            d->memSurface = 0;
         }
      }
      qWarning("QQnxScreen: cannot attach to an usable surface; create a new one.");
   }
#endif
   int ret = gf_surface_create(&d->memSurface, d->device, w, h, d->displayInfo.format, 0,
                               GF_SURFACE_CREATE_CPU_FAST_ACCESS | GF_SURFACE_CREATE_CPU_LINEAR_ACCESSIBLE
                               | GF_SURFACE_CREATE_PHYS_CONTIG | GF_SURFACE_CREATE_SHAREABLE);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_surface_create(%dx%d) failed with error code %d",
               w, h, ret);
      return false;
   }

   gf_surface_get_info(d->memSurface, &d->memSurfaceInfo);

   if (d->memSurfaceInfo.sid == unsigned(GF_SID_INVALID)) {
      qWarning("QQnxScreen: gf_surface_get_info() failed.");
      return false;
   }

   return true;
}

/*!
    \reimp
    Connects to QNX's io-display based device based on the \a displaySpec parameters
    from the \c{QWS_DISPLAY} environment variable. See the QQnxScreen class documentation
    for possible parameters.

    \sa QQnxScreen
*/
bool QQnxScreen::connect(const QString &displaySpec)
{
   const QStringList params = displaySpec.split(QLatin1Char(':'), QString::SkipEmptyParts);

   // default to device 0
   int deviceIndex = 0;
   if (!params.isEmpty()) {
      QRegExp deviceRegExp(QLatin1String("^device=(.+)$"));
      if (params.indexOf(deviceRegExp) != -1) {
         deviceIndex = deviceRegExp.cap(1).toInt();
      }
   }

   if (!attachDevice(d, GF_DEVICE_INDEX(deviceIndex))) {
      return false;
   }

   qDebug("QQnxScreen: Attached to Device, number of displays: %d", d->deviceInfo.ndisplays);

   // default to display id passed to constructor
   int displayIndex = displayId;
   if (!params.isEmpty()) {
      QRegExp displayRegexp(QLatin1String("^display=(\\d+)$"));
      if (params.indexOf(displayRegexp) != -1) {
         displayIndex = displayRegexp.cap(1).toInt();
      }
   }

   if (!attachDisplay(d, displayIndex)) {
      return false;
   }

   qDebug("QQnxScreen: Attached to Display %d, resolution %dx%d, refresh %d Hz",
          displayIndex, d->displayInfo.xres, d->displayInfo.yres, d->displayInfo.refresh);

   // default to main_layer_index from the displayInfo struct
   int layerIndex = d->displayInfo.main_layer_index;
   if (!params.isEmpty()) {
      QRegExp layerRegexp(QLatin1String("^layer=(\\d+)$"));
      if (params.indexOf(layerRegexp) != -1) {
         layerIndex = layerRegexp.cap(1).toInt();
      }
   }

   if (!attachLayer(d, layerIndex)) {
      return false;
   }

   // determine the pixel format and the pixel type
   switch (d->displayInfo.format) {
#if defined(QT_QWS_DEPTH_32) || defined(QT_QWS_DEPTH_GENERIC)
      case GF_FORMAT_ARGB8888:
         pixeltype = QScreen::BGRPixel;
      // fall through
      case GF_FORMAT_BGRA8888:
         setPixelFormat(QImage::Format_ARGB32);
         break;
#endif
#if defined(QT_QWS_DEPTH_24)
      case GF_FORMAT_BGR888:
         pixeltype = QScreen::BGRPixel;
         setPixelFormat(QImage::Format_RGB888);
         break;
#endif
#if defined(QT_QWS_DEPTH_16) || defined(QT_QWS_DEPTH_GENERIC)
      case GF_FORMAT_PACK_RGB565:
      case GF_FORMAT_PKLE_RGB565:
      case GF_FORMAT_PKBE_RGB565:
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
         setFrameBufferLittleEndian((d->displayInfo.format & GF_FORMAT_PKLE) == GF_FORMAT_PKLE);
#endif
         setPixelFormat(QImage::Format_RGB16);
         break;
#endif
      default:
         return false;
   }

   // tell QWSDisplay the width and height of the display
   w = dw = d->displayInfo.xres;
   h = dh = d->displayInfo.yres;
   QScreen::d = (d->displayInfo.format & GF_FORMAT_BPP);  // colour depth

   // assume 72 dpi as default, to calculate the physical dimensions if not specified
   const int defaultDpi = 72;
   // Handle display physical size
   physWidth = qRound(dw * 25.4 / defaultDpi);
   physHeight = qRound(dh * 25.4 / defaultDpi);
   if (!params.isEmpty()) {
      QRegExp mmWidthRegexp(QLatin1String("^mmWidth=(\\d+)$"));
      if (params.indexOf(mmWidthRegexp) != -1) {
         physWidth = mmWidthRegexp.cap(1).toInt();
      }

      QRegExp mmHeightRegexp(QLatin1String("^mmHeight=(\\d+)$"));
      if (params.indexOf(mmHeightRegexp) != -1) {
         physHeight = mmHeightRegexp.cap(1).toInt();
      }
   }

   if (QApplication::type() == QApplication::GuiServer) {
      // create a hardware surface with our dimensions. In the old days, it was possible
      // to get a pointer directly to the hw surface, so we could blit directly. Now, we
      // have to use one indirection more, because it's not guaranteed that the hw surface
      // is mappable into our process.
      if (!createHwSurface(d, w, h)) {
         return false;
      }
   }

   // create an in-memory linear surface that is used by QWS. QWS will blit directly in here.
   if (!createMemSurface(d, w, h)) {
      return false;
   }

   // set the address of the in-memory buffer that QWS is blitting to
   data = d->memSurfaceInfo.vaddr;
   // set the line stepping
   lstep = d->memSurfaceInfo.stride;

   // the overall size of the in-memory buffer is linestep * height
   size = mapsize = lstep * h;

   // done, the driver should be connected to the display now.
   return true;
}

/*!
    \reimp
*/
void QQnxScreen::disconnect()
{
   d->cleanup();
}

/*!
    \reimp
*/
void QQnxScreen::shutdownDevice()
{
}

/*!
    \reimp
    QQnxScreen doesn't support setting the mode, use io-display instead.
*/
void QQnxScreen::setMode(int, int, int)
{
   qWarning("QQnxScreen: Unable to change mode, use io-display instead.");
}

/*!
    \reimp
*/
bool QQnxScreen::supportsDepth(int depth) const
{
   gf_modeinfo_t displayMode;
   for (int i = 0; gf_display_query_mode(d->display, i, &displayMode) == GF_ERR_OK; ++i) {
      switch (displayMode.primary_format) {
#if defined(QT_QWS_DEPTH_32) || defined(QT_QWS_DEPTH_GENERIC)
         case GF_FORMAT_ARGB8888:
         case GF_FORMAT_BGRA8888:
            if (depth == 32) {
               return true;
            }
            break;
#endif
#if defined(QT_QWS_DEPTH_24)
         case GF_FORMAT_BGR888:
            if (depth == 24) {
               return true;
            }
            break;
#endif
#if defined(QT_QWS_DEPTH_16) || defined(QT_QWS_DEPTH_GENERIC)
         case GF_FORMAT_PACK_RGB565:
         case GF_FORMAT_PKLE_RGB565:
         case GF_FORMAT_PKBE_RGB565:
            if (depth == 16) {
               return true;
            }
            break;
#endif
         default:
            break;
      }
   }

   return false;
}

/*!
    \reimp
*/
void QQnxScreen::blank(bool on)
{
   int ret = gf_display_set_dpms(d->display, on ? GF_DPMS_OFF : GF_DPMS_ON);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_display_set_dpms() failed with error code %d", ret);
   }
}

/*!
    \reimp
*/
void QQnxScreen::exposeRegion(QRegion r, int changing)
{
   // here is where the actual magic happens. QWS will call exposeRegion whenever
   // a region on the screen is dirty and needs to be updated on the actual screen.

   // first, call the parent implementation. The parent implementation will update
   // the region on our in-memory surface
   QScreen::exposeRegion(r, changing);

   // now our in-memory surface should be up to date with the latest changes.

   if (!d->hwSurface) {
      return;
   }

   // the code below copies the region from the in-memory surface to the hardware.

   // just get the bounding rectangle of the region. Most screen updates are rectangular
   // anyways. Code could be optimized to blit each and every member of the region
   // individually, but in real life, the speed-up is neglectable
   const QRect br = r.boundingRect();
   if (br.isEmpty()) {
      return;   // ignore empty regions because gf_draw_blit2 doesn't like 0x0 dimensions
   }

   // start drawing.
   int ret = gf_draw_begin(d->context);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_draw_begin() failed with error code %d", ret);
      return;
   }

   // blit the changed region from the memory surface to the hardware surface
   ret = gf_draw_blit2(d->context, d->memSurface, d->hwSurface,
                       br.x(), br.y(), br.right(), br.bottom(), br.x(), br.y());
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_draw_blit2() failed with error code %d", ret);
   }

   // flush all drawing commands (in our case, a single blit)
   ret = gf_draw_flush(d->context);
   if (ret != GF_ERR_OK) {
      qWarning("QQnxScreen: gf_draw_flush() failed with error code %d", ret);
   }

   // tell QNX that we're done drawing.
   gf_draw_end(d->context);
}

QT_END_NAMESPACE
