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

#include <qpicture.h>
#include <qpicture_p.h>

#ifndef QT_NO_PICTURE

#include <qalgorithms.h>
#include <qguiapplication.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qfile.h>
#include <qimage.h>
#include <qmutex.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpixmap.h>
#include <qregion.h>

#ifndef QT_NO_PICTUREIO
#include <qregularexpression.h>
#include <qpictureformatplugin.h>
#endif

#include <qfactoryloader_p.h>
#include <qpaintengine_pic_p.h>
#include <qfont_p.h>

#include <algorithm>

void qt_format_text(const QFont &fnt, const QRectF &_r, int tf, const QTextOption *opt,
      const QString &str, QRectF *brect, int tabstops, int *, int tabarraylen, QPainter *painter);

const char *qt_mfhdr_tag = "QPIC";               // header tag

static constexpr const quint16 mfhdr_maj = 11;   // major version #
static constexpr const quint16 mfhdr_min = 0;    // minor version #

QPicture::QPicture(int formatVersion)
   : QPaintDevice(), d_ptr(new QPicturePrivate)
{
   Q_D(QPicture);

   if (formatVersion == 0) {
      qWarning("QPicture::QPicture() Invalid format version");
   }

   if (formatVersion > 0 && formatVersion != (int)mfhdr_maj) {
      d->formatMajor = formatVersion;
      d->formatMinor = 0;
      d->formatOk = false;
   } else {
      d->resetFormat();
   }
}

QPicture::QPicture(const QPicture &pic)
   : QPaintDevice(), d_ptr(pic.d_ptr)
{
}

QPicture::QPicture(QPicturePrivate &dptr)
   : QPaintDevice(), d_ptr(&dptr)
{
}

QPicture::~QPicture()
{
}

int QPicture::devType() const
{
   return QInternal::Picture;
}

bool QPicture::isNull() const
{
   return d_func()->pictb.buffer().isNull();
}

uint QPicture::size() const
{
   return d_func()->pictb.buffer().size();
}

const char *QPicture::data() const
{
   return d_func()->pictb.buffer().constData();
}

void QPicture::detach()
{
   d_ptr.detach();
}

bool QPicture::isDetached() const
{
   return d_func()->ref.load() == 1;
}

void QPicture::setData(const char *data, uint size)
{
   detach();
   d_func()->pictb.setData(data, size);
   d_func()->resetFormat();
}

bool QPicture::load(const QString &fileName, const QString &format)
{
   QFile f(fileName);

   if (! f.open(QIODevice::ReadOnly)) {
      operator=(QPicture());

      return false;
   }

   return load(&f, format);
}

bool QPicture::load(QIODevice *dev, const QString &format)
{
   if (! format.isEmpty()) {

#ifndef QT_NO_PICTUREIO
      QPictureIO io(dev, format);
      if (io.read()) {
         operator=(io.picture());
         return true;
      }
#endif

      qWarning("QPicture::load() No picture format with the name %s", csPrintable(format));
      operator=(QPicture());
      return false;
   }

   detach();
   QByteArray a = dev->readAll();

   d_func()->pictb.setData(a);                        // set byte array in buffer
   return d_func()->checkFormat();
}

bool QPicture::save(const QString &fileName, const QString &format)
{
   if (paintingActive()) {
      qWarning("QPicture::save() Painting in progress, QPainter::end() required before saving");
      return false;
   }

   if (! format.isEmpty()) {

#ifndef QT_NO_PICTUREIO
      QPictureIO io(fileName, format);
      bool result = io.write();

      if (result) {
         operator=(io.picture());

      } else if (! format.isEmpty())

#else
      bool result = false;
#endif

      {
         qWarning("QPicture::save() Picture format was not found, %s", csPrintable(format));
      }

      return result;
   }

   QFile f(fileName);

   if (! f.open(QIODevice::WriteOnly)) {
      return false;
   }

   return save(&f, format);
}

bool QPicture::save(QIODevice *dev, const QString &format)
{
   if (paintingActive()) {
      qWarning("QPicture::save() Painting in progress, QPainter::end() required before saving");
      return false;
   }

   if (! format.isEmpty()) {

#ifndef QT_NO_PICTUREIO
      QPictureIO io(dev, format);
      bool result = io.write();

      if (result) {
         operator=(io.picture());

      } else if (! format.isEmpty())

#else
      bool result = false;
#endif

      {
         qWarning("QPicture::save() Picture format was not found, %s", csPrintable(format));
      }
      return result;
   }

   dev->write(d_func()->pictb.buffer().constData(), d_func()->pictb.buffer().size());
   return true;
}

