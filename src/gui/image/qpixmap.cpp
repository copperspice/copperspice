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

#include <qglobal.h>

#include <qpixmap.h>
#include <qpixmapdata_p.h>
#include <qimagepixmapcleanuphooks_p.h>
#include <qbitmap.h>
#include <qcolormap.h>
#include <qimage.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <qapplication.h>
#include <qapplication_p.h>
#include <qgraphicssystem_p.h>
#include <qwidget_p.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpixmapcache.h>
#include <qdatetime.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qpaintengine.h>
#include <qthread.h>

#ifdef Q_OS_MAC
# include <qt_mac_p.h>
# include <qpixmap_mac_p.h>
#endif

#ifdef Q_WS_QPA
# include <qplatformintegration_qpa.h>
#endif

#if defined(Q_WS_X11)
# include <qx11info_x11.h>
# include <qt_x11_p.h>
# include <qpixmap_x11_p.h>
#endif

#include <qpixmap_raster_p.h>
#include <qhexstring_p.h>

QT_BEGIN_NAMESPACE

// ### Qt5: remove
Q_GUI_EXPORT qint64 qt_pixmap_id(const QPixmap &pixmap)
{
   return pixmap.cacheKey();
}

static bool qt_pixmap_thread_test()
{
   if (!qApp) {
      qFatal("QPixmap: Must construct a QApplication before a QPaintDevice");
      return false;
   }

   if (qApp->thread() != QThread::currentThread()) {
      bool fail = false;
#if defined (Q_WS_X11)
      if (!QApplication::testAttribute(Qt::AA_X11InitThreads)) {
         fail = true;
      }
#elif defined (Q_WS_QPA)
      if (!QApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::ThreadedPixmaps)) {
         printf("Lighthouse plugin does not support threaded pixmaps!\n");
         fail = true;
      }
#else
      if (QApplicationPrivate::graphics_system_name != QLatin1String("raster")) {
         fail = true;
      }
#endif
      if (fail) {
         qWarning("QPixmap: It is not safe to use pixmaps outside the GUI thread");
         return false;
      }
   }
   return true;
}

void QPixmap::init(int w, int h, Type type)
{
   init(w, h, int(type));
}

extern QApplication::Type qt_appType;

void QPixmap::init(int w, int h, int type)
{
   if (qt_appType == QApplication::Tty) {
      qWarning("QPixmap: Cannot create a QPixmap when no GUI is being used");
      data = 0;
      return;
   }

   if ((w > 0 && h > 0) || type == QPixmapData::BitmapType) {
      data = QPixmapData::create(w, h, (QPixmapData::PixelType) type);
   } else {
      data = 0;
   }
}

/*!
    \enum QPixmap::ColorMode

    \compat

    This enum type defines the color modes that exist for converting
    QImage objects to QPixmap.  It is provided here for compatibility
    with earlier versions of Qt.

    Use Qt::ImageConversionFlags instead.

    \value Auto  Select \c Color or \c Mono on a case-by-case basis.
    \value Color Always create colored pixmaps.
    \value Mono  Always create bitmaps.
*/

/*!
    Constructs a null pixmap.

    \sa isNull()
*/

QPixmap::QPixmap()
   : QPaintDevice()
{
   (void) qt_pixmap_thread_test();
   init(0, 0, QPixmapData::PixmapType);
}

/*!
    \fn QPixmap::QPixmap(int width, int height)

    Constructs a pixmap with the given \a width and \a height. If
    either \a width or \a height is zero, a null pixmap is
    constructed.

    \warning This will create a QPixmap with uninitialized data. Call
    fill() to fill the pixmap with an appropriate color before drawing
    onto it with QPainter.

    \sa isNull()
*/

QPixmap::QPixmap(int w, int h)
   : QPaintDevice()
{
   if (!qt_pixmap_thread_test()) {
      init(0, 0, QPixmapData::PixmapType);
   } else {
      init(w, h, QPixmapData::PixmapType);
   }
}

/*!
    \overload

    Constructs a pixmap of the given \a size.

    \warning This will create a QPixmap with uninitialized data. Call
    fill() to fill the pixmap with an appropriate color before drawing
    onto it with QPainter.
*/

QPixmap::QPixmap(const QSize &size)
   : QPaintDevice()
{
   if (!qt_pixmap_thread_test()) {
      init(0, 0, QPixmapData::PixmapType);
   } else {
      init(size.width(), size.height(), QPixmapData::PixmapType);
   }
}

/*!
  \internal
*/
QPixmap::QPixmap(const QSize &s, Type type)
{
   if (!qt_pixmap_thread_test()) {
      init(0, 0, type);
   } else {
      init(s.width(), s.height(), type);
   }
}

/*!
  \internal
*/
QPixmap::QPixmap(const QSize &s, int type)
{
   if (!qt_pixmap_thread_test()) {
      init(0, 0, static_cast<QPixmapData::PixelType>(type));
   } else {
      init(s.width(), s.height(), static_cast<QPixmapData::PixelType>(type));
   }
}

/*!
    \internal
*/
QPixmap::QPixmap(QPixmapData *d)
   : QPaintDevice(), data(d)
{
}

/*!
    Constructs a pixmap from the file with the given \a fileName. If the
    file does not exist or is of an unknown format, the pixmap becomes a
    null pixmap.

    The loader attempts to read the pixmap using the specified \a
    format. If the \a format is not specified (which is the default),
    the loader probes the file for a header to guess the file format.

    The file name can either refer to an actual file on disk or to
    one of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how
    to embed images and other resource files in the application's
    executable.

    If the image needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a
    flags to control the conversion.

    The \a fileName, \a format and \a flags parameters are
    passed on to load(). This means that the data in \a fileName is
    not compiled into the binary. If \a fileName contains a relative
    path (e.g. the filename only) the relevant file must be found
    relative to the runtime working directory.

    \sa {QPixmap#Reading and Writing Image Files}{Reading and Writing
    Image Files}
*/

QPixmap::QPixmap(const QString &fileName, const char *format, Qt::ImageConversionFlags flags)
   : QPaintDevice()
{
   init(0, 0, QPixmapData::PixmapType);
   if (!qt_pixmap_thread_test()) {
      return;
   }

   load(fileName, format, flags);
}

