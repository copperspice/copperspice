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

#include <qbrush.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpixmapcache.h>
#include <qplatform_pixmap.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <qline.h>
#include <qdebug.h>
#include <qcoreapplication.h>
#include <qhexstring_p.h>
#include <qnumeric.h>

static void qt_cleanup_brush_pattern_image_cache();

const uchar *qt_patternForBrush(int brushStyle, bool invert)
{
   Q_ASSERT(brushStyle > Qt::SolidPattern && brushStyle < Qt::LinearGradientPattern);
   static const uchar pat_tbl[][2][8] = {
      {
         /* dense1 */ { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 },
         /*~dense1 */ { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff },
      }, {
         /* dense2 */ { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 },
         /*~dense2 */ { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff },
      }, {
         /* dense3 */ { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 },
         /*~dense3 */ { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee },
      }, {
         /* dense4 */ { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa },
         /*~dense4 */ { 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 },
      }, {
         /* dense5 */ { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee },
         /*~dense5 */ { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 },
      }, {
         /* dense6 */ { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff },
         /*~dense6 */ { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 },
      }, {
         /* dense7 */ { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff },
         /*~dense7 */ { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 },
      }, {
         /* hor */    { 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff },
         /*~hor */    { 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00 },
      }, {
         /* ver */    { 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef },
         /*~ver */    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 },
      }, {
         /* cross */  { 0xef, 0xef, 0xef, 0x00, 0xef, 0xef, 0xef, 0xef },
         /*~cross */  { 0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 0x10, 0x10 },
      }, {
         /* bdiag */  { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe },
         /*~bdiag */  { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 },
      }, {
         /* fdiag */  { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f },
         /*~fdiag */  { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 },
      }, {
         /* dcross */ { 0x7e, 0xbd, 0xdb, 0xe7, 0xe7, 0xdb, 0xbd, 0x7e },
         /*~dcross */ { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 },
      },
   };
   return pat_tbl[brushStyle - Qt::Dense1Pattern][invert];
}

QPixmap qt_pixmapForBrush(int brushStyle, bool invert)
{
   QPixmap pm;
   QString key = "$cs_brush$" + HexString<uint>(brushStyle) + QChar(invert ? '1' : '0');

   if (! QPixmapCache::find(key, pm)) {
      pm = QBitmap::fromData(QSize(8, 8), qt_patternForBrush(brushStyle, invert), QImage::Format_MonoLSB);
      QPixmapCache::insert(key, pm);
   }

   return pm;
}

class QBrushPatternImageCache
{
 public:
   QBrushPatternImageCache()
      : m_initialized(false) {
      init();
   }

   void init() {
      qAddPostRoutine(qt_cleanup_brush_pattern_image_cache);

      for (int style = Qt::Dense1Pattern; style <= Qt::DiagCrossPattern; ++style) {
         int i = style - Qt::Dense1Pattern;
         m_images[i][0] = QImage(qt_patternForBrush(style, 0), 8, 8, 1, QImage::Format_MonoLSB);
         m_images[i][1] = QImage(qt_patternForBrush(style, 1), 8, 8, 1, QImage::Format_MonoLSB);
      }
      m_initialized = true;
   }

   QImage getImage(int brushStyle, bool invert) const {
      Q_ASSERT(brushStyle >= Qt::Dense1Pattern && brushStyle <= Qt::DiagCrossPattern);

      if (!m_initialized) {
         const_cast<QBrushPatternImageCache *>(this)->init();
      }
      return m_images[brushStyle - Qt::Dense1Pattern][invert];
   }

   void cleanup() {
      for (int style = Qt::Dense1Pattern; style <= Qt::DiagCrossPattern; ++style) {
         int i = style - Qt::Dense1Pattern;
         m_images[i][0] = QImage();
         m_images[i][1] = QImage();
      }
      m_initialized = false;
   }

 private:
   QImage m_images[Qt::DiagCrossPattern - Qt::Dense1Pattern + 1][2];
   bool m_initialized;
};

Q_GLOBAL_STATIC(QBrushPatternImageCache, qt_brushPatternImageCache)

static void qt_cleanup_brush_pattern_image_cache()
{
   qt_brushPatternImageCache()->cleanup();
}

Q_GUI_EXPORT QImage qt_imageForBrush(int brushStyle, bool invert)
{
   return qt_brushPatternImageCache()->getImage(brushStyle, invert);
}

struct QTexturedBrushData : public QBrushData {
   QTexturedBrushData() {
      m_has_pixmap_texture = false;
      m_pixmap = 0;
   }
   ~QTexturedBrushData() {
      delete m_pixmap;
   }

   void setPixmap(const QPixmap &pm) {
      delete m_pixmap;

      if (pm.isNull()) {
         m_pixmap = 0;
         m_has_pixmap_texture = false;
      } else {
         m_pixmap = new QPixmap(pm);
         m_has_pixmap_texture = true;
      }

      m_image = QImage();
   }

   void setImage(const QImage &image) {
      m_image = image;
      delete m_pixmap;
      m_pixmap = 0;
      m_has_pixmap_texture = false;
   }

   QPixmap &pixmap() {
      if (!m_pixmap) {
         m_pixmap = new QPixmap(QPixmap::fromImage(m_image));
      }
      return *m_pixmap;
   }