QRect QPicture::boundingRect() const
{
   Q_D(const QPicture);

   // Use override rect where possible.
   if (!d->override_rect.isEmpty()) {
      return d->override_rect;
   }

   if (! d->formatOk) {
      d_ptr->checkFormat();
   }

   return d->brect;
}

void QPicture::setBoundingRect(const QRect &r)
{
   d_func()->override_rect = r;
}

bool QPicture::play(QPainter *painter)
{
   Q_D(QPicture);

   if (d->pictb.size() == 0) {                       // nothing recorded
      return true;
   }

   if (! d->formatOk && !d->checkFormat()) {
      return false;
   }

   d->pictb.open(QIODevice::ReadOnly);               // open buffer device
   QDataStream s;
   s.setDevice(&d->pictb);                           // attach data stream to buffer
   s.device()->seek(10);                             // go directly to the data
   s.setVersion(d->formatMajor == 4 ? 3 : d->formatMajor);

   quint8  c, clen;
   quint32 nrecords;
   s >> c >> clen;
   Q_ASSERT(c == QPicturePrivate::PdcBegin);

   // bounding rect was introduced in ver 4. Read in checkFormat().
   if (d->formatMajor >= 4) {
      qint32 dummy;
      s >> dummy >> dummy >> dummy >> dummy;
   }

   s >> nrecords;
   if (!exec(painter, s, nrecords)) {
      qWarning("QPicture::play() Format error");
      d->pictb.close();
      return false;
   }

   d->pictb.close();
   return true;                                // no end-command
}

// QFakeDevice is used to create fonts with a custom DPI
class QFakeDevice : public QPaintDevice
{
 public:
   QFakeDevice() {
      dpi_x = qt_defaultDpiX();
      dpi_y = qt_defaultDpiY();
   }

   void setDpiX(int dpi) {
      dpi_x = dpi;
   }

   void setDpiY(int dpi) {
      dpi_y = dpi;
   }

   QPaintEngine *paintEngine() const override {
      return nullptr;
   }

   int metric(PaintDeviceMetric m) const override {
      switch (m) {
         case PdmPhysicalDpiX:
         case PdmDpiX:
            return dpi_x;
         case PdmPhysicalDpiY:
         case PdmDpiY:
            return dpi_y;
         default:
            return QPaintDevice::metric(m);
      }
   }

 private:
   int dpi_x;
   int dpi_y;
};