/*!
    Constructs a pixmap that is a copy of the given \a pixmap.

    \sa copy()
*/

QPixmap::QPixmap(const QPixmap &pixmap)
   : QPaintDevice()
{
   if (!qt_pixmap_thread_test()) {
      init(0, 0, QPixmapData::PixmapType);
      return;
   }
   if (pixmap.paintingActive()) {                // make a deep copy
      operator=(pixmap.copy());
   } else {
      data = pixmap.data;
   }
}

/*!
    Constructs a pixmap from the given \a xpm data, which must be a
    valid XPM image.

    Errors are silently ignored.

    Note that it's possible to squeeze the XPM variable a little bit
    by using an unusual declaration:

    \snippet doc/src/snippets/code/src_gui_image_qpixmap.cpp 0

    The extra \c const makes the entire definition read-only, which is
    slightly more efficient (for example, when the code is in a shared
    library) and ROMable when the application is to be stored in ROM.
*/
#ifndef QT_NO_IMAGEFORMAT_XPM
QPixmap::QPixmap(const char *const xpm[])
   : QPaintDevice()
{
   init(0, 0, QPixmapData::PixmapType);
   if (!xpm) {
      return;
   }

   QImage image(xpm);
   if (!image.isNull()) {
      if (data && data->pixelType() == QPixmapData::BitmapType) {
         *this = QBitmap::fromImage(image);
      } else {
         *this = fromImage(image);
      }
   }
}
#endif


/*!
    Destroys the pixmap.
*/

QPixmap::~QPixmap()
{
   Q_ASSERT(!data || data->ref.load() >= 1); // Catch if ref-counting changes again
}

/*!
  \internal
*/
int QPixmap::devType() const
{
   return QInternal::Pixmap;
}

/*!
    \fn QPixmap QPixmap::copy(int x, int y, int width, int height) const
    \overload

    Returns a deep copy of the subset of the pixmap that is specified
    by the rectangle QRect( \a x, \a y, \a width, \a height).
*/

/*!
    \fn QPixmap QPixmap::copy(const QRect &rectangle) const

    Returns a deep copy of the subset of the pixmap that is specified
    by the given \a rectangle. For more information on deep copies,
    see the \l {Implicit Data Sharing} documentation.

    If the given \a rectangle is empty, the whole image is copied.

    \sa operator=(), QPixmap(), {QPixmap#Pixmap
    Transformations}{Pixmap Transformations}
*/
QPixmap QPixmap::copy(const QRect &rect) const
{
   if (isNull()) {
      return QPixmap();
   }

   QRect r(0, 0, width(), height());
   if (!rect.isEmpty()) {
      r = r.intersected(rect);
   }

   QPixmapData *d = data->createCompatiblePixmapData();
   d->copy(data.data(), r);
   return QPixmap(d);
}

/*!
    \fn QPixmap::scroll(int dx, int dy, int x, int y, int width, int height, QRegion *exposed)
    \since 4.6

    This convenience function is equivalent to calling QPixmap::scroll(\a dx,
    \a dy, QRect(\a x, \a y, \a width, \a height), \a exposed).

    \sa QWidget::scroll(), QGraphicsItem::scroll()
*/

/*!
    \since 4.6

    Scrolls the area \a rect of this pixmap by (\a dx, \a dy). The exposed
    region is left unchanged. You can optionally pass a pointer to an empty
    QRegion to get the region that is \a exposed by the scroll operation.

    \snippet doc/src/snippets/code/src_gui_image_qpixmap.cpp 2

    You cannot scroll while there is an active painter on the pixmap.

    \sa QWidget::scroll(), QGraphicsItem::scroll()
*/
void QPixmap::scroll(int dx, int dy, const QRect &rect, QRegion *exposed)
{
   if (isNull() || (dx == 0 && dy == 0)) {
      return;
   }
   QRect dest = rect & this->rect();
   QRect src = dest.translated(-dx, -dy) & dest;
   if (src.isEmpty()) {
      if (exposed) {
         *exposed += dest;
      }
      return;
   }

   detach();

   if (!data->scroll(dx, dy, src)) {
      // Fallback
      QPixmap pix = *this;
      QPainter painter(&pix);
      painter.setCompositionMode(QPainter::CompositionMode_Source);
      painter.drawPixmap(src.translated(dx, dy), *this, src);
      painter.end();
      *this = pix;
   }

   if (exposed) {
      *exposed += dest;
      *exposed -= src.translated(dx, dy);
   }
}

/*!
    Assigns the given \a pixmap to this pixmap and returns a reference
    to this pixmap.

    \sa copy(), QPixmap()
*/

QPixmap &QPixmap::operator=(const QPixmap &pixmap)
{
   if (paintingActive()) {
      qWarning("QPixmap::operator=: Cannot assign to pixmap during painting");
      return *this;
   }
   if (pixmap.paintingActive()) {                // make a deep copy
      *this = pixmap.copy();
   } else {
      data = pixmap.data;
   }
   return *this;
}

/*!
    \fn void QPixmap::swap(QPixmap &other)
    \since 4.8

    Swaps pixmap \a other with this pixmap. This operation is very
    fast and never fails.
*/

/*!
   Returns the pixmap as a QVariant.
*/
QPixmap::operator QVariant() const
{
   return QVariant(QVariant::Pixmap, this);
}

/*!
    \fn bool QPixmap::operator!() const

    Returns true if this is a null pixmap; otherwise returns false.

    \sa isNull()
*/

/*!
    \fn QPixmap::operator QImage() const

    Returns the pixmap as a QImage.

    Use the toImage() function instead.
*/

/*!
    Converts the pixmap to a QImage. Returns a null image if the
    conversion fails.

    If the pixmap has 1-bit depth, the returned image will also be 1
    bit deep. Images with more bits will be returned in a format
    closely represents the underlying system. Usually this will be
    QImage::Format_ARGB32_Premultiplied for pixmaps with an alpha and
    QImage::Format_RGB32 or QImage::Format_RGB16 for pixmaps without
    alpha.

    Note that for the moment, alpha masks on monochrome images are
    ignored.

    \sa fromImage(), {QImage#Image Formats}{Image Formats}
*/
QImage QPixmap::toImage() const
{
   if (isNull()) {
      return QImage();
   }

   return data->toImage();
}