   QImage &image() {
      if (m_image.isNull() && m_pixmap) {
         m_image = m_pixmap->toImage();
      }
      return m_image;
   }

   QPixmap *m_pixmap;
   QImage m_image;
   bool m_has_pixmap_texture;
};

// returns true if the brush has a pixmap (or bitmap) set as the
// brush texture, false otherwise
bool Q_GUI_EXPORT qHasPixmapTexture(const QBrush &brush)
{
   if (brush.style() != Qt::TexturePattern) {
      return false;
   }
   QTexturedBrushData *tx_data = static_cast<QTexturedBrushData *>(brush.d.data());
   return tx_data->m_has_pixmap_texture;
}

struct QGradientBrushData : public QBrushData {
   QGradient gradient;
};

struct QBrushDataPointerDeleter {
   static inline void deleteData(QBrushData *d) {
      switch (d->style) {
         case Qt::TexturePattern:
            delete static_cast<QTexturedBrushData *>(d);
            break;
         case Qt::LinearGradientPattern:
         case Qt::RadialGradientPattern:
         case Qt::ConicalGradientPattern:
            delete static_cast<QGradientBrushData *>(d);
            break;
         default:
            delete d;
      }
   }

   static inline void cleanup(QBrushData *d) {
      if (d && !d->ref.deref()) {
         deleteData(d);
      }
   }
};


class QNullBrushData
{
 public:
   QBrushData *brush;
   QNullBrushData() : brush(new QBrushData) {
      brush->ref.store(1);
      brush->style = Qt::BrushStyle(0);
      brush->color = Qt::black;
   }
   ~QNullBrushData() {
      if (!brush->ref.deref()) {
         delete brush;
      }
      brush = 0;
   }
};

Q_GLOBAL_STATIC(QNullBrushData, nullBrushInstance_holder)

static QBrushData *nullBrushInstance()
{
   return nullBrushInstance_holder()->brush;
}

static bool qbrush_check_type(Qt::BrushStyle style)
{
   switch (style) {
      case Qt::TexturePattern:
         qWarning("QBrush: Incorrect use of TexturePattern");
         break;
      case Qt::LinearGradientPattern:
      case Qt::RadialGradientPattern:
      case Qt::ConicalGradientPattern:
         qWarning("QBrush: Wrong use of a gradient pattern");
         break;
      default:
         return true;
   }
   return false;
}

/*!
  \internal
  Initializes the brush.
*/

void QBrush::init(const QColor &color, Qt::BrushStyle style)
{
   switch (style) {
      case Qt::NoBrush:
         d.reset(nullBrushInstance());
         d->ref.ref();
         if (d->color != color) {
            setColor(color);
         }

         return;
      case Qt::TexturePattern:
         d.reset(new QTexturedBrushData);
         break;
      case Qt::LinearGradientPattern:
      case Qt::RadialGradientPattern:
      case Qt::ConicalGradientPattern:
         d.reset(new QGradientBrushData);
         break;
      default:
         d.reset(new QBrushData);
         break;
   }
   d->ref.store(1);
   d->style = style;
   d->color = color;
}

QBrush::QBrush()
   : d(nullBrushInstance())
{
   Q_ASSERT(d);
   d->ref.ref();
}

QBrush::QBrush(const QPixmap &pixmap)
{
   init(Qt::black, Qt::TexturePattern);
   setTexture(pixmap);
}


QBrush::QBrush(const QImage &image)
{
   init(Qt::black, Qt::TexturePattern);
   setTextureImage(image);
}

/*!
    Constructs a black brush with the given \a style.

    \sa setStyle()
*/

QBrush::QBrush(Qt::BrushStyle style)
{
   if (qbrush_check_type(style)) {
      init(Qt::black, style);
   } else {
      d.reset(nullBrushInstance());
      d->ref.ref();
   }
}

/*!
    Constructs a brush with the given \a color and \a style.

    \sa setColor(), setStyle()
*/

QBrush::QBrush(const QColor &color, Qt::BrushStyle style)
{
   if (qbrush_check_type(style)) {
      init(color, style);
   } else {
      d.reset(nullBrushInstance());
      d->ref.ref();
   }
}

/*!
    \fn QBrush::QBrush(Qt::GlobalColor color, Qt::BrushStyle style)

    Constructs a brush with the given \a color and \a style.

    \sa setColor(), setStyle()
*/
QBrush::QBrush(Qt::GlobalColor color, Qt::BrushStyle style)
{
   if (qbrush_check_type(style)) {
      init(color, style);
   } else {
      d.reset(nullBrushInstance());
      d->ref.ref();
   }
}

/*!
    Constructs a brush with the given \a color and the custom pattern
    stored in \a pixmap.

    The style is set to Qt::TexturePattern. The color will only have
    an effect for QBitmaps.

    \sa setColor(), setPixmap()
*/

QBrush::QBrush(const QColor &color, const QPixmap &pixmap)
{
   init(color, Qt::TexturePattern);
   setTexture(pixmap);
}

/*!

    Constructs a brush with the given \a color and the custom pattern
    stored in \a pixmap.

    The style is set to Qt::TexturePattern. The color will only have
    an effect for QBitmaps.

    \sa setColor(), setPixmap()
*/
QBrush::QBrush(Qt::GlobalColor color, const QPixmap &pixmap)
{
   init(color, Qt::TexturePattern);
   setTexture(pixmap);
}