bool QPicture::exec(QPainter *painter, QDataStream &s, int nrecords)
{
   Q_D(QPicture);

#if defined(CS_SHOW_DEBUG_GUI_IMAGE)
   int        strm_pos;
#endif

   quint8     c;                      // command id
   quint8     tiny_len;               // 8-bit length descriptor
   qint32     len;                    // 32-bit length descriptor
   qint16     i_16, i1_16, i2_16;     // parameters...
   qint8      i_8;
   quint32    ul;
   double     dbl;
   bool       bl;

   QByteArray  str1;
   QString     str;
   QPointF     p, p1, p2;
   QPoint      ip, ip1, ip2;
   QRect       ir;
   QRectF      r;
   QPolygonF   a;
   QPolygon    ia;
   QColor      color;
   QFont       font;
   QPen        pen;
   QBrush      brush;
   QRegion     rgn;
   QMatrix     wmatrix;
   QTransform  matrix;

   QTransform worldMatrix = painter->transform();
   worldMatrix.scale(qreal(painter->device()->logicalDpiX()) / qreal(qt_defaultDpiX()),
      qreal(painter->device()->logicalDpiY()) / qreal(qt_defaultDpiY()));
   painter->setTransform(worldMatrix);

   while (nrecords-- && !s.atEnd()) {
      s >> c;                 // read cmd
      s >> tiny_len;          // read param length

      if (tiny_len == 255) {  // longer than 254 bytes
         s >> len;
      } else {
         len = tiny_len;
      }

#if defined(CS_SHOW_DEBUG_GUI_IMAGE)
      strm_pos = s.device()->pos();
#endif

      switch (c) {            // exec cmd
         case QPicturePrivate::PdcNOP:
            break;

         case QPicturePrivate::PdcDrawPoint:
            if (d->formatMajor <= 5) {
               s >> ip;
               painter->drawPoint(ip);
            } else {
               s >> p;
               painter->drawPoint(p);
            }
            break;

         case QPicturePrivate::PdcDrawPoints:
            // ## implement in the picture paint engine
            // s >> a >> i1_32 >> i2_32;
            // painter->drawPoints(a.mid(i1_32, i2_32));
            break;

         case QPicturePrivate::PdcDrawPath: {
            QPainterPath path;
            s >> path;
            painter->drawPath(path);
            break;
         }

         case QPicturePrivate::PdcDrawLine:
            if (d->formatMajor <= 5) {
               s >> ip1 >> ip2;
               painter->drawLine(ip1, ip2);
            } else {
               s >> p1 >> p2;
               painter->drawLine(p1, p2);
            }
            break;

         case QPicturePrivate::PdcDrawRect:
            if (d->formatMajor <= 5) {
               s >> ir;
               painter->drawRect(ir);
            } else {
               s >> r;
               painter->drawRect(r);
            }
            break;

         case QPicturePrivate::PdcDrawRoundRect:
            if (d->formatMajor <= 5) {
               s >> ir >> i1_16 >> i2_16;
               painter->drawRoundedRect(ir, i1_16, i2_16, Qt::RelativeSize);
            } else {
               s >> r >> i1_16 >> i2_16;
               painter->drawRoundedRect(r, i1_16, i2_16, Qt::RelativeSize);
            }
            break;

         case QPicturePrivate::PdcDrawEllipse:
            if (d->formatMajor <= 5) {
               s >> ir;
               painter->drawEllipse(ir);
            } else {
               s >> r;
               painter->drawEllipse(r);
            }
            break;

         case QPicturePrivate::PdcDrawArc:
            if (d->formatMajor <= 5) {
               s >> ir;
               r = ir;
            } else {
               s >> r;
            }
            s >> i1_16 >> i2_16;
            painter->drawArc(r, i1_16, i2_16);
            break;

         case QPicturePrivate::PdcDrawPie:
            if (d->formatMajor <= 5) {
               s >> ir;
               r = ir;
            } else {
               s >> r;
            }
            s >> i1_16 >> i2_16;
            painter->drawPie(r, i1_16, i2_16);
            break;

         case QPicturePrivate::PdcDrawChord:
            if (d->formatMajor <= 5) {
               s >> ir;
               r = ir;
            } else {
               s >> r;
            }
            s >> i1_16 >> i2_16;
            painter->drawChord(r, i1_16, i2_16);
            break;

         case QPicturePrivate::PdcDrawLineSegments:
            s >> ia;
            painter->drawLines(ia);
            ia.clear();
            break;

         case QPicturePrivate::PdcDrawPolyline:
            if (d->formatMajor <= 5) {
               s >> ia;
               painter->drawPolyline(ia);
               ia.clear();
            } else {
               s >> a;
               painter->drawPolyline(a);
               a.clear();
            }
            break;

         case QPicturePrivate::PdcDrawPolygon:
            if (d->formatMajor <= 5) {
               s >> ia >> i_8;
               painter->drawPolygon(ia, i_8 ? Qt::WindingFill : Qt::OddEvenFill);
               a.clear();
            } else {
               s >> a >> i_8;
               painter->drawPolygon(a, i_8 ? Qt::WindingFill : Qt::OddEvenFill);
               a.clear();
            }
            break;

         case QPicturePrivate::PdcDrawCubicBezier: {
            s >> ia;
            QPainterPath path;
            Q_ASSERT(ia.size() == 4);
            path.moveTo(ia.at(0));
            path.cubicTo(ia.at(1), ia.at(2), ia.at(3));
            painter->strokePath(path, painter->pen());
            a.clear();
         }
         break;

         case QPicturePrivate::PdcDrawText:
            s >> ip >> str1;
            painter->drawText(ip, QString::fromLatin1(str1));
            break;

         case QPicturePrivate::PdcDrawTextFormatted:
            s >> ir >> i_16 >> str1;
            painter->drawText(ir, i_16, QString::fromLatin1(str1));
            break;

         case QPicturePrivate::PdcDrawText2:
            if (d->formatMajor <= 5) {
               s >> ip >> str;
               painter->drawText(ip, str);
            } else {
               s >> p >> str;
               painter->drawText(p, str);
            }
            break;

         case QPicturePrivate::PdcDrawText2Formatted:
            s >> ir;
            s >> i_16;
            s >> str;
            painter->drawText(ir, i_16, str);
            break;

         case QPicturePrivate::PdcDrawTextItem: {
            s >> p >> str >> font >> ul;

            // the text layout direction is not used here because it's already
            // aligned when QPicturePaintEngine::drawTextItem() serializes the
            // drawText() call, therefore ul is unsed in this context

            if (d->formatMajor >= 9) {
               s >> dbl;
               QFont fnt(font);
               if (dbl != 1.0) {
                  QFakeDevice fake;
                  fake.setDpiX(qRound(dbl * qt_defaultDpiX()));
                  fake.setDpiY(qRound(dbl * qt_defaultDpiY()));
                  fnt = QFont(font, &fake);
               }

               qreal justificationWidth;
               s >> justificationWidth;

               int flags = Qt::TextSingleLine | Qt::TextDontClip | Qt::TextForceLeftToRight;

               QSizeF size(1, 1);
               if (justificationWidth > 0) {
                  size.setWidth(justificationWidth);
                  flags |= Qt::TextJustificationForced;
                  flags |= Qt::AlignJustify;
               }

               QFontMetrics fm(fnt);
               QPointF pt(p.x(), p.y() - fm.ascent());
               qt_format_text(fnt, QRectF(pt, size), flags, nullptr,
                     str, nullptr, 0, nullptr, 0, painter);

            } else {
               qt_format_text(font, QRectF(p, QSizeF(1, 1)), Qt::TextSingleLine | Qt::TextDontClip, nullptr,
                     str, nullptr, 0, nullptr, 0, painter);
            }

            break;
         }

         case QPicturePrivate::PdcDrawPixmap: {
            QPixmap pixmap;
            if (d->formatMajor < 4) {
               s >> ip >> pixmap;
               painter->drawPixmap(ip, pixmap);

            } else if (d->formatMajor <= 5) {
               s >> ir >> pixmap;
               painter->drawPixmap(ir, pixmap);

            } else {
               QRectF sr;
               if (d->in_memory_only) {
                  int index;
                  s >> r >> index >> sr;
                  Q_ASSERT(index < d->pixmap_list.size());
                  pixmap = d->pixmap_list.at(index);
               } else {
                  s >> r >> pixmap >> sr;
               }
               painter->drawPixmap(r, pixmap, sr);
            }
         }
         break;

         case QPicturePrivate::PdcDrawTiledPixmap: {
            QPixmap pixmap;
            if (d->in_memory_only) {
               int index;
               s >> r >> index >> p;
               Q_ASSERT(index < d->pixmap_list.size());
               pixmap = d->pixmap_list.at(index);
            } else {
               s >> r >> pixmap >> p;
            }
            painter->drawTiledPixmap(r, pixmap, p);
         }
         break;

         case QPicturePrivate::PdcDrawImage: {
            QImage image;

            if (d->formatMajor < 4) {
               s >> p >> image;
               painter->drawImage(p, image);
            } else if (d->formatMajor <= 5) {
               s >> ir >> image;
               painter->drawImage(ir, image, QRect(0, 0, ir.width(), ir.height()));
            } else {
               QRectF sr;
               if (d->in_memory_only) {
                  int index;
                  s >> r >> index >> sr >> ul;
                  Q_ASSERT(index < d->image_list.size());
                  image = d->image_list.at(index);
               } else {
                  s >> r >> image >> sr >> ul;
               }
               painter->drawImage(r, image, sr, Qt::ImageConversionFlags(ul));
            }
         }
         break;

         case QPicturePrivate::PdcBegin:
            s >> ul;                        // number of records
            if (!exec(painter, s, ul)) {
               return false;
            }
            break;

         case QPicturePrivate::PdcEnd:
            if (nrecords == 0) {
               return true;
            }
            break;

         case QPicturePrivate::PdcSave:
            painter->save();
            break;

         case QPicturePrivate::PdcRestore:
            painter->restore();
            break;

         case QPicturePrivate::PdcSetBkColor:
            s >> color;
            painter->setBackground(color);
            break;

         case QPicturePrivate::PdcSetBkMode:
            s >> i_8;
            painter->setBackgroundMode((Qt::BGMode)i_8);
            break;

         case QPicturePrivate::PdcSetROP: // NOP
            s >> i_8;
            break;

         case QPicturePrivate::PdcSetBrushOrigin:
            if (d->formatMajor <= 5) {
               s >> ip;
               painter->setBrushOrigin(ip);
            } else {
               s >> p;
               painter->setBrushOrigin(p);
            }
            break;

         case QPicturePrivate::PdcSetFont:
            s >> font;
            painter->setFont(font);
            break;

         case QPicturePrivate::PdcSetPen:
            if (d->in_memory_only) {
               int index;
               s >> index;
               Q_ASSERT(index < d->pen_list.size());
               pen = d->pen_list.at(index);
            } else {
               s >> pen;
            }
            painter->setPen(pen);
            break;

         case QPicturePrivate::PdcSetBrush:
            if (d->in_memory_only) {
               int index;
               s >> index;
               Q_ASSERT(index < d->brush_list.size());
               brush = d->brush_list.at(index);
            } else {
               s >> brush;
            }
            painter->setBrush(brush);
            break;

         case QPicturePrivate::PdcSetVXform:
            s >> i_8;
            painter->setViewTransformEnabled(i_8);
            break;

         case QPicturePrivate::PdcSetWindow:
            if (d->formatMajor <= 5) {
               s >> ir;
               painter->setWindow(ir);
            } else {
               s >> r;
               painter->setWindow(r.toRect());
            }
            break;

         case QPicturePrivate::PdcSetViewport:
            if (d->formatMajor <= 5) {
               s >> ir;
               painter->setViewport(ir);
            } else {
               s >> r;
               painter->setViewport(r.toRect());
            }
            break;

        case QPicturePrivate::PdcSetWXform:
            s >> i_8;
            painter->setMatrixEnabled(i_8);
            break;

        case QPicturePrivate::PdcSetWMatrix:
            if (d->formatMajor >= 8) {
               s >> matrix >> i_8;
            } else {
               s >> wmatrix >> i_8;
               matrix = QTransform(wmatrix);
            }
            // i_8 is always false due to updateXForm() in qpaintengine_pic.cpp
            painter->setTransform(matrix * worldMatrix, i_8);
            break;

         case QPicturePrivate::PdcSetClip:
            s >> i_8;
            painter->setClipping(i_8);
            break;

        case QPicturePrivate::PdcSetClipRegion:
            s >> rgn >> i_8;
            if (d->formatMajor >= 9) {
               painter->setClipRegion(rgn, Qt::ClipOperation(i_8));
            } else {
               painter->setClipRegion(rgn);
            }
            break;

         case QPicturePrivate::PdcSetClipPath: {
            QPainterPath path;
            s >> path >> i_8;
            painter->setClipPath(path, Qt::ClipOperation(i_8));
            break;
         }

         case QPicturePrivate::PdcSetRenderHint:
            s >> ul;
            painter->setRenderHint(QPainter::Antialiasing,
               bool(ul & QPainter::Antialiasing));

            painter->setRenderHint(QPainter::SmoothPixmapTransform,
               bool(ul & QPainter::SmoothPixmapTransform));

            break;

         case QPicturePrivate::PdcSetCompositionMode:
            s >> ul;
            painter->setCompositionMode((QPainter::CompositionMode)ul);
            break;

         case QPicturePrivate::PdcSetClipEnabled:
            s >> bl;
            painter->setClipping(bl);
            break;

         case QPicturePrivate::PdcSetOpacity:
            s >> dbl;
            painter->setOpacity(qreal(dbl));
            break;

         default:
            qWarning("QPicture::play() Invalid command %d", c);

            if (len) {
               // skip unknown command
               s.device()->seek(s.device()->pos() + len);
            }

      }

#if defined(CS_SHOW_DEBUG_GUI_IMAGE)
      qDebug("device->at(): %i, strm_pos: %i len: %i", (int)s.device()->pos(), strm_pos, len);
      Q_ASSERT(qint32(s.device()->pos() - strm_pos) == len);
#endif

   }

   return false;
}