/*!
    \fn QMatrix QPixmap::trueMatrix(const QTransform &matrix, int width, int height)

    Returns the actual matrix used for transforming a pixmap with the
    given \a width, \a height and \a matrix.

    When transforming a pixmap using the transformed() function, the
    transformation matrix is internally adjusted to compensate for
    unwanted translation, i.e. transformed() returns the smallest
    pixmap containing all transformed points of the original
    pixmap. This function returns the modified matrix, which maps
    points correctly from the original pixmap into the new pixmap.

    \sa transformed(), {QPixmap#Pixmap Transformations}{Pixmap
    Transformations}
*/
QTransform QPixmap::trueMatrix(const QTransform &m, int w, int h)
{
   return QImage::trueMatrix(m, w, h);
}

/*!
  \overload

  This convenience function loads the matrix \a m into a
  QTransform and calls the overloaded function with the
  QTransform and the width \a w and the height \a h.
 */
QMatrix QPixmap::trueMatrix(const QMatrix &m, int w, int h)
{
   return trueMatrix(QTransform(m), w, h).toAffine();
}


/*!
    \fn bool QPixmap::isQBitmap() const

    Returns true if this is a QBitmap; otherwise returns false.
*/

bool QPixmap::isQBitmap() const
{
   return data->type == QPixmapData::BitmapType;
}

/*!
    \fn bool QPixmap::isNull() const

    Returns true if this is a null pixmap; otherwise returns false.

    A null pixmap has zero width, zero height and no contents. You
    cannot draw in a null pixmap.
*/
bool QPixmap::isNull() const
{
   return !data || data->isNull();
}

/*!
    \fn int QPixmap::width() const

    Returns the width of the pixmap.

    \sa size(), {QPixmap#Pixmap Information}{Pixmap Information}
*/
int QPixmap::width() const
{
   return data ? data->width() : 0;
}

/*!
    \fn int QPixmap::height() const

    Returns the height of the pixmap.

    \sa size(), {QPixmap#Pixmap Information}{Pixmap Information}
*/
int QPixmap::height() const
{
   return data ? data->height() : 0;
}

/*!
    \fn QSize QPixmap::size() const

    Returns the size of the pixmap.

    \sa width(), height(), {QPixmap#Pixmap Information}{Pixmap
    Information}
*/
QSize QPixmap::size() const
{
   return data ? QSize(data->width(), data->height()) : QSize(0, 0);
}

/*!
    \fn QRect QPixmap::rect() const

    Returns the pixmap's enclosing rectangle.

    \sa {QPixmap#Pixmap Information}{Pixmap Information}
*/
QRect QPixmap::rect() const
{
   return data ? QRect(0, 0, data->width(), data->height()) : QRect();
}

/*!
    \fn int QPixmap::depth() const

    Returns the depth of the pixmap.

    The pixmap depth is also called bits per pixel (bpp) or bit planes
    of a pixmap. A null pixmap has depth 0.

    \sa defaultDepth(), {QPixmap#Pixmap Information}{Pixmap
    Information}
*/
int QPixmap::depth() const
{
   return data ? data->depth() : 0;
}

/*!
    \fn void QPixmap::resize(const QSize &size)
    \overload
    \compat

    Use QPixmap::copy() instead to get the pixmap with the new size.

    \oldcode
        pixmap.resize(size);
    \newcode
        pixmap = pixmap.copy(QRect(QPoint(0, 0), size));
    \endcode
*/

/*!
    \fn void QPixmap::resize(int width, int height)
    \compat

    Use QPixmap::copy() instead to get the pixmap with the new size.

    \oldcode
        pixmap.resize(10, 20);
    \newcode
        pixmap = pixmap.copy(0, 0, 10, 20);
    \endcode
*/

/*!
    \fn bool QPixmap::selfMask() const
    \compat

    Returns whether the pixmap is its own mask or not.

    This function is no longer relevant since the concept of self
    masking doesn't exists anymore.
*/

/*!
    Sets a mask bitmap.

    This function merges the \a mask with the pixmap's alpha channel. A pixel
    value of 1 on the mask means the pixmap's pixel is unchanged; a value of 0
    means the pixel is transparent. The mask must have the same size as this
    pixmap.

    Setting a null mask resets the mask, leaving the previously transparent
    pixels black. The effect of this function is undefined when the pixmap is
    being painted on.

    \warning This is potentially an expensive operation.

    \sa mask(), {QPixmap#Pixmap Transformations}{Pixmap Transformations},
    QBitmap
*/
void QPixmap::setMask(const QBitmap &mask)
{
   if (paintingActive()) {
      qWarning("QPixmap::setMask: Cannot set mask while pixmap is being painted on");
      return;
   }

   if (!mask.isNull() && mask.size() != size()) {
      qWarning("QPixmap::setMask() mask size differs from pixmap size");
      return;
   }

   if (isNull()) {
      return;
   }

   if (static_cast<const QPixmap &>(mask).data == data) { // trying to selfmask
      return;
   }

   detach();
   data->setMask(mask);
}

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
/*!
    Creates and returns a heuristic mask for this pixmap.

    The function works by selecting a color from one of the corners
    and then chipping away pixels of that color, starting at all the
    edges.  If \a clipTight is true (the default) the mask is just
    large enough to cover the pixels; otherwise, the mask is larger
    than the data pixels.

    The mask may not be perfect but it should be reasonable, so you
    can do things such as the following:

    \snippet doc/src/snippets/code/src_gui_image_qpixmap.cpp 1

    This function is slow because it involves converting to/from a
    QImage, and non-trivial computations.

    \sa QImage::createHeuristicMask(), createMaskFromColor()
*/
QBitmap QPixmap::createHeuristicMask(bool clipTight) const
{
   QBitmap m = QBitmap::fromImage(toImage().createHeuristicMask(clipTight));
   return m;
}
#endif

/*!
    Creates and returns a mask for this pixmap based on the given \a
    maskColor. If the \a mode is Qt::MaskInColor, all pixels matching the
    maskColor will be transparent. If \a mode is Qt::MaskOutColor, all pixels
    matching the maskColor will be opaque.

    This function is slow because it involves converting to/from a
    QImage.

    \sa createHeuristicMask(), QImage::createMaskFromColor()
*/
QBitmap QPixmap::createMaskFromColor(const QColor &maskColor, Qt::MaskMode mode) const
{
   QImage image = toImage().convertToFormat(QImage::Format_ARGB32);
   return QBitmap::fromImage(image.createMaskFromColor(maskColor.rgba(), mode));
}