/*!
    Constructs a copy of \a other.
*/

QBrush::QBrush(const QBrush &other)
   : d(other.d.data())
{
   d->ref.ref();
}

/*!
    Constructs a brush based on the given \a gradient.

    The brush style is set to the corresponding gradient style (either
    Qt::LinearGradientPattern, Qt::RadialGradientPattern or
    Qt::ConicalGradientPattern).
*/
QBrush::QBrush(const QGradient &gradient)
{
   Q_ASSERT_X(gradient.type() != QGradient::NoGradient, "QBrush::QBrush",
      "QGradient should not be used directly, use the linear, radial\n"
      "or conical gradients instead");

   const Qt::BrushStyle enum_table[] = {
      Qt::LinearGradientPattern,
      Qt::RadialGradientPattern,
      Qt::ConicalGradientPattern
   };

   init(QColor(), enum_table[gradient.type()]);
   QGradientBrushData *grad = static_cast<QGradientBrushData *>(d.data());
   grad->gradient = gradient;
}

/*!
    Destroys the brush.
*/

QBrush::~QBrush()
{
}

void QBrush::cleanUp(QBrushData *x)
{
   QBrushDataPointerDeleter::deleteData(x);
}


void QBrush::detach(Qt::BrushStyle newStyle)
{
   if (newStyle == d->style && d->ref.load() == 1) {
      return;
   }

   QScopedPointer<QBrushData, QBrushDataPointerDeleter> x;

   switch (newStyle) {
      case Qt::TexturePattern: {
         QTexturedBrushData *tbd = new QTexturedBrushData;
         if (d->style == Qt::TexturePattern) {
            QTexturedBrushData *data = static_cast<QTexturedBrushData *>(d.data());
            if (data->m_has_pixmap_texture) {
               tbd->setPixmap(data->pixmap());
            } else {
               tbd->setImage(data->image());
            }
         }
         x.reset(tbd);
         break;
      }
      case Qt::LinearGradientPattern:
      case Qt::RadialGradientPattern:
      case Qt::ConicalGradientPattern: {
         QGradientBrushData *gbd = new QGradientBrushData;
         switch (d->style) {
            case Qt::LinearGradientPattern:
            case Qt::RadialGradientPattern:
            case Qt::ConicalGradientPattern:
               gbd->gradient =
                  static_cast<QGradientBrushData *>(d.data())->gradient;
               break;
            default:
               break;
         }
         x.reset(gbd);
         break;
      }
      default:
         x.reset(new QBrushData);
         break;
   }
   x->ref.store(1);
   x->style = newStyle;
   x->color = d->color;
   x->transform = d->transform;
   d.swap(x);
}

QBrush &QBrush::operator=(const QBrush &b)
{
   if (d == b.d) {
      return *this;
   }

   b.d->ref.ref();
   d.reset(b.d.data());
   return *this;
}


QBrush::operator QVariant() const
{
   return QVariant(QVariant::Brush, this);
}

void QBrush::setStyle(Qt::BrushStyle style)
{
   if (d->style == style) {
      return;
   }

   if (qbrush_check_type(style)) {
      detach(style);
      d->style = style;
   }
}


void QBrush::setColor(const QColor &c)
{
   if (d->color == c) {
      return;
   }
   detach(d->style);
   d->color = c;
}

QPixmap QBrush::texture() const
{
   return d->style == Qt::TexturePattern
      ? (static_cast<QTexturedBrushData *>(d.data()))->pixmap()
      : QPixmap();
}

/*!
    Sets the brush pixmap to \a pixmap. The style is set to
    Qt::TexturePattern.

    The current brush color will only have an effect for monochrome
    pixmaps, i.e. for QPixmap::depth() == 1 (\l {QBitmap}{QBitmaps}).

    \sa texture()
*/

void QBrush::setTexture(const QPixmap &pixmap)
{
   if (!pixmap.isNull()) {
      detach(Qt::TexturePattern);
      QTexturedBrushData *data = static_cast<QTexturedBrushData *>(d.data());
      data->setPixmap(pixmap);
   } else {
      detach(Qt::NoBrush);
   }
}


/*!
    \since 4.2

    Returns the custom brush pattern, or a null image if no custom
    brush pattern has been set.

    If the texture was set as a QPixmap it will be converted to a
    QImage.

    \sa setTextureImage()
*/

QImage QBrush::textureImage() const
{
   return d->style == Qt::TexturePattern
      ? (static_cast<QTexturedBrushData *>(d.data()))->image()
      : QImage();
}


/*!
    \since 4.2

    Sets the brush image to \a image. The style is set to
    Qt::TexturePattern.

    Note the current brush color will \e not have any affect on
    monochrome images, as opposed to calling setTexture() with a
    QBitmap. If you want to change the color of monochrome image
    brushes, either convert the image to QBitmap with \c
    QBitmap::fromImage() and set the resulting QBitmap as a texture,
    or change the entries in the color table for the image.

    \sa textureImage(), setTexture()
*/

void QBrush::setTextureImage(const QImage &image)
{
   if (!image.isNull()) {
      detach(Qt::TexturePattern);
      QTexturedBrushData *data = static_cast<QTexturedBrushData *>(d.data());
      data->setImage(image);
   } else {
      detach(Qt::NoBrush);
   }
}