int QPicture::metric(PaintDeviceMetric m) const
{
   int val;
   QRect brect = boundingRect();

   switch (m) {
      case PdmWidth:
         val = brect.width();
         break;

      case PdmHeight:
         val = brect.height();
         break;

      case PdmWidthMM:
         val = int(25.4 / qt_defaultDpiX() * brect.width());
         break;

      case PdmHeightMM:
         val = int(25.4 / qt_defaultDpiY() * brect.height());
         break;

      case PdmDpiX:
      case PdmPhysicalDpiX:
         val = qt_defaultDpiX();
         break;

      case PdmDpiY:
      case PdmPhysicalDpiY:
         val = qt_defaultDpiY();
         break;

      case PdmNumColors:
         val = 16777216;
         break;

      case PdmDepth:
         val = 24;
         break;

      case PdmDevicePixelRatio:
         val = 1;
         break;

      case PdmDevicePixelRatioScaled:
         val = 1 * QPaintDevice::devicePixelRatioFScale();
         break;

      default:
         val = 0;
         qWarning("QPicture::metric() Invalid metric command");
   }
   return val;
}

QPicture &QPicture::operator=(const QPicture &p)
{
   d_ptr = p.d_ptr;
   return *this;
}

QPicturePrivate::QPicturePrivate()
   : in_memory_only(false)
{
}