/*! \overload

    Creates and returns a mask for this pixmap based on the given \a
    maskColor. Same as calling createMaskFromColor(maskColor,
    Qt::MaskInColor)

    \sa createHeuristicMask(), QImage::createMaskFromColor()
*/
QBitmap QPixmap::createMaskFromColor(const QColor &maskColor) const
{
   return createMaskFromColor(maskColor, Qt::MaskInColor);
}

/*!
    Loads a pixmap from the file with the given \a fileName. Returns
    true if the pixmap was successfully loaded; otherwise returns
    false.

    The loader attempts to read the pixmap using the specified \a
    format. If the \a format is not specified (which is the default),
    the loader probes the file for a header to guess the file format.

    The file name can either refer to an actual file on disk or to one
    of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how to
    embed pixmaps and other resource files in the application's
    executable.

    If the data needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a flags to
    control the conversion.

    Note that QPixmaps are automatically added to the QPixmapCache
    when loaded from a file; the key used is internal and can not
    be acquired.

    \sa loadFromData(), {QPixmap#Reading and Writing Image
    Files}{Reading and Writing Image Files}
*/

bool QPixmap::load(const QString &fileName, const char *format, Qt::ImageConversionFlags flags)
{
   if (fileName.isEmpty()) {
      return false;
   }

   QFileInfo info(fileName);

   QString key = "cs_pixmap"
                 + info.absoluteFilePath()
                 + HexString<uint>(info.lastModified().toTime_t())
                 + HexString<quint64>(info.size())
                 + HexString<uint>(data ? data->pixelType() : QPixmapData::PixmapType);

   // Note: If no extension is provided, we try to match the
   // file against known plugin extensions
   if (!info.completeSuffix().isEmpty() && !info.exists()) {
      return false;
   }

   if (QPixmapCache::find(key, *this)) {
      return true;
   }

   QScopedPointer<QPixmapData> tmp(QPixmapData::create(0, 0, data ? data->type : QPixmapData::PixmapType));
   if (tmp->fromFile(fileName, format, flags)) {
      data = tmp.take();
      QPixmapCache::insert(key, *this);
      return true;
   }

   return false;
}

/*!
    \fn bool QPixmap::loadFromData(const uchar *data, uint len, const char *format, Qt::ImageConversionFlags flags)

    Loads a pixmap from the \a len first bytes of the given binary \a
    data.  Returns true if the pixmap was loaded successfully;
    otherwise returns false.

    The loader attempts to read the pixmap using the specified \a
    format. If the \a format is not specified (which is the default),
    the loader probes the file for a header to guess the file format.

    If the data needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a flags to
    control the conversion.

    \sa load(), {QPixmap#Reading and Writing Image Files}{Reading and
    Writing Image Files}
*/

bool QPixmap::loadFromData(const uchar *buf, uint len, const char *format, Qt::ImageConversionFlags flags)
{
   if (len == 0 || buf == 0) {
      return false;
   }

   if (!data) {
      data = QPixmapData::create(0, 0, QPixmapData::PixmapType);
   }

   return data->fromData(buf, len, format, flags);
}

/*!
    \fn bool QPixmap::loadFromData(const QByteArray &data, const char *format, Qt::ImageConversionFlags flags)

    \overload

    Loads a pixmap from the binary \a data using the specified \a
    format and conversion \a flags.
*/


/*!
    Saves the pixmap to the file with the given \a fileName using the
    specified image file \a format and \a quality factor. Returns true
    if successful; otherwise returns false.

    The \a quality factor must be in the range [0,100] or -1. Specify
    0 to obtain small compressed files, 100 for large uncompressed
    files, and -1 to use the default settings.

    If \a format is 0, an image format will be chosen from \a fileName's
    suffix.

    \sa {QPixmap#Reading and Writing Image Files}{Reading and Writing
    Image Files}
*/

bool QPixmap::save(const QString &fileName, const char *format, int quality) const
{
   if (isNull()) {
      return false;   // nothing to save
   }
   QImageWriter writer(fileName, format);
   return doImageIO(&writer, quality);
}

/*!
    \overload

    This function writes a QPixmap to the given \a device using the
    specified image file \a format and \a quality factor. This can be
    used, for example, to save a pixmap directly into a QByteArray:

    \snippet doc/src/snippets/image/image.cpp 1
*/

bool QPixmap::save(QIODevice *device, const char *format, int quality) const
{
   if (isNull()) {
      return false;   // nothing to save
   }
   QImageWriter writer(device, format);
   return doImageIO(&writer, quality);
}

/*! \internal
*/
bool QPixmap::doImageIO(QImageWriter *writer, int quality) const
{
   if (quality > 100  || quality < -1) {
      qWarning("QPixmap::save: quality out of range [-1,100]");
   }
   if (quality >= 0) {
      writer->setQuality(qMin(quality, 100));
   }
   return writer->write(toImage());
}


// The implementation (and documentation) of
// QPixmap::fill(const QWidget *, const QPoint &)
// is in qwidget.cpp

/*!
    \fn void QPixmap::fill(const QWidget *widget, int x, int y)
    \overload

    Fills the pixmap with the \a widget's background color or pixmap.
    The given point, (\a x, \a y), defines an offset in widget
    coordinates to which the pixmap's top-left pixel will be mapped
    to.
*/

/*!
    Fills the pixmap with the given \a color.

    The effect of this function is undefined when the pixmap is
    being painted on.

    \sa {QPixmap#Pixmap Transformations}{Pixmap Transformations}
*/

void QPixmap::fill(const QColor &color)
{
   if (isNull()) {
      return;
   }

   // Some people are probably already calling fill while a painter is active, so to not break
   // their programs, only print a warning and return when the fill operation could cause a crash.
   if (paintingActive() && (color.alpha() != 255) && !hasAlphaChannel()) {
      qWarning("QPixmap::fill: Cannot fill while pixmap is being painted on");
      return;
   }

   if (data->ref.load() == 1) {
      // detach() will also remove this pixmap from caches, so
      // it has to be called even when ref == 1.
      detach();
   } else {
      // Don't bother to make a copy of the data object, since
      // it will be filled with new pixel data anyway.
      QPixmapData *d = data->createCompatiblePixmapData();
      d->resize(data->width(), data->height());
      data = d;
   }
   data->fill(color);
}