/*!
    Returns the gradient describing this brush.
*/
const QGradient *QBrush::gradient() const
{
   if (d->style == Qt::LinearGradientPattern
      || d->style == Qt::RadialGradientPattern
      || d->style == Qt::ConicalGradientPattern) {
      return &static_cast<const QGradientBrushData *>(d.data())->gradient;
   }
   return 0;
}

Q_GUI_EXPORT bool qt_isExtendedRadialGradient(const QBrush &brush)
{
   if (brush.style() == Qt::RadialGradientPattern) {
      const QGradient *g = brush.gradient();
      const QRadialGradient *rg = static_cast<const QRadialGradient *>(g);

      if (!qFuzzyIsNull(rg->focalRadius())) {
         return true;
      }

      QPointF delta = rg->focalPoint() - rg->center();
      if (delta.x() * delta.x() + delta.y() * delta.y() > rg->radius() * rg->radius()) {
         return true;
      }
   }

   return false;
}

/*!
    Returns true if the brush is fully opaque otherwise false. A brush
    is considered opaque if:

    \list
    \i The alpha component of the color() is 255.
    \i Its texture() does not have an alpha channel and is not a QBitmap.
    \i The colors in the gradient() all have an alpha component that is 255.
    \i It is an extended radial gradient.
    \endlist
*/

bool QBrush::isOpaque() const
{
   bool opaqueColor = d->color.alpha() == 255;

   // Test awfully simple case first
   if (d->style == Qt::SolidPattern) {
      return opaqueColor;
   }

   if (qt_isExtendedRadialGradient(*this)) {
      return false;
   }

   if (d->style == Qt::LinearGradientPattern
      || d->style == Qt::RadialGradientPattern
      || d->style == Qt::ConicalGradientPattern) {
      QGradientStops stops = gradient()->stops();
      for (int i = 0; i < stops.size(); ++i)
         if (stops.at(i).second.alpha() != 255) {
            return false;
         }
      return true;
   } else if (d->style == Qt::TexturePattern) {
      return qHasPixmapTexture(*this)
         ? !texture().hasAlphaChannel() && !texture().isQBitmap()
         : !textureImage().hasAlphaChannel();
   }

   return false;
}


/*!
    \since 4.2

    Sets \a matrix as an explicit transformation matrix on the
    current brush. The brush transformation matrix is merged with
    QPainter transformation matrix to produce the final result.

    \sa matrix()
*/
void QBrush::setMatrix(const QMatrix &matrix)
{
   setTransform(QTransform(matrix));
}

/*!
    \since 4.3

    Sets \a matrix as an explicit transformation matrix on the
    current brush. The brush transformation matrix is merged with
    QPainter transformation matrix to produce the final result.

    \sa transform()
*/
void QBrush::setTransform(const QTransform &matrix)
{
   detach(d->style);
   d->transform = matrix;
}

bool QBrush::operator==(const QBrush &b) const
{
   if (b.d == d) {
      return true;
   }

   if (b.d->style != d->style || b.d->color != d->color || b.d->transform != d->transform) {
      return false;
   }

   switch (d->style) {
      case Qt::TexturePattern: {
         // Note this produces false negatives if the textures have identical data,
         // but does not share the same data in memory. Since equality is likely to
         // be used to avoid iterating over the data for a texture update, this should
         // still be better than doing an accurate comparison.

         const QPixmap *us = 0, *them = 0;
         qint64 cacheKey1, cacheKey2;

         if (qHasPixmapTexture(*this)) {
            us = (static_cast<QTexturedBrushData *>(d.data()))->m_pixmap;
            cacheKey1 = us->cacheKey();
         } else {
            cacheKey1 = (static_cast<QTexturedBrushData *>(d.data()))->image().cacheKey();
         }

         if (qHasPixmapTexture(b)) {
            them = (static_cast<QTexturedBrushData *>(b.d.data()))->m_pixmap;
            cacheKey2 = them->cacheKey();
         } else {
            cacheKey2 = (static_cast<QTexturedBrushData *>(b.d.data()))->image().cacheKey();
         }

         if (cacheKey1 != cacheKey2) {
            return false;
         }
         if (!us == !them) { // both images or both pixmaps
            return true;
         }
         // Only raster QPixmaps use the same cachekeys as QImages.
         if (us && us->handle()->classId() == QPlatformPixmap::RasterClass) {
            return true;
         }
         if (them && them->handle()->classId() == QPlatformPixmap::RasterClass) {
            return true;
         }
         return false;
      }

      case Qt::LinearGradientPattern:
      case Qt::RadialGradientPattern:
      case Qt::ConicalGradientPattern: {
         const QGradientBrushData *d1 = static_cast<QGradientBrushData *>(d.data());
         const QGradientBrushData *d2 = static_cast<QGradientBrushData *>(b.d.data());
         return d1->gradient == d2->gradient;
      }
      default:
         return true;
   }
}