QPicturePrivate::QPicturePrivate(const QPicturePrivate &other)
   : trecs(other.trecs), formatOk(other.formatOk), formatMinor(other.formatMinor), brect(other.brect),
     override_rect(other.override_rect), in_memory_only(false)
{
   pictb.setData(other.pictb.data().constData(), other.pictb.size());

   if (other.pictb.isOpen()) {
      pictb.open(other.pictb.openMode());
      pictb.seek(other.pictb.pos());
   }
}

void QPicturePrivate::resetFormat()
{
   formatOk = false;
   formatMajor = mfhdr_maj;
   formatMinor = mfhdr_min;
}

bool QPicturePrivate::checkFormat()
{
   resetFormat();

   // can not check anything in an empty buffer
   if (pictb.size() == 0 || pictb.isOpen()) {
      return false;
   }

   pictb.open(QIODevice::ReadOnly);              // open buffer device
   QDataStream s;
   s.setDevice(&pictb);                          // attach data stream to buffer

   char mf_id[4];                                // picture header tag
   s.readRawData(mf_id, 4);                      // read actual tag

   if (memcmp(mf_id, qt_mfhdr_tag, 4) != 0) {
      // wrong header id
      qWarning("QPicturePaintEngine::checkFormat() Incorrect header");
      pictb.close();

      return false;
   }

   int cs_start = sizeof(quint32);               // pos of checksum word
   int data_start = cs_start + sizeof(quint16);
   quint16 cs, ccs;
   QByteArray buf = pictb.buffer();              // pointer to data

   s >> cs;                                      // read checksum
   ccs = (quint16) qChecksum(buf.constData() + data_start, buf.size() - data_start);

   if (ccs != cs) {
      qWarning("QPicturePaintEngine::checkFormat() Invalid checksum %x, expected %x", ccs, cs);
      pictb.close();

      return false;
   }

   quint16 major, minor;
   s >> major >> minor;                          // read version number

   if (major > mfhdr_maj) {
      // new, incompatible version
      qWarning("QPicturePaintEngine::checkFormat() Incompatible version %d.%d", major, minor);
      pictb.close();

      return false;
   }
   s.setVersion(major != 4 ? major : 3);

   quint8 c, clen;
   s >> c >> clen;

   if (c == QPicturePrivate::PdcBegin) {
      if (! (major >= 1 && major <= 3)) {
         qint32 l, t, w, h;
         s >> l >> t >> w >> h;
         brect = QRect(l, t, w, h);
      }

   } else {
      qWarning("QPicturePaintEngine::checkFormat() Format error");
      pictb.close();
      return false;
   }
   pictb.close();

   formatOk    = true;
   formatMajor = major;
   formatMinor = minor;

   return true;
}