/*! \obsolete
    Returns a number that identifies the contents of this QPixmap
    object. Distinct QPixmap objects can only have the same serial
    number if they refer to the same contents (but they don't have
    to).

    Use cacheKey() instead.

    \warning The serial number doesn't necessarily change when
    the pixmap is altered. This means that it may be dangerous to use
    it as a cache key. For caching pixmaps, we recommend using the
    QPixmapCache class whenever possible.
*/
int QPixmap::serialNumber() const
{
   if (isNull()) {
      return 0;
   }
   return data->serialNumber();
}

/*!
    Returns a number that identifies this QPixmap. Distinct QPixmap
    objects can only have the same cache key if they refer to the same
    contents.

    The cacheKey() will change when the pixmap is altered.
*/
qint64 QPixmap::cacheKey() const
{
   if (isNull()) {
      return 0;
   }

   Q_ASSERT(data);
   return data->cacheKey();
}

static void sendResizeEvents(QWidget *target)
{
   QResizeEvent e(target->size(), QSize());
   QApplication::sendEvent(target, &e);

   const QObjectList children = target->children();
   for (int i = 0; i < children.size(); ++i) {
      QWidget *child = static_cast<QWidget *>(children.at(i));
      if (child->isWidgetType() && !child->isWindow() && child->testAttribute(Qt::WA_PendingResizeEvent)) {
         sendResizeEvents(child);
      }
   }
}

/*!
    \fn QPixmap QPixmap::grabWidget(QWidget * widget, const QRect &rectangle)

    Creates a pixmap and paints the given \a widget, restricted by the
    given \a rectangle, in it. If the \a widget has any children, then
    they are also painted in the appropriate positions.

    If no rectangle is specified (the default) the entire widget is
    painted.

    If \a widget is 0, the specified rectangle doesn't overlap the
    widget's rectangle, or an error occurs, the function will return a
    null QPixmap.  If the rectangle is a superset of the given \a
    widget, the areas outside the \a widget are covered with the
    widget's background.

    This function actually asks \a widget to paint itself (and its
    children to paint themselves) by calling paintEvent() with painter
    redirection turned on. But QPixmap also provides the grabWindow()
    function which is a bit faster by grabbing pixels directly off the
    screen. In addition, if there are overlaying windows,
    grabWindow(), unlike grabWidget(), will see them.

    \warning Do not grab a widget from its QWidget::paintEvent().
    However, it is safe to grab a widget from another widget's
    \l {QWidget::}{paintEvent()}.

    \sa grabWindow()
*/

QPixmap QPixmap::grabWidget(QWidget *widget, const QRect &rect)
{
   if (!widget) {
      return QPixmap();
   }

   if (widget->testAttribute(Qt::WA_PendingResizeEvent) || !widget->testAttribute(Qt::WA_WState_Created)) {
      sendResizeEvents(widget);
   }

   widget->d_func()->prepareToRender(QRegion(),
                                     QWidget::DrawWindowBackground | QWidget::DrawChildren | QWidget::IgnoreMask);

   QRect r(rect);
   if (r.width() < 0) {
      r.setWidth(widget->width() - rect.x());
   }
   if (r.height() < 0) {
      r.setHeight(widget->height() - rect.y());
   }

   if (!r.intersects(widget->rect())) {
      return QPixmap();
   }

   QPixmap res(r.size());
   if (!qt_widget_private(widget)->isOpaque) {
      res.fill(Qt::transparent);
   }

   widget->d_func()->render(&res, QPoint(), r, QWidget::DrawWindowBackground
                            | QWidget::DrawChildren | QWidget::IgnoreMask, true);
   return res;
}

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
Qt::HANDLE QPixmap::handle() const
{
#if defined(Q_WS_X11)
   const QPixmapData *pd = pixmapData();
   if (pd && pd->classId() == QPixmapData::X11Class) {
      return static_cast<const QX11PixmapData *>(pd)->handle();
   }
#endif
   return 0;
}
#endif


/*****************************************************************************
  QPixmap stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM)
/*!
    \relates QPixmap

    Writes the given \a pixmap to the given \a stream as a PNG
    image. Note that writing the stream to a file will not produce a
    valid image file.

    \sa QPixmap::save(), {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &stream, const QPixmap &pixmap)
{
   return stream << pixmap.toImage();
}

/*!
    \relates QPixmap

    Reads an image from the given \a stream into the given \a pixmap.

    \sa QPixmap::load(), {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &stream, QPixmap &pixmap)
{
   QImage image;
   stream >> image;

   if (image.isNull()) {
      pixmap = QPixmap();
   } else if (image.depth() == 1) {
      pixmap = QBitmap::fromImage(image);
   } else {
      pixmap = QPixmap::fromImage(image);
   }
   return stream;
}

#endif // QT_NO_DATASTREAM


/*!
    \internal
*/

bool QPixmap::isDetached() const
{
   return data && data->ref.load() == 1;
}


/*!
    \fn QImage QPixmap::convertToImage() const

    Use the toImage() function instead.
*/

/*!
    Replaces this pixmap's data with the given \a image using the
    specified \a flags to control the conversion.  The \a flags
    argument is a bitwise-OR of the \l{Qt::ImageConversionFlags}.
    Passing 0 for \a flags sets all the default options. Returns true
    if the result is that this pixmap is not null.

    Note: this function was part of Qt 3 support in Qt 4.6 and earlier.
    It has been promoted to official API status in 4.7 to support updating
    the pixmap's image without creating a new QPixmap as fromImage() would.

    \sa fromImage()
    \since 4.7
*/
bool QPixmap::convertFromImage(const QImage &image, Qt::ImageConversionFlags flags)
{
   if (image.isNull() || !data) {
      *this = QPixmap::fromImage(image, flags);
   } else {
      data->fromImage(image, flags);
   }
   return !isNull();
}

/*!
    \fn QPixmap QPixmap::xForm(const QMatrix &matrix) const

    Use transformed() instead.
*/