QDebug operator<<(QDebug dbg, const QBrush &b)
{
   static const char *const BRUSH_STYLES[] = {
      "NoBrush",
      "SolidPattern",
      "Dense1Pattern",
      "Dense2Pattern",
      "Dense3Pattern",
      "Dense4Pattern",
      "Dense5Pattern",
      "Dense6Pattern",
      "Dense7Pattern",
      "HorPattern",
      "VerPattern",
      "CrossPattern",
      "BDiagPattern",
      "FDiagPattern",
      "DiagCrossPattern",
      "LinearGradientPattern",
      "RadialGradientPattern",
      "ConicalGradientPattern",
      0, 0, 0, 0, 0, 0,
      "TexturePattern" // 24
   };

   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QBrush(" << b.color() << ',' << BRUSH_STYLES[b.style()] << ')';

   return dbg;
}

QDataStream &operator<<(QDataStream &s, const QBrush &b)
{
   quint8 style = (quint8) b.style();
   bool gradient_style = false;

   if (style == Qt::LinearGradientPattern || style == Qt::RadialGradientPattern
      || style == Qt::ConicalGradientPattern) {
      gradient_style = true;
   }

   s << style << b.color();

   if (b.style() == Qt::TexturePattern) {
      s << b.textureImage();

   } else if (gradient_style) {
      const QGradient *gradient = b.gradient();

      int type_as_int = int(gradient->type());
      s << type_as_int;

      s << int(gradient->spread());
      s << int(gradient->coordinateMode());
      s << int(gradient->interpolationMode());

      if (sizeof(qreal) == sizeof(double)) {
         s << gradient->stops();

      } else {
         // ensure that we write doubles here instead of streaming the stops
         // directly; otherwise, platforms that redefine qreal might generate
         // data that cannot be read on other platforms.
         QVector<QGradientStop> stops = gradient->stops();
         s << quint32(stops.size());

         for (int i = 0; i < stops.size(); ++i) {
            const QGradientStop &stop = stops.at(i);
            s << QPair<double, QColor>(double(stop.first), stop.second);
         }
      }

      if (gradient->type() == QGradient::LinearGradient) {
         s << static_cast<const QLinearGradient *>(gradient)->start();
         s << static_cast<const QLinearGradient *>(gradient)->finalStop();
      } else if (gradient->type() == QGradient::RadialGradient) {
         s << static_cast<const QRadialGradient *>(gradient)->center();
         s << static_cast<const QRadialGradient *>(gradient)->focalPoint();
         s << (double) static_cast<const QRadialGradient *>(gradient)->radius();
      } else { // type == Conical
         s << static_cast<const QConicalGradient *>(gradient)->center();
         s << (double) static_cast<const QConicalGradient *>(gradient)->angle();
      }
   }

   s << b.transform();

   return s;
}

QDataStream &operator>>(QDataStream &s, QBrush &b)
{
   quint8 style;
   QColor color;

   s >> style;
   s >> color;
   b = QBrush(color);

   if (style == Qt::TexturePattern) {
      QImage img;
      s >> img;
      b.setTextureImage(std::move(img));

   } else if (style == Qt::LinearGradientPattern
      || style == Qt::RadialGradientPattern
      || style == Qt::ConicalGradientPattern) {

      int type_as_int;
      QGradient::Type type;
      QGradientStops stops;
      QGradient::CoordinateMode cmode = QGradient::LogicalMode;
      QGradient::Spread spread = QGradient::PadSpread;
      QGradient::InterpolationMode imode = QGradient::ColorInterpolation;

      s >> type_as_int;
      type = QGradient::Type(type_as_int);

      s >> type_as_int;
      spread = QGradient::Spread(type_as_int);

      s >> type_as_int;
      cmode = QGradient::CoordinateMode(type_as_int);

      s >> type_as_int;
      imode = QGradient::InterpolationMode(type_as_int);


      if (sizeof(qreal) == sizeof(double)) {
         s >> stops;

      } else {
         quint32 numStops;
         double n;
         QColor c;

         s >> numStops;
         for (quint32 i = 0; i < numStops; ++i) {
            s >> n >> c;
            stops << QPair<qreal, QColor>(n, c);
         }
      }

      if (type == QGradient::LinearGradient) {
         QPointF p1, p2;
         s >> p1;
         s >> p2;
         QLinearGradient lg(p1, p2);
         lg.setStops(stops);
         lg.setSpread(spread);
         lg.setCoordinateMode(cmode);
         lg.setInterpolationMode(imode);
         b = QBrush(lg);
      } else if (type == QGradient::RadialGradient) {
         QPointF center, focal;
         double radius;
         s >> center;
         s >> focal;
         s >> radius;
         QRadialGradient rg(center, radius, focal);
         rg.setStops(stops);
         rg.setSpread(spread);
         rg.setCoordinateMode(cmode);
         rg.setInterpolationMode(imode);
         b = QBrush(rg);
      } else { // type == QGradient::ConicalGradient
         QPointF center;
         double angle;
         s >> center;
         s >> angle;
         QConicalGradient cg(center, angle);
         cg.setStops(stops);
         cg.setSpread(spread);
         cg.setCoordinateMode(cmode);
         cg.setInterpolationMode(imode);
         b = QBrush(cg);
      }

   } else {
      b = QBrush(color, (Qt::BrushStyle)style);
   }

   QTransform transform;
   s >> transform;
   b.setTransform(transform);

   return s;
}

QGradient::QGradient()
   : m_type(NoGradient), dummy(0)
{
}

void QGradient::setColorAt(qreal pos, const QColor &color)
{
   if ((pos > 1 || pos < 0) && !qIsNaN(pos)) {
      qWarning("QGradient::setColorAt: Color position must be specified in the range 0 to 1");
      return;
   }

   int index = 0;
   if (!qIsNaN(pos))
      while (index < m_stops.size() && m_stops.at(index).first < pos) {
         ++index;
      }

   if (index < m_stops.size() && m_stops.at(index).first == pos) {
      m_stops[index].second = color;
   } else {
      m_stops.insert(index, QGradientStop(pos, color));
   }
}

void QGradient::setStops(const QGradientStops &stops)
{
   m_stops.clear();
   for (int i = 0; i < stops.size(); ++i) {
      setColorAt(stops.at(i).first, stops.at(i).second);
   }
}


/*!
    Returns the stop points for this gradient.

    If no stop points have been specified, a gradient of black at 0 to white
    at 1 is used.

    \sa setStops(), setColorAt()
*/
QGradientStops QGradient::stops() const
{
   if (m_stops.isEmpty()) {
      QGradientStops tmp;
      tmp << QGradientStop(0, Qt::black) << QGradientStop(1, Qt::white);
      return tmp;
   }
   return m_stops;
}

#define Q_DUMMY_ACCESSOR union {void *p; uint i;}; p = dummy;


QGradient::CoordinateMode QGradient::coordinateMode() const
{
   Q_DUMMY_ACCESSOR
   return CoordinateMode(i & 0x03);
}

/*!
    \since 4.4

    Sets the coordinate mode of this gradient to \a mode. The default
    mode is LogicalMode.
*/
void QGradient::setCoordinateMode(CoordinateMode mode)
{
   Q_DUMMY_ACCESSOR
   i &= ~0x03;
   i |= uint(mode);
   dummy = p;
}


QGradient::InterpolationMode QGradient::interpolationMode() const
{
   Q_DUMMY_ACCESSOR
   return InterpolationMode((i >> 2) & 0x01);
}


void QGradient::setInterpolationMode(InterpolationMode mode)
{
   Q_DUMMY_ACCESSOR
   i &= ~(1 << 2);
   i |= (uint(mode) << 2);
   dummy = p;
}

/*!
    \fn bool QGradient::operator!=(const QGradient &gradient) const
    \since 4.2

    Returns true if the gradient is the same as the other \a gradient
    specified; otherwise returns false.

    \sa operator==()
*/

/*!
    Returns true if the gradient is the same as the other \a gradient
    specified; otherwise returns false.

    \sa operator!=()
*/
bool QGradient::operator==(const QGradient &gradient) const
{
   if (gradient.m_type != m_type
      || gradient.m_spread != m_spread
      || gradient.dummy != dummy) {
      return false;
   }

   if (m_type == LinearGradient) {
      if (m_data.linear.x1 != gradient.m_data.linear.x1
         || m_data.linear.y1 != gradient.m_data.linear.y1
         || m_data.linear.x2 != gradient.m_data.linear.x2
         || m_data.linear.y2 != gradient.m_data.linear.y2) {
         return false;
      }
   } else if (m_type == RadialGradient) {
      if (m_data.radial.cx != gradient.m_data.radial.cx
         || m_data.radial.cy != gradient.m_data.radial.cy
         || m_data.radial.fx != gradient.m_data.radial.fx
         || m_data.radial.fy != gradient.m_data.radial.fy
         || m_data.radial.cradius != gradient.m_data.radial.cradius) {
         return false;
      }
   } else { // m_type == ConicalGradient
      if (m_data.conical.cx != gradient.m_data.conical.cx
         || m_data.conical.cy != gradient.m_data.conical.cy
         || m_data.conical.angle != gradient.m_data.conical.angle) {
         return false;
      }
   }

   return stops() == gradient.stops();
}


QLinearGradient::QLinearGradient()
{
   m_type = LinearGradient;
   m_spread = PadSpread;
   m_data.linear.x1 = 0;
   m_data.linear.y1 = 0;
   m_data.linear.x2 = 1;
   m_data.linear.y2 = 1;
}
QLinearGradient::QLinearGradient(const QPointF &start, const QPointF &finalStop)
{
   m_type = LinearGradient;
   m_spread = PadSpread;
   m_data.linear.x1 = start.x();
   m_data.linear.y1 = start.y();
   m_data.linear.x2 = finalStop.x();
   m_data.linear.y2 = finalStop.y();
}

QLinearGradient::QLinearGradient(qreal xStart, qreal yStart, qreal xFinalStop, qreal yFinalStop)
{
   m_type = LinearGradient;
   m_spread = PadSpread;
   m_data.linear.x1 = xStart;
   m_data.linear.y1 = yStart;
   m_data.linear.x2 = xFinalStop;
   m_data.linear.y2 = yFinalStop;
}


QPointF QLinearGradient::start() const
{
   Q_ASSERT(m_type == LinearGradient);
   return QPointF(m_data.linear.x1, m_data.linear.y1);
}



void QLinearGradient::setStart(const QPointF &start)
{
   Q_ASSERT(m_type == LinearGradient);
   m_data.linear.x1 = start.x();
   m_data.linear.y1 = start.y();
}



QPointF QLinearGradient::finalStop() const
{
   Q_ASSERT(m_type == LinearGradient);
   return QPointF(m_data.linear.x2, m_data.linear.y2);
}


/*!
    \since 4.2

    Sets the final stop point of this linear gradient in logical
    coordinates to \a stop.

    \sa finalStop()
*/

void QLinearGradient::setFinalStop(const QPointF &stop)
{
   Q_ASSERT(m_type == LinearGradient);
   m_data.linear.x2 = stop.x();
   m_data.linear.y2 = stop.y();
}




static QPointF qt_radial_gradient_adapt_focal_point(const QPointF &center,
   qreal radius,
   const QPointF &focalPoint)
{
   // We have a one pixel buffer zone to avoid numerical instability on the
   // circle border
   //### this is hacky because technically we should adjust based on current matrix
   const qreal compensated_radius = radius - radius * qreal(0.001);
   QLineF line(center, focalPoint);
   if (line.length() > (compensated_radius)) {
      line.setLength(compensated_radius);
   }
   return line.p2();
}

/*!
    Constructs a simple radial gradient with the given \a center, \a
    radius and \a focalPoint.

    \note If the given focal point is outside the circle defined by the
    \a center point and \a radius, it will be re-adjusted to lie at a point on
    the circle where it intersects with the line from \a center to
    \a focalPoint.

    \sa QGradient::setColorAt(), QGradient::setStops()
*/

QRadialGradient::QRadialGradient(const QPointF &center, qreal radius, const QPointF &focalPoint)
{
   m_type = RadialGradient;
   m_spread = PadSpread;
   m_data.radial.cx = center.x();
   m_data.radial.cy = center.y();
   m_data.radial.cradius = radius;

   QPointF adapted_focal = qt_radial_gradient_adapt_focal_point(center, radius, focalPoint);
   m_data.radial.fx = adapted_focal.x();
   m_data.radial.fy = adapted_focal.y();
}

/*!
    Constructs a simple radial gradient with the given \a center, \a
    radius and the focal point in the circle center.

    \sa QGradient::setColorAt(), QGradient::setStops()
*/
QRadialGradient::QRadialGradient(const QPointF &center, qreal radius)
{
   m_type = RadialGradient;
   m_spread = PadSpread;
   m_data.radial.cx = center.x();
   m_data.radial.cy = center.y();
   m_data.radial.cradius = radius;
   m_data.radial.fx = center.x();
   m_data.radial.fy = center.y();
}


/*!
    Constructs a simple radial gradient with the given center (\a cx, \a cy),
    \a radius and focal point (\a fx, \a fy).

    \note If the given focal point is outside the circle defined by the
    center (\a cx, \a cy) and the \a radius it will be re-adjusted to
    the intersection between the line from the center to the focal point
    and the circle.

    \sa QGradient::setColorAt(), QGradient::setStops()
*/

QRadialGradient::QRadialGradient(qreal cx, qreal cy, qreal radius, qreal fx, qreal fy)
{
   m_type = RadialGradient;
   m_spread = PadSpread;
   m_data.radial.cx = cx;
   m_data.radial.cy = cy;
   m_data.radial.cradius = radius;

   QPointF adapted_focal = qt_radial_gradient_adapt_focal_point(QPointF(cx, cy),
         radius,
         QPointF(fx, fy));

   m_data.radial.fx = adapted_focal.x();
   m_data.radial.fy = adapted_focal.y();
}

/*!
    Constructs a simple radial gradient with the center at (\a cx, \a cy) and the
    specified \a radius. The focal point lies at the center of the circle.

    \sa QGradient::setColorAt(), QGradient::setStops()
 */
QRadialGradient::QRadialGradient(qreal cx, qreal cy, qreal radius)
{
   m_type = RadialGradient;
   m_spread = PadSpread;
   m_data.radial.cx = cx;
   m_data.radial.cy = cy;
   m_data.radial.cradius = radius;
   m_data.radial.fx = cx;
   m_data.radial.fy = cy;
}


/*!
    Constructs a simple radial gradient with the center and focal point at
    (0, 0) with a radius of 1.
*/
QRadialGradient::QRadialGradient()
{
   m_type = RadialGradient;
   m_spread = PadSpread;
   m_data.radial.cx = 0;
   m_data.radial.cy = 0;
   m_data.radial.cradius = 1;
   m_data.radial.fx = 0;
   m_data.radial.fy = 0;
}

/*!
    \since 4.8

    Constructs an extended radial gradient with the given \a center, \a
    centerRadius, \a focalPoint, and \a focalRadius.
*/
QRadialGradient::QRadialGradient(const QPointF &center, qreal centerRadius, const QPointF &focalPoint,
   qreal focalRadius)
{
   m_type = RadialGradient;
   m_spread = PadSpread;
   m_data.radial.cx = center.x();
   m_data.radial.cy = center.y();
   m_data.radial.cradius = centerRadius;

   m_data.radial.fx = focalPoint.x();
   m_data.radial.fy = focalPoint.y();
   setFocalRadius(focalRadius);
}

/*!
    \since 4.8

    Constructs an extended radial gradient with the given center
    (\a cx, \a cy), center radius, \a centerRadius, focal point, (\a fx, \a fy),
    and focal radius \a focalRadius.
*/
QRadialGradient::QRadialGradient(qreal cx, qreal cy, qreal centerRadius, qreal fx, qreal fy, qreal focalRadius)
{
   m_type = RadialGradient;
   m_spread = PadSpread;
   m_data.radial.cx = cx;
   m_data.radial.cy = cy;
   m_data.radial.cradius = centerRadius;

   m_data.radial.fx = fx;
   m_data.radial.fy = fy;
   setFocalRadius(focalRadius);
}

/*!
    Returns the center of this radial gradient in logical coordinates.

    \sa QGradient::stops()
*/

QPointF QRadialGradient::center() const
{
   Q_ASSERT(m_type == RadialGradient);
   return QPointF(m_data.radial.cx, m_data.radial.cy);
}

void QRadialGradient::setCenter(const QPointF &center)
{
   Q_ASSERT(m_type == RadialGradient);
   m_data.radial.cx = center.x();
   m_data.radial.cy = center.y();
}


/*!
    Returns the radius of this radial gradient in logical coordinates.

    Equivalent to centerRadius()

    \sa QGradient::stops()
*/

qreal QRadialGradient::radius() const
{
   Q_ASSERT(m_type == RadialGradient);
   return m_data.radial.cradius;
}


/*!
    \since 4.2

    Sets the radius of this radial gradient in logical coordinates
    to \a radius

    Equivalent to setCenterRadius()
*/
void QRadialGradient::setRadius(qreal radius)
{
   Q_ASSERT(m_type == RadialGradient);
   m_data.radial.cradius = radius;
}

/*!
    \since 4.8

    Returns the center radius of this radial gradient in logical
    coordinates.

    \sa QGradient::stops()
*/
qreal QRadialGradient::centerRadius() const
{
   Q_ASSERT(m_type == RadialGradient);
   return m_data.radial.cradius;
}

/*!
   \since 4.8

   Sets the center radius of this radial gradient in logical coordinates
   to \a radius
*/
void QRadialGradient::setCenterRadius(qreal radius)
{
   Q_ASSERT(m_type == RadialGradient);
   m_data.radial.cradius = radius;
}

/*!
    \since 4.8

    Returns the focal radius of this radial gradient in logical
    coordinates.

    \sa QGradient::stops()
*/
qreal QRadialGradient::focalRadius() const
{
   Q_ASSERT(m_type == RadialGradient);
   Q_DUMMY_ACCESSOR

   // mask away low three bits
   union {
      float f;
      quint32 i;
   } u;
   u.i = i & ~0x07;
   return u.f;
}

/*!
   \since 4.8

   Sets the focal radius of this radial gradient in logical coordinates
   to \a radius
*/
void QRadialGradient::setFocalRadius(qreal radius)
{
   Q_ASSERT(m_type == RadialGradient);
   Q_DUMMY_ACCESSOR

   // Since there's no QGradientData, we only have the dummy void * to
   // store additional data in. The three lowest bits are already
   // taken, thus we cut the three lowest bits from the significand
   // and store the radius as a float.
   union {
      float f;
      quint32 i;
   } u;
   u.f = float(radius);
   // add 0x04 to round up when we drop the three lowest bits
   i |= (u.i + 0x04) & ~0x07;
   dummy = p;
}

/*!
    Returns the focal point of this radial gradient in logical
    coordinates.

    \sa QGradient::stops()
*/

QPointF QRadialGradient::focalPoint() const
{
   Q_ASSERT(m_type == RadialGradient);
   return QPointF(m_data.radial.fx, m_data.radial.fy);
}

void QRadialGradient::setFocalPoint(const QPointF &focalPoint)
{
   Q_ASSERT(m_type == RadialGradient);
   m_data.radial.fx = focalPoint.x();
   m_data.radial.fy = focalPoint.y();
}

QConicalGradient::QConicalGradient(const QPointF &center, qreal angle)
{
   m_type = ConicalGradient;
   m_spread = PadSpread;
   m_data.conical.cx = center.x();
   m_data.conical.cy = center.y();
   m_data.conical.angle = angle;
}

QConicalGradient::QConicalGradient(qreal cx, qreal cy, qreal angle)
{
   m_type = ConicalGradient;
   m_spread = PadSpread;
   m_data.conical.cx = cx;
   m_data.conical.cy = cy;
   m_data.conical.angle = angle;
}

QConicalGradient::QConicalGradient()
{
   m_type = ConicalGradient;
   m_spread = PadSpread;
   m_data.conical.cx = 0;
   m_data.conical.cy = 0;
   m_data.conical.angle = 0;
}

QPointF QConicalGradient::center() const
{
   Q_ASSERT(m_type == ConicalGradient);
   return QPointF(m_data.conical.cx, m_data.conical.cy);
}

void QConicalGradient::setCenter(const QPointF &center)
{
   Q_ASSERT(m_type == ConicalGradient);
   m_data.conical.cx = center.x();
   m_data.conical.cy = center.y();
}

/*!
    Returns the start angle of the conical gradient in logical
    coordinates.

    \sa stops()
*/

qreal QConicalGradient::angle() const
{
   Q_ASSERT(m_type == ConicalGradient);
   return m_data.conical.angle;
}

void QConicalGradient::setAngle(qreal angle)
{
   Q_ASSERT(m_type == ConicalGradient);
   m_data.conical.angle = angle;
}

#undef Q_DUMMY_ACCESSOR