QPaintEngine *QPicture::paintEngine() const
{
   if (!d_func()->paintEngine) {
      const_cast<QPicture *>(this)->d_func()->paintEngine.reset(new QPicturePaintEngine);
   }
   return d_func()->paintEngine.data();
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &s, const QPicture &r)
{
   quint32 size = r.d_func()->pictb.buffer().size();
   s << size;

   // null picture ?
   if (size == 0) {
      return s;
   }

   // just write the whole buffer to the stream
   s.writeRawData (r.d_func()->pictb.buffer().constData(), r.d_func()->pictb.buffer().size());
   return s;
}

QDataStream &operator>>(QDataStream &s, QPicture &r)
{
   QDataStream sr;

   // "init"; this code is similar to the beginning of QPicture::cmd()
   sr.setDevice(&r.d_func()->pictb);
   sr.setVersion(r.d_func()->formatMajor);

   quint32 len;
   s >> len;

   QByteArray data;
   if (len > 0) {
      data.resize(len);
      s.readRawData(data.data(), len);
   }

   r.d_func()->pictb.setData(data);
   r.d_func()->resetFormat();
   return s;
}
#endif


#ifndef QT_NO_PICTUREIO

QString QPicture::pictureFormat(const QString &fileName)
{
   return QPictureIO::pictureFormat(fileName);
}

QStringList QPicture::inputFormats()
{
   return QPictureIO::inputFormats();
}

QStringList QPicture::inputFormatList()
{
   return QPictureIO::inputFormats();
}

QStringList QPicture::outputFormatList()
{
   return QPictureIO::outputFormats();
}

QStringList QPicture::outputFormats()
{
   return QPictureIO::outputFormats();
}

struct QPictureIOData {
   QPicture        pi;                                // picture
   int             iostat;                            // IO status
   QString         frmt;                              // picture format
   QIODevice      *iodev;                             // IO device
   QString         fname;                             // file name
   QString         descr;                             // picture description
   const char     *parameters;
   int             quality;
   float           gamma;
};

QPictureIO::QPictureIO()
{
   init();
}

QPictureIO::QPictureIO(QIODevice *ioDevice, const QString &format)
{
   init();
   d->iodev = ioDevice;
   d->frmt  = format;
}

QPictureIO::QPictureIO(const QString &fileName, const QString &format)
{
   init();
   d->fname = fileName;
   d->frmt  = format;
}

void QPictureIO::init()
{
   d = new QPictureIOData();
   d->parameters = nullptr;
   d->quality    = -1; // default quality of the current format
   d->gamma      = 0.0f;
   d->iostat     = 0;
   d->iodev      = nullptr;
}