/*!
    \fn QPixmap QPixmap::scaled(int width, int height,
    Qt::AspectRatioMode aspectRatioMode, Qt::TransformationMode
    transformMode) const

    \overload

    Returns a copy of the pixmap scaled to a rectangle with the given
    \a width and \a height according to the given \a aspectRatioMode and
    \a transformMode.

    If either the \a width or the \a height is zero or negative, this
    function returns a null pixmap.
*/

/*!
    \fn QPixmap QPixmap::scaled(const QSize &size, Qt::AspectRatioMode
    aspectRatioMode, Qt::TransformationMode transformMode) const

    Scales the pixmap to the given \a size, using the aspect ratio and
    transformation modes specified by \a aspectRatioMode and \a
    transformMode.

    \image qimage-scaling.png

    \list
    \i If \a aspectRatioMode is Qt::IgnoreAspectRatio, the pixmap
       is scaled to \a size.
    \i If \a aspectRatioMode is Qt::KeepAspectRatio, the pixmap is
       scaled to a rectangle as large as possible inside \a size, preserving the aspect ratio.
    \i If \a aspectRatioMode is Qt::KeepAspectRatioByExpanding,
       the pixmap is scaled to a rectangle as small as possible
       outside \a size, preserving the aspect ratio.
    \endlist

    If the given \a size is empty, this function returns a null
    pixmap.


    In some cases it can be more beneficial to draw the pixmap to a
    painter with a scale set rather than scaling the pixmap. This is
    the case when the painter is for instance based on OpenGL or when
    the scale factor changes rapidly.

    \sa isNull(), {QPixmap#Pixmap Transformations}{Pixmap
    Transformations}

*/
QPixmap QPixmap::scaled(const QSize &s, Qt::AspectRatioMode aspectMode, Qt::TransformationMode mode) const
{
   if (isNull()) {
      qWarning("QPixmap::scaled: Pixmap is a null pixmap");
      return QPixmap();
   }
   if (s.isEmpty()) {
      return QPixmap();
   }

   QSize newSize = size();
   newSize.scale(s, aspectMode);
   newSize.rwidth() = qMax(newSize.width(), 1);
   newSize.rheight() = qMax(newSize.height(), 1);
   if (newSize == size()) {
      return *this;
   }

   QTransform wm = QTransform::fromScale((qreal)newSize.width() / width(),
                                         (qreal)newSize.height() / height());
   QPixmap pix = transformed(wm, mode);
   return pix;
}

/*!
    \fn QPixmap QPixmap::scaledToWidth(int width, Qt::TransformationMode
    mode) const

    Returns a scaled copy of the image. The returned image is scaled
    to the given \a width using the specified transformation \a mode.
    The height of the pixmap is automatically calculated so that the
    aspect ratio of the pixmap is preserved.

    If \a width is 0 or negative, a null pixmap is returned.

    \sa isNull(), {QPixmap#Pixmap Transformations}{Pixmap
    Transformations}
*/
QPixmap QPixmap::scaledToWidth(int w, Qt::TransformationMode mode) const
{
   if (isNull()) {
      qWarning("QPixmap::scaleWidth: Pixmap is a null pixmap");
      return copy();
   }
   if (w <= 0) {
      return QPixmap();
   }

   qreal factor = (qreal) w / width();
   QTransform wm = QTransform::fromScale(factor, factor);
   return transformed(wm, mode);
}

/*!
    \fn QPixmap QPixmap::scaledToHeight(int height,
    Qt::TransformationMode mode) const

    Returns a scaled copy of the image. The returned image is scaled
    to the given \a height using the specified transformation \a mode.
    The width of the pixmap is automatically calculated so that the
    aspect ratio of the pixmap is preserved.

    If \a height is 0 or negative, a null pixmap is returned.

    \sa isNull(), {QPixmap#Pixmap Transformations}{Pixmap
    Transformations}
*/
QPixmap QPixmap::scaledToHeight(int h, Qt::TransformationMode mode) const
{
   if (isNull()) {
      qWarning("QPixmap::scaleHeight: Pixmap is a null pixmap");
      return copy();
   }
   if (h <= 0) {
      return QPixmap();
   }

   qreal factor = (qreal) h / height();
   QTransform wm = QTransform::fromScale(factor, factor);
   return transformed(wm, mode);
}

/*!
    Returns a copy of the pixmap that is transformed using the given
    transformation \a transform and transformation \a mode. The original
    pixmap is not changed.

    The transformation \a transform is internally adjusted to compensate
    for unwanted translation; i.e. the pixmap produced is the smallest
    pixmap that contains all the transformed points of the original
    pixmap. Use the trueMatrix() function to retrieve the actual
    matrix used for transforming the pixmap.

    This function is slow because it involves transformation to a
    QImage, non-trivial computations and a transformation back to a
    QPixmap.

    \sa trueMatrix(), {QPixmap#Pixmap Transformations}{Pixmap
    Transformations}
*/
QPixmap QPixmap::transformed(const QTransform &transform,
                             Qt::TransformationMode mode) const
{
   if (isNull() || transform.type() <= QTransform::TxTranslate) {
      return *this;
   }

   return data->transformed(transform, mode);
}

/*!
  \overload

  This convenience function loads the \a matrix into a
  QTransform and calls the overloaded function.
 */
QPixmap QPixmap::transformed(const QMatrix &matrix, Qt::TransformationMode mode) const
{
   return transformed(QTransform(matrix), mode);
}

bool QPixmap::hasAlpha() const
{
#if defined(Q_WS_X11)
   if (data && data->hasAlphaChannel()) {
      return true;
   }

   QPixmapData *pd = pixmapData();

   if (pd && pd->classId() == QPixmapData::X11Class) {
      QX11PixmapData *x11Data = static_cast<QX11PixmapData *>(pd);

#ifndef QT_NO_XRENDER
      if (x11Data->picture && x11Data->d == 32) {
         return true;
      }
#endif
      if (x11Data->d == 1 || x11Data->x11_mask) {
         return true;
      }
   }
   return false;

#else
   return data && data->hasAlphaChannel();

#endif
}

bool QPixmap::hasAlphaChannel() const
{
   return data && data->hasAlphaChannel();
}

/*!
    \internal
*/
int QPixmap::metric(PaintDeviceMetric metric) const
{
   return data ? data->metric(metric) : 0;
}