QPictureIO::~QPictureIO()
{
   if (d->parameters) {
      delete [] d->parameters;
   }

   delete d;
}

class QPictureHandler
{
 public:
   enum TMode {
      Untranslated = 0,
      TranslateIn,
      TranslateInOut
   };

   QPictureHandler(const QString &f, const QString &h, const QString &fl,
      picture_io_handler r, picture_io_handler w);

   QString  format;                           // picture format
   QRegularExpression  header;                           // picture header pattern

   picture_io_handler  read_picture;          // picture read function
   picture_io_handler  write_picture;         // picture write function
   bool                obsolete;              // support not "published"

   TMode text_mode;
};

QPictureHandler::QPictureHandler(const QString &f, const QString &h, const QString &fl,
   picture_io_handler r, picture_io_handler w)
   : format(f), header(h)
{
   text_mode = Untranslated;

   if (fl.contains('t')) {
      text_mode = TranslateIn;

   } else if (fl.contains('T')) {
      text_mode = TranslateInOut;

   }

   obsolete      = fl.contains('O');
   read_picture  = r;
   write_picture = w;
}

using QPHList  = QList<QPictureHandler *>;

static QPHList *pictureHandlers()
{
   static QPHList retval;
   return &retval;
}

void qt_init_picture_plugins()
{
   static QMutex mutex;
   QMutexLocker locker(&mutex);

   static QFactoryLoader loader(QPictureFormatInterface_ID, "/pictureformats");
   auto keySet = loader.keySet();

   for (auto item : keySet) {
      if (QPictureFormatPlugin *format = qobject_cast<QPictureFormatPlugin *>(loader.instance(item))) {
         format->installIOHandler(item);
      }
   }
}

static void cleanup()
{
   // make sure that picture handlers are delete before plugin manager
   if (QPHList *list = pictureHandlers()) {
      qDeleteAll(*list);
      list->clear();
   }
}

void qt_init_picture_handlers()
{
   // initialize picture handlers
   static QAtomicInt done = 0;

   int expected = 0;

   if (done.compareExchange(expected, 1, std::memory_order_relaxed)) {
      qAddPostRoutine(cleanup);
   }
}

static QPictureHandler *get_picture_handler(const QString &format)
{
   // get pointer to handler
   qt_init_picture_handlers();
   qt_init_picture_plugins();

   if (QPHList *list = pictureHandlers()) {
      for (int i = 0; i < list->size(); ++i) {
         if (list->at(i)->format == format) {
            return list->at(i);
         }
      }
   }

   return nullptr;         // no such handler
}

void QPictureIO::defineIOHandler(const QString &format, const QString &header, const char *flags,
   picture_io_handler readPicture, picture_io_handler writePicture)
{
   qt_init_picture_handlers();

   if (QPHList *list = pictureHandlers()) {
      QPictureHandler *p;
      p = new QPictureHandler(format, header, QByteArray(flags), readPicture, writePicture);
      list->prepend(p);
   }
}

const QPicture &QPictureIO::picture() const
{
   return d->pi;
}

int QPictureIO::status() const
{
   return d->iostat;
}

QString QPictureIO::format() const
{
   return d->frmt;
}

QIODevice *QPictureIO::ioDevice() const
{
   return d->iodev;
}

QString QPictureIO::fileName() const
{
   return d->fname;
}

QString QPictureIO::description() const
{
   return d->descr;
}

void QPictureIO::setPicture(const QPicture &picture)
{
   d->pi = picture;
}

void QPictureIO::setStatus(int status)
{
   d->iostat = status;
}

void QPictureIO::setFormat(const QString &format)
{
   d->frmt = format;
}

void QPictureIO::setIODevice(QIODevice *ioDevice)
{
   d->iodev = ioDevice;
}

void QPictureIO::setFileName(const QString &fileName)
{
   d->fname = fileName;
}

int QPictureIO::quality() const
{
   return d->quality;
}

void QPictureIO::setQuality(int q)
{
   d->quality = q;
}

const char *QPictureIO::parameters() const
{
   return d->parameters;
}

void QPictureIO::setParameters(const char *parameters)
{
   if (d->parameters) {
      delete [] d->parameters;
   }
   d->parameters = qstrdup(parameters);
}

void QPictureIO::setGamma(float gamma)
{
   d->gamma = gamma;
}

float QPictureIO::gamma() const
{
   return d->gamma;
}

void QPictureIO::setDescription(const QString &description)
{
   d->descr = description;
}

QString QPictureIO::pictureFormat(const QString &fileName)
{
   QFile file(fileName);
   QString format;

   if (! file.open(QIODevice::ReadOnly)) {
      return format;
   }

   format = pictureFormat(&file);
   file.close();

   return format;
}