void QPixmap::setAlphaChannel(const QPixmap &alphaChannel)
{
   if (alphaChannel.isNull()) {
      return;
   }

   if (paintingActive()) {
      qWarning("QPixmap::setAlphaChannel: "
               "Cannot set alpha channel while pixmap is being painted on");
      return;
   }

   if (width() != alphaChannel.width() && height() != alphaChannel.height()) {
      qWarning("QPixmap::setAlphaChannel: "
               "The pixmap and the alpha channel pixmap must have the same size");
      return;
   }

   detach();
   data->setAlphaChannel(alphaChannel);
}

/*!
    \obsolete

    Returns the alpha channel of the pixmap as a new grayscale QPixmap in which
    each pixel's red, green, and blue values are given the alpha value of the
    original pixmap. The color depth of the returned pixmap is the system depth
    on X11 and 8-bit on Windows and Mac OS X.

    You can use this function while debugging
    to get a visible image of the alpha channel. If the pixmap doesn't have an
    alpha channel, i.e., the alpha channel's value for all pixels equals
    0xff), a null pixmap is returned. You can check this with the \c isNull()
    function.

    We show an example:

    \snippet doc/src/snippets/alphachannel.cpp 0

    \image alphachannelimage.png The pixmap and channelImage QPixmaps

    \warning This is an expensive operation. The alpha channel of the
    pixmap is extracted dynamically from the pixeldata. Most usecases of this
    function are covered by QPainter and compositionModes which will normally
    execute faster.

    \sa setAlphaChannel(), {QPixmap#Pixmap Information}{Pixmap
    Information}
*/
QPixmap QPixmap::alphaChannel() const
{
   return data ? data->alphaChannel() : QPixmap();
}

/*!
    \internal
*/
QPaintEngine *QPixmap::paintEngine() const
{
   return data ? data->paintEngine() : 0;
}

QBitmap QPixmap::mask() const
{
   return data ? data->mask() : QBitmap();
}

int QPixmap::defaultDepth()
{
#if defined(Q_WS_QWS)
   return QScreen::instance()->depth();

#elif defined(Q_WS_X11)
   return QX11Info::appDepth();

#elif defined(Q_OS_WIN)
   return 32;

#elif defined(Q_OS_MAC)
   return 32;

#elif defined(Q_WS_QPA)
   return 32;       //LITE: use graphicssystem (we should do that in general)

#endif
}

/*!
    Detaches the pixmap from shared pixmap data.

    A pixmap is automatically detached by Qt whenever its contents are
    about to change. This is done in almost all QPixmap member
    functions that modify the pixmap (fill(), fromImage(),
    load(), etc.), and in QPainter::begin() on a pixmap.

    There are two exceptions in which detach() must be called
    explicitly, that is when calling the handle() or the
    x11PictureHandle() function (only available on X11). Otherwise,
    any modifications done using system calls, will be performed on
    the shared data.

    The detach() function returns immediately if there is just a
    single reference or if the pixmap has not been initialized yet.
*/
void QPixmap::detach()
{
   if (!data) {
      return;
   }

   // QPixmap.data member may be QRuntimePixmapData so use pixmapData() function to get
   // the actual underlaying runtime pixmap data.
   QPixmapData *pd = pixmapData();
   QPixmapData::ClassId id = pd->classId();
   if (id == QPixmapData::RasterClass) {
      QRasterPixmapData *rasterData = static_cast<QRasterPixmapData *>(pd);
      rasterData->image.detach();
   }

   if (data->is_cached && data->ref.load() == 1) {
      QImagePixmapCleanupHooks::executePixmapDataModificationHooks(data.data());
   }

#if defined(Q_OS_MAC)
   QMacPixmapData *macData = id == QPixmapData::MacClass ? static_cast<QMacPixmapData *>(pd) : 0;
   if (macData) {
      if (macData->cg_mask) {
         CGImageRelease(macData->cg_mask);
         macData->cg_mask = 0;
      }
   }
#endif

   if (data->ref.load() != 1) {
      *this = copy();
   }
   ++data->detach_no;

#if defined(Q_WS_X11)
   if (pd->classId() == QPixmapData::X11Class) {
      QX11PixmapData *d = static_cast<QX11PixmapData *>(pd);
      d->flags &= ~QX11PixmapData::Uninitialized;

      // reset the cache data
      if (d->hd2) {
         XFreePixmap(X11->display, d->hd2);
         d->hd2 = 0;
      }
   }
#elif defined(Q_OS_MAC)
   if (macData) {
      macData->macReleaseCGImageRef();
      macData->uninit = false;
   }
#endif
}

/*!
    \fn QPixmap QPixmap::fromImage(const QImage &image, Qt::ImageConversionFlags flags)

    Converts the given \a image to a pixmap using the specified \a
    flags to control the conversion.  The \a flags argument is a
    bitwise-OR of the \l{Qt::ImageConversionFlags}. Passing 0 for \a
    flags sets all the default options.

    In case of monochrome and 8-bit images, the image is first
    converted to a 32-bit pixmap and then filled with the colors in
    the color table. If this is too expensive an operation, you can
    use QBitmap::fromImage() instead.

    \sa fromImageReader(), toImage(), {QPixmap#Pixmap Conversion}{Pixmap Conversion}
*/
QPixmap QPixmap::fromImage(const QImage &image, Qt::ImageConversionFlags flags)
{
   if (image.isNull()) {
      return QPixmap();
   }

   QGraphicsSystem *gs = QApplicationPrivate::graphicsSystem();
   QScopedPointer<QPixmapData> data(gs ? gs->createPixmapData(QPixmapData::PixmapType)
                                    : QGraphicsSystem::createDefaultPixmapData(QPixmapData::PixmapType));
   data->fromImage(image, flags);
   return QPixmap(data.take());
}

/*!
    \fn QPixmap QPixmap::fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags)

    Create a QPixmap from an image read directly from an \a imageReader.
    The \a flags argument is a bitwise-OR of the \l{Qt::ImageConversionFlags}.
    Passing 0 for \a flags sets all the default options.

    On some systems, reading an image directly to QPixmap can use less memory than
    reading a QImage to convert it to QPixmap.

    \sa fromImage(), toImage(), {QPixmap#Pixmap Conversion}{Pixmap Conversion}
*/
QPixmap QPixmap::fromImageReader(QImageReader *imageReader, Qt::ImageConversionFlags flags)
{
   QGraphicsSystem *gs = QApplicationPrivate::graphicsSystem();
   QScopedPointer<QPixmapData> data(gs ? gs->createPixmapData(QPixmapData::PixmapType)
                                    : QGraphicsSystem::createDefaultPixmapData(QPixmapData::PixmapType));
   data->fromImageReader(imageReader, flags);
   return QPixmap(data.take());
}

/*!
    \fn QPixmap QPixmap::grabWindow(WId window, int x, int y, int
    width, int height)

    Creates and returns a pixmap constructed by grabbing the contents
    of the given \a window restricted by QRect(\a x, \a y, \a width,
    \a height).

    The arguments (\a{x}, \a{y}) specify the offset in the window,
    whereas (\a{width}, \a{height}) specify the area to be copied.  If
    \a width is negative, the function copies everything to the right
    border of the window. If \a height is negative, the function
    copies everything to the bottom of the window.

    The window system identifier (\c WId) can be retrieved using the
    QWidget::winId() function. The rationale for using a window
    identifier and not a QWidget, is to enable grabbing of windows
    that are not part of the application, window system frames, and so
    on.

    The grabWindow() function grabs pixels from the screen, not from
    the window, i.e. if there is another window partially or entirely
    over the one you grab, you get pixels from the overlying window,
    too. The mouse cursor is generally not grabbed.

    Note on X11 that if the given \a window doesn't have the same depth
    as the root window, and another window partially or entirely
    obscures the one you grab, you will \e not get pixels from the
    overlying window.  The contents of the obscured areas in the
    pixmap will be undefined and uninitialized.

    On Windows Vista and above grabbing a layered window, which is
    created by setting the Qt::WA_TranslucentBackground attribute, will
    not work. Instead grabbing the desktop widget should work.

    \warning In general, grabbing an area outside the screen is not
    safe. This depends on the underlying window system.

    \sa grabWidget(), {Screenshot Example}
*/

/*!
  \internal
*/
QPixmapData *QPixmap::pixmapData() const
{
   if (data) {
      QPixmapData *pm = data.data();
      return pm->runtimeData() ? pm->runtimeData() : pm;
   }

   return 0;
}


/*!
    \enum QPixmap::HBitmapFormat

    \bold{Win32 only:} This enum defines how the conversion between \c
    HBITMAP and QPixmap is performed.

    \warning This enum is only available on Windows.

    \value NoAlpha The alpha channel is ignored and always treated as
    being set to fully opaque. This is preferred if the \c HBITMAP is
    used with standard GDI calls, such as \c BitBlt().

    \value PremultipliedAlpha The \c HBITMAP is treated as having an
    alpha channel and premultiplied colors. This is preferred if the
    \c HBITMAP is accessed through the \c AlphaBlend() GDI function.

    \value Alpha The \c HBITMAP is treated as having a plain alpha
    channel. This is the preferred format if the \c HBITMAP is going
    to be used as an application icon or systray icon.

    \sa fromWinHBITMAP(), toWinHBITMAP()
*/

/*! \fn HBITMAP QPixmap::toWinHBITMAP(HBitmapFormat format) const
    \bold{Win32 only:} Creates a \c HBITMAP equivalent to the QPixmap,
    based on the given \a format. Returns the \c HBITMAP handle.

    It is the caller's responsibility to free the \c HBITMAP data
    after use.

    \warning This function is only available on Windows.

    \sa fromWinHBITMAP(), {QPixmap#Pixmap Conversion}{Pixmap Conversion}
*/

/*! \fn QPixmap QPixmap::fromWinHBITMAP(HBITMAP bitmap, HBitmapFormat format)
    \bold{Win32 only:} Returns a QPixmap that is equivalent to the
    given \a bitmap. The conversion is based on the specified \a
    format.

    \warning This function is only available on Windows.

    \sa toWinHBITMAP(), {QPixmap#Pixmap Conversion}{Pixmap Conversion}

*/

/*! \fn HICON QPixmap::toWinHICON() const
    \since 4.6

    \bold{Win32 only:} Creates a \c HICON equivalent to the QPixmap.
    Returns the \c HICON handle.

    It is the caller's responsibility to free the \c HICON data after use.

    \warning This function is only available on Windows.

    \sa fromWinHICON(), {QPixmap#Pixmap Conversion}{Pixmap Conversion}
*/

/*! \fn QPixmap QPixmap::fromWinHICON(HICON icon)
    \since 4.6

    \bold{Win32 only:} Returns a QPixmap that is equivalent to the given
    \a icon.

    \warning This function is only available on Windows.

    \sa toWinHICON(), {QPixmap#Pixmap Conversion}{Pixmap Conversion}

*/

/*! \fn const QX11Info &QPixmap::x11Info() const
    \bold{X11 only:} Returns information about the configuration of
    the X display used by the screen to which the pixmap currently belongs.

    \warning This function is only available on X11.

    \sa {QPixmap#Pixmap Information}{Pixmap Information}
*/

/*! \fn Qt::HANDLE QPixmap::x11PictureHandle() const
    \bold{X11 only:} Returns the X11 Picture handle of the pixmap for
    XRender support.

    This function will return 0 if XRender support is not compiled
    into Qt, if the XRender extension is not supported on the X11
    display, or if the handle could not be created. Use of this
    function is not portable.

    \warning This function is only available on X11.

    \sa {QPixmap#Pixmap Information}{Pixmap Information}
*/

/*! \fn int QPixmap::x11SetDefaultScreen(int screen)
  \internal
*/

/*! \fn void QPixmap::x11SetScreen(int screen)
  \internal
*/

/*! \fn QRgb* QPixmap::clut() const
    \internal
*/

/*! \fn int QPixmap::numCols() const
    \obsolete
    \internal
    \sa colorCount()
*/

/*! \fn int QPixmap::colorCount() const
    \since 4.6
    \internal
*/

/*! \fn const uchar* QPixmap::qwsBits() const
    \internal
    \since 4.1
*/

/*! \fn int QPixmap::qwsBytesPerLine() const
    \internal
    \since 4.1
*/

QT_END_NAMESPACE