QString QPictureIO::pictureFormat(QIODevice *d)
{
   // if you change this change the documentation for defineIOHandler()
   const int buflen = 14;

   char buf[buflen];
   char buf2[buflen];
   qt_init_picture_handlers();
   qt_init_picture_plugins();
   int pos = d->pos();                      // save position
   int rdlen = d->read(buf, buflen);        // read a few bytes

   QString format;

   if (rdlen != buflen) {
      return format;
   }

   memcpy(buf2, buf, buflen);

   for (int n = 0; n < rdlen; n++) {
      if (buf[n] == '\0') {
         buf[n] = '\001';
      }
   }

   if (rdlen > 0) {
      buf[rdlen - 1] = '\0';

      QString bufStr = QString::fromLatin1(buf);

      if (QPHList *list = pictureHandlers()) {
         for (int i = 0; i < list->size(); ++i) {

            if (list->at(i)->header.match(bufStr).hasMatch()) {
               // try match with headers
               format = list->at(i)->format;
               break;
            }
         }

      }
   }

   d->seek(pos);                                // restore position
   return format;
}

QStringList QPictureIO::inputFormats()
{
   QStringList result;

   qt_init_picture_handlers();
   qt_init_picture_plugins();

   if (QPHList *list = pictureHandlers()) {
      for (int i = 0; i < list->size(); ++i) {
         QPictureHandler *p = list->at(i);

         if (p->read_picture && !p->obsolete  && !result.contains(p->format)) {
            result.append(p->format);
         }
      }
   }

   std::sort(result.begin(), result.end());

   return result;
}

QStringList QPictureIO::outputFormats()
{
   qt_init_picture_handlers();
   qt_init_picture_plugins();

   QStringList result;

   if (QPHList *list = pictureHandlers()) {
      for (int i = 0; i < list->size(); ++i) {
         QPictureHandler *p = list->at(i);

         if (p->write_picture && !p->obsolete && !result.contains(p->format)) {
            result.append(p->format);
         }
      }
   }

   return result;
}

bool QPictureIO::read()
{
   QFile            file;
   QString          picture_format;
   QPictureHandler  *h;

   if (d->iodev) {                                     // read from io device
      // ok, already open

   } else if (!d->fname.isEmpty()) {                   // read from file
      file.setFileName(d->fname);

      if (! file.open(QIODevice::ReadOnly)) {
         return false;   // cannot open file
      }
      d->iodev = &file;

   } else {                                            // no file name or io device
      return false;
   }

   if (d->frmt.isEmpty()) {
      // try to guess format
      picture_format = pictureFormat(d->iodev);        // get picture format

      if (picture_format.isEmpty()) {
         if (file.isOpen()) {                          // unknown format
            file.close();
            d->iodev = nullptr;
         }

         return false;
      }

   } else {
      picture_format = d->frmt;
   }

   h = get_picture_handler(picture_format);

   if (file.isOpen()) {

#if ! defined(Q_OS_UNIX)
      if (h && h->text_mode) {                         // reopen in translated mode
         file.close();
         file.open(QIODevice::ReadOnly | QIODevice::Text);
      } else
#endif

         file.seek(0);                                 // position to start
   }

   d->iostat = 1;                                      // assume error

   if (h && h->read_picture) {
      (*h->read_picture)(this);
   }

   if (file.isOpen()) {                                // picture was read using file
      file.close();
      d->iodev = nullptr;
   }

   return d->iostat == 0;                              // picture successfully read?
}

bool QPictureIO::write()
{
   if (d->frmt.isEmpty()) {
      return false;
   }

   QPictureHandler *h = get_picture_handler(d->frmt);
   if (! h || ! h->write_picture) {
      qWarning("QPictureIO::write() No picture format handler for %s", csPrintable(format()) );
      return false;
   }

   QFile file;
   if (! d->iodev && ! d->fname.isEmpty()) {
      file.setFileName(d->fname);
      bool translate = h->text_mode == QPictureHandler::TranslateInOut;

      QIODevice::OpenMode fmode = translate ? QIODevice::WriteOnly | QIODevice::Text : QIODevice::OpenMode(
            QIODevice::WriteOnly);

      if (! file.open(fmode)) {
         // could not create file
         return false;
      }

      d->iodev = &file;
   }

   d->iostat = 1;
   (*h->write_picture)(this);

   if (file.isOpen()) {
      // picture was written using file
      file.close();
      d->iodev = nullptr;
   }

   return d->iostat == 0;                   // picture successfully written?
}
#endif // QT_NO_PICTUREIO

#endif // QT_NO_PICTURE

