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

#include <qcolor.h>
#include <qcolor_p.h>
#include <qnamespace.h>
#include <qcolormap.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <qdebug.h>

#ifdef Q_WS_X11
#  include <qapplication.h>
#  include <qx11info_x11.h>
#  include <qt_x11_p.h>

static bool allowX11ColorNames = false;

#endif

#include <math.h>
#include <stdio.h>
#include <limits.h>

#define QCOLOR_INT_RANGE_CHECK(fn, var) \
    do { \
        if (var < 0 || var > 255) { \
            qWarning(#fn": invalid value %d", var); \
            var = qMax(0, qMin(var, 255)); \
        } \
    } while (0)

#define QCOLOR_REAL_RANGE_CHECK(fn, var) \
    do { \
        if (var < qreal(0.0) || var > qreal(1.0)) { \
            qWarning(#fn": invalid value %g", var); \
            var = qMax(qreal(0.0), qMin(var, qreal(1.0)));      \
        } \
    } while (0)

QColor::QColor(Qt::GlobalColor color)
{
#define QRGB(r, g, b) \
    QRgb(((0xffu << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff)))

#define QRGBA(r, g, b, a) \
    QRgb(((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff))

   static const QRgb global_colors[] = {
      QRGB(255, 255, 255), // Qt::color0
      QRGB(  0,   0,   0), // Qt::color1
      QRGB(  0,   0,   0), // black
      QRGB(255, 255, 255), // white
      /*
       * From the "The Palette Manager: How and Why" by Ron Gery,
       * March 23, 1992, archived on MSDN:
       *
       *     The Windows system palette is broken up into two
       *     sections, one with fixed colors and one with colors
       *     that can be changed by applications. The system palette
       *     predefines 20 entries; these colors are known as the
       *     static or reserved colors and consist of the 16 colors
       *     found in the Windows version 3.0 VGA driver and 4
       *     additional colors chosen for their visual appeal.  The
       *     DEFAULT_PALETTE stock object is, as the name implies,
       *     the default palette selected into a device context (DC)
       *     and consists of these static colors. Applications can
       *     set the remaining 236 colors using the Palette Manager.
       *
       * The 20 reserved entries have indices in [0,9] and
       * [246,255]. We reuse 17 of them.
       */
      QRGB(128, 128, 128), // index 248   medium gray
      QRGB(160, 160, 164), // index 247   light gray
      QRGB(192, 192, 192), // index 7     light gray
      QRGB(255,   0,   0), // index 249   red
      QRGB(  0, 255,   0), // index 250   green
      QRGB(  0,   0, 255), // index 252   blue
      QRGB(  0, 255, 255), // index 254   cyan
      QRGB(255,   0, 255), // index 253   magenta
      QRGB(255, 255,   0), // index 251   yellow
      QRGB(128,   0,   0), // index 1     dark red
      QRGB(  0, 128,   0), // index 2     dark green
      QRGB(  0,   0, 128), // index 4     dark blue
      QRGB(  0, 128, 128), // index 6     dark cyan
      QRGB(128,   0, 128), // index 5     dark magenta
      QRGB(128, 128,   0), // index 3     dark yellow
      QRGBA(0, 0, 0, 0)    //             transparent
   };
#undef QRGB
#undef QRGBA

   setRgb(qRed(global_colors[color]),
          qGreen(global_colors[color]),
          qBlue(global_colors[color]),
          qAlpha(global_colors[color]));
}

QColor::QColor(QRgb color)
{
   cspec = Rgb;
   ct.argb.alpha = 0xffff;
   ct.argb.red   = ushort(qRed(color)   * 0x101);
   ct.argb.green = ushort(qGreen(color) * 0x101);
   ct.argb.blue  = ushort(qBlue(color)  * 0x101);
   ct.argb.pad   = 0;
}

// internal
QColor::QColor(Spec spec)
{
   switch (spec) {
      case Invalid:
         invalidate();
         break;
      case Rgb:
         setRgb(0, 0, 0);
         break;
      case Hsv:
         setHsv(0, 0, 0);
         break;
      case Cmyk:
         setCmyk(0, 0, 0, 0);
         break;
      case Hsl:
         setHsl(0, 0, 0, 0);
         break;
   }
}

QString QColor::name() const
{
   QString s;
   s.sprintf("#%02x%02x%02x", red(), green(), blue());
   return s;
}

void QColor::setNamedColor(const QString &name)
{
   setColorFromString(name);
}

bool QColor::isValidColor(const QString &name)
{
   return !name.isEmpty() && QColor().setColorFromString(name);
}

bool QColor::setColorFromString(const QString &name)
{
   if (name.isEmpty()) {
      invalidate();
      return true;
   }

   if (name.startsWith(QLatin1Char('#'))) {
      QRgb rgb;
      if (qt_get_hex_rgb(name.constData(), name.length(), &rgb)) {
         setRgb(rgb);
         return true;
      } else {
         invalidate();
         return false;
      }
   }

#ifndef QT_NO_COLORNAMES
   QRgb rgb;
   if (qt_get_named_rgb(name.constData(), name.length(), &rgb)) {
      setRgba(rgb);
      return true;
   } else
#endif
   {
#ifdef Q_WS_X11
      XColor result;
      if (allowX11ColorNames()
            && QApplication::instance()
            && QX11Info::display()
            && XParseColor(QX11Info::display(), QX11Info::appColormap(), name.toLatin1().constData(), &result)) {
         setRgb(result.red >> 8, result.green >> 8, result.blue >> 8);
         return true;
      } else
#endif
      {
         invalidate();
         return false;
      }
   }
}

QStringList QColor::colorNames()
{
#ifndef QT_NO_COLORNAMES
   return qt_get_colornames();
#else
   return QStringList();
#endif
}

void QColor::getHsvF(qreal *h, qreal *s, qreal *v, qreal *a) const
{
   if (!h || !s || !v) {
      return;
   }

   if (cspec != Invalid && cspec != Hsv) {
      toHsv().getHsvF(h, s, v, a);
      return;
   }

   *h = ct.ahsv.hue == USHRT_MAX ? qreal(-1.0) : ct.ahsv.hue / qreal(36000.0);
   *s = ct.ahsv.saturation / qreal(USHRT_MAX);
   *v = ct.ahsv.value / qreal(USHRT_MAX);

   if (a) {
      *a = ct.ahsv.alpha / qreal(USHRT_MAX);
   }
}

void QColor::getHsv(int *h, int *s, int *v, int *a) const
{
   if (!h || !s || !v) {
      return;
   }

   if (cspec != Invalid && cspec != Hsv) {
      toHsv().getHsv(h, s, v, a);
      return;
   }

   *h = ct.ahsv.hue == USHRT_MAX ? -1 : ct.ahsv.hue / 100;
   *s = ct.ahsv.saturation >> 8;
   *v = ct.ahsv.value      >> 8;

   if (a) {
      *a = ct.ahsv.alpha >> 8;
   }
}

void QColor::setHsvF(qreal h, qreal s, qreal v, qreal a)
{
   if (((h < qreal(0.0) || h > qreal(1.0)) && h != qreal(-1.0))
         || (s < qreal(0.0) || s > qreal(1.0))
         || (v < qreal(0.0) || v > qreal(1.0))
         || (a < qreal(0.0) || a > qreal(1.0))) {
      qWarning("QColor::setHsvF: HSV parameters out of range");
      return;
   }

   cspec = Hsv;
   ct.ahsv.alpha      = qRound(a * USHRT_MAX);
   ct.ahsv.hue        = h == qreal(-1.0) ? USHRT_MAX : qRound(h * 36000);
   ct.ahsv.saturation = qRound(s * USHRT_MAX);
   ct.ahsv.value      = qRound(v * USHRT_MAX);
   ct.ahsv.pad        = 0;
}

void QColor::setHsv(int h, int s, int v, int a)
{
   if (h < -1 || (uint)s > 255 || (uint)v > 255 || (uint)a > 255) {
      qWarning("QColor::setHsv: HSV parameters out of range");
      invalidate();
      return;
   }

   cspec = Hsv;
   ct.ahsv.alpha      = a * 0x101;
   ct.ahsv.hue        = h == -1 ? USHRT_MAX : (h % 360) * 100;
   ct.ahsv.saturation = s * 0x101;
   ct.ahsv.value      = v * 0x101;
   ct.ahsv.pad        = 0;
}

void QColor::getHslF(qreal *h, qreal *s, qreal *l, qreal *a) const
{
   if (!h || !s || !l) {
      return;
   }

   if (cspec != Invalid && cspec != Hsl) {
      toHsl().getHslF(h, s, l, a);
      return;
   }

   *h = ct.ahsl.hue == USHRT_MAX ? qreal(-1.0) : ct.ahsl.hue / qreal(36000.0);
   *s = ct.ahsl.saturation / qreal(USHRT_MAX);
   *l = ct.ahsl.lightness / qreal(USHRT_MAX);

   if (a) {
      *a = ct.ahsl.alpha / qreal(USHRT_MAX);
   }
}

void QColor::getHsl(int *h, int *s, int *l, int *a) const
{
   if (!h || !s || !l) {
      return;
   }

   if (cspec != Invalid && cspec != Hsl) {
      toHsl().getHsl(h, s, l, a);
      return;
   }

   *h = ct.ahsl.hue == USHRT_MAX ? -1 : ct.ahsl.hue / 100;
   *s = ct.ahsl.saturation >> 8;
   *l = ct.ahsl.lightness  >> 8;

   if (a) {
      *a = ct.ahsl.alpha >> 8;
   }
}

void QColor::setHslF(qreal h, qreal s, qreal l, qreal a)
{
   if (((h < qreal(0.0) || h > qreal(1.0)) && h != qreal(-1.0))
         || (s < qreal(0.0) || s > qreal(1.0))
         || (l < qreal(0.0) || l > qreal(1.0))
         || (a < qreal(0.0) || a > qreal(1.0))) {
      qWarning("QColor::setHsvF: HSV parameters out of range");
      return;
   }

   cspec = Hsl;
   ct.ahsl.alpha      = qRound(a * USHRT_MAX);
   ct.ahsl.hue        = h == qreal(-1.0) ? USHRT_MAX : qRound(h * 36000);
   ct.ahsl.saturation = qRound(s * USHRT_MAX);
   ct.ahsl.lightness  = qRound(l * USHRT_MAX);
   ct.ahsl.pad        = 0;
}

void QColor::setHsl(int h, int s, int l, int a)
{
   if (h < -1 || (uint)s > 255 || (uint)l > 255 || (uint)a > 255) {
      qWarning("QColor::setHsv: HSV parameters out of range");
      invalidate();
      return;
   }

   cspec = Hsl;
   ct.ahsl.alpha      = a * 0x101;
   ct.ahsl.hue        = h == -1 ? USHRT_MAX : (h % 360) * 100;
   ct.ahsl.saturation = s * 0x101;
   ct.ahsl.lightness  = l * 0x101;
   ct.ahsl.pad        = 0;
}

void QColor::getRgbF(qreal *r, qreal *g, qreal *b, qreal *a) const
{
   if (!r || !g || !b) {
      return;
   }

   if (cspec != Invalid && cspec != Rgb) {
      toRgb().getRgbF(r, g, b, a);
      return;
   }

   *r = ct.argb.red   / qreal(USHRT_MAX);
   *g = ct.argb.green / qreal(USHRT_MAX);
   *b = ct.argb.blue  / qreal(USHRT_MAX);

   if (a) {
      *a = ct.argb.alpha / qreal(USHRT_MAX);
   }

}

void QColor::getRgb(int *r, int *g, int *b, int *a) const
{
   if (!r || !g || !b) {
      return;
   }

   if (cspec != Invalid && cspec != Rgb) {
      toRgb().getRgb(r, g, b, a);
      return;
   }

   *r = ct.argb.red   >> 8;
   *g = ct.argb.green >> 8;
   *b = ct.argb.blue  >> 8;

   if (a) {
      *a = ct.argb.alpha >> 8;
   }
}

void QColor::setRgbF(qreal r, qreal g, qreal b, qreal a)
{
   if (r < qreal(0.0) || r > qreal(1.0)
         || g < qreal(0.0) || g > qreal(1.0)
         || b < qreal(0.0) || b > qreal(1.0)
         || a < qreal(0.0) || a > qreal(1.0)) {
      qWarning("QColor::setRgbF: RGB parameters out of range");
      invalidate();
      return;
   }

   cspec = Rgb;
   ct.argb.alpha = qRound(a * USHRT_MAX);
   ct.argb.red   = qRound(r * USHRT_MAX);
   ct.argb.green = qRound(g * USHRT_MAX);
   ct.argb.blue  = qRound(b * USHRT_MAX);
   ct.argb.pad   = 0;
}

void QColor::setRgb(int r, int g, int b, int a)
{
   if ((uint)r > 255 || (uint)g > 255 || (uint)b > 255 || (uint)a > 255) {
      qWarning("QColor::setRgb: RGB parameters out of range");
      invalidate();
      return;
   }

   cspec = Rgb;
   ct.argb.alpha = a * 0x101;
   ct.argb.red   = r * 0x101;
   ct.argb.green = g * 0x101;
   ct.argb.blue  = b * 0x101;
   ct.argb.pad   = 0;
}

/*!
    \obsolete
    \fn void QColor::setRgba(int r, int g, int b, int a)

    Use setRgb() instead.
*/

QRgb QColor::rgba() const
{
   if (cspec != Invalid && cspec != Rgb) {
      return toRgb().rgba();
   }
   return qRgba(ct.argb.red >> 8, ct.argb.green >> 8, ct.argb.blue >> 8, ct.argb.alpha >> 8);
}

void QColor::setRgba(QRgb rgba)
{
   cspec = Rgb;
   ct.argb.alpha = qAlpha(rgba) * 0x101;
   ct.argb.red   = qRed(rgba)   * 0x101;
   ct.argb.green = qGreen(rgba) * 0x101;
   ct.argb.blue  = qBlue(rgba)  * 0x101;
   ct.argb.pad   = 0;
}

QRgb QColor::rgb() const
{
   if (cspec != Invalid && cspec != Rgb) {
      return toRgb().rgb();
   }
   return qRgb(ct.argb.red >> 8, ct.argb.green >> 8, ct.argb.blue >> 8);
}

void QColor::setRgb(QRgb rgb)
{
   cspec = Rgb;
   ct.argb.alpha = 0xffff;
   ct.argb.red   = qRed(rgb)   * 0x101;
   ct.argb.green = qGreen(rgb) * 0x101;
   ct.argb.blue  = qBlue(rgb)  * 0x101;
   ct.argb.pad   = 0;
}

int QColor::alpha() const
{
   return ct.argb.alpha >> 8;
}


void QColor::setAlpha(int alpha)
{
   QCOLOR_INT_RANGE_CHECK("QColor::setAlpha", alpha);
   ct.argb.alpha = alpha * 0x101;
}

qreal QColor::alphaF() const
{
   return ct.argb.alpha / qreal(USHRT_MAX);
}

void QColor::setAlphaF(qreal alpha)
{
   QCOLOR_REAL_RANGE_CHECK("QColor::setAlphaF", alpha);
   qreal tmp = alpha * USHRT_MAX;
   ct.argb.alpha = qRound(tmp);
}

int QColor::red() const
{
   if (cspec != Invalid && cspec != Rgb) {
      return toRgb().red();
   }
   return ct.argb.red >> 8;
}

void QColor::setRed(int red)
{
   QCOLOR_INT_RANGE_CHECK("QColor::setRed", red);
   if (cspec != Rgb) {
      setRgb(red, green(), blue(), alpha());
   } else {
      ct.argb.red = red * 0x101;
   }
}

int QColor::green() const
{
   if (cspec != Invalid && cspec != Rgb) {
      return toRgb().green();
   }
   return ct.argb.green >> 8;
}

void QColor::setGreen(int green)
{
   QCOLOR_INT_RANGE_CHECK("QColor::setGreen", green);
   if (cspec != Rgb) {
      setRgb(red(), green, blue(), alpha());
   } else {
      ct.argb.green = green * 0x101;
   }
}

int QColor::blue() const
{
   if (cspec != Invalid && cspec != Rgb) {
      return toRgb().blue();
   }
   return ct.argb.blue >> 8;
}

void QColor::setBlue(int blue)
{
   QCOLOR_INT_RANGE_CHECK("QColor::setBlue", blue);
   if (cspec != Rgb) {
      setRgb(red(), green(), blue, alpha());
   } else {
      ct.argb.blue = blue * 0x101;
   }
}

qreal QColor::redF() const
{
   if (cspec != Invalid && cspec != Rgb) {
      return toRgb().redF();
   }
   return ct.argb.red / qreal(USHRT_MAX);
}

void QColor::setRedF(qreal red)
{
   QCOLOR_REAL_RANGE_CHECK("QColor::setRedF", red);
   if (cspec != Rgb) {
      setRgbF(red, greenF(), blueF(), alphaF());
   } else {
      ct.argb.red = qRound(red * USHRT_MAX);
   }
}

qreal QColor::greenF() const
{
   if (cspec != Invalid && cspec != Rgb) {
      return toRgb().greenF();
   }
   return ct.argb.green / qreal(USHRT_MAX);
}

void QColor::setGreenF(qreal green)
{
   QCOLOR_REAL_RANGE_CHECK("QColor::setGreenF", green);
   if (cspec != Rgb) {
      setRgbF(redF(), green, blueF(), alphaF());
   } else {
      ct.argb.green = qRound(green * USHRT_MAX);
   }
}

qreal QColor::blueF() const
{
   if (cspec != Invalid && cspec != Rgb) {
      return toRgb().blueF();
   }
   return ct.argb.blue / qreal(USHRT_MAX);
}

void QColor::setBlueF(qreal blue)
{
   QCOLOR_REAL_RANGE_CHECK("QColor::setBlueF", blue);
   if (cspec != Rgb) {
      setRgbF(redF(), greenF(), blue, alphaF());
   } else {
      ct.argb.blue = qRound(blue * USHRT_MAX);
   }
}

int QColor::hue() const
{
   return hsvHue();
}

int QColor::hsvHue() const
{
   if (cspec != Invalid && cspec != Hsv) {
      return toHsv().hue();
   }
   return ct.ahsv.hue == USHRT_MAX ? -1 : ct.ahsv.hue / 100;
}

int QColor::saturation() const
{
   return hsvSaturation();
}

int QColor::hsvSaturation() const
{
   if (cspec != Invalid && cspec != Hsv) {
      return toHsv().saturation();
   }
   return ct.ahsv.saturation >> 8;
}

int QColor::value() const
{
   if (cspec != Invalid && cspec != Hsv) {
      return toHsv().value();
   }
   return ct.ahsv.value >> 8;
}

qreal QColor::hueF() const
{
   return hsvHueF();
}

qreal QColor::hsvHueF() const
{
   if (cspec != Invalid && cspec != Hsv) {
      return toHsv().hueF();
   }
   return ct.ahsv.hue == USHRT_MAX ? qreal(-1.0) : ct.ahsv.hue / qreal(36000.0);
}

qreal QColor::saturationF() const
{
   return hsvSaturationF();
}

qreal QColor::hsvSaturationF() const
{
   if (cspec != Invalid && cspec != Hsv) {
      return toHsv().saturationF();
   }
   return ct.ahsv.saturation / qreal(USHRT_MAX);
}

qreal QColor::valueF() const
{
   if (cspec != Invalid && cspec != Hsv) {
      return toHsv().valueF();
   }
   return ct.ahsv.value / qreal(USHRT_MAX);
}

int QColor::hslHue() const
{
   if (cspec != Invalid && cspec != Hsl) {
      return toHsl().hslHue();
   }
   return ct.ahsl.hue == USHRT_MAX ? -1 : ct.ahsl.hue / 100;
}

int QColor::hslSaturation() const
{
   if (cspec != Invalid && cspec != Hsl) {
      return toHsl().hslSaturation();
   }
   return ct.ahsl.saturation >> 8;
}

int QColor::lightness() const
{
   if (cspec != Invalid && cspec != Hsl) {
      return toHsl().lightness();
   }
   return ct.ahsl.lightness >> 8;
}

qreal QColor::hslHueF() const
{
   if (cspec != Invalid && cspec != Hsl) {
      return toHsl().hslHueF();
   }
   return ct.ahsl.hue == USHRT_MAX ? qreal(-1.0) : ct.ahsl.hue / qreal(36000.0);
}

qreal QColor::hslSaturationF() const
{
   if (cspec != Invalid && cspec != Hsl) {
      return toHsl().hslSaturationF();
   }
   return ct.ahsl.saturation / qreal(USHRT_MAX);
}

qreal QColor::lightnessF() const
{
   if (cspec != Invalid && cspec != Hsl) {
      return toHsl().lightnessF();
   }
   return ct.ahsl.lightness / qreal(USHRT_MAX);
}

int QColor::cyan() const
{
   if (cspec != Invalid && cspec != Cmyk) {
      return toCmyk().cyan();
   }
   return ct.acmyk.cyan >> 8;
}

int QColor::magenta() const
{
   if (cspec != Invalid && cspec != Cmyk) {
      return toCmyk().magenta();
   }
   return ct.acmyk.magenta >> 8;
}

int QColor::yellow() const
{
   if (cspec != Invalid && cspec != Cmyk) {
      return toCmyk().yellow();
   }
   return ct.acmyk.yellow >> 8;
}

int QColor::black() const
{
   if (cspec != Invalid && cspec != Cmyk) {
      return toCmyk().black();
   }
   return ct.acmyk.black >> 8;
}

qreal QColor::cyanF() const
{
   if (cspec != Invalid && cspec != Cmyk) {
      return toCmyk().cyanF();
   }
   return ct.acmyk.cyan / qreal(USHRT_MAX);
}

qreal QColor::magentaF() const
{
   if (cspec != Invalid && cspec != Cmyk) {
      return toCmyk().magentaF();
   }
   return ct.acmyk.magenta / qreal(USHRT_MAX);
}

qreal QColor::yellowF() const
{
   if (cspec != Invalid && cspec != Cmyk) {
      return toCmyk().yellowF();
   }
   return ct.acmyk.yellow / qreal(USHRT_MAX);
}

qreal QColor::blackF() const
{
   if (cspec != Invalid && cspec != Cmyk) {
      return toCmyk().blackF();
   }
   return ct.acmyk.black / qreal(USHRT_MAX);
}

QColor QColor::toRgb() const
{
   if (!isValid() || cspec == Rgb) {
      return *this;
   }

   QColor color;
   color.cspec = Rgb;
   color.ct.argb.alpha = ct.argb.alpha;
   color.ct.argb.pad = 0;

   switch (cspec) {
      case Hsv: {
         if (ct.ahsv.saturation == 0 || ct.ahsv.hue == USHRT_MAX) {
            // achromatic case
            color.ct.argb.red = color.ct.argb.green = color.ct.argb.blue = ct.ahsv.value;
            break;
         }

         // chromatic case
         const qreal h = ct.ahsv.hue == 36000 ? 0 : ct.ahsv.hue / qreal(6000.);
         const qreal s = ct.ahsv.saturation / qreal(USHRT_MAX);
         const qreal v = ct.ahsv.value / qreal(USHRT_MAX);
         const int i = int(h);
         const qreal f = h - i;
         const qreal p = v * (qreal(1.0) - s);

         if (i & 1) {
            const qreal q = v * (qreal(1.0) - (s * f));

            switch (i) {
               case 1:
                  color.ct.argb.red   = qRound(q * USHRT_MAX);
                  color.ct.argb.green = qRound(v * USHRT_MAX);
                  color.ct.argb.blue  = qRound(p * USHRT_MAX);
                  break;
               case 3:
                  color.ct.argb.red   = qRound(p * USHRT_MAX);
                  color.ct.argb.green = qRound(q * USHRT_MAX);
                  color.ct.argb.blue  = qRound(v * USHRT_MAX);
                  break;
               case 5:
                  color.ct.argb.red   = qRound(v * USHRT_MAX);
                  color.ct.argb.green = qRound(p * USHRT_MAX);
                  color.ct.argb.blue  = qRound(q * USHRT_MAX);
                  break;
            }
         } else {
            const qreal t = v * (qreal(1.0) - (s * (qreal(1.0) - f)));

            switch (i) {
               case 0:
                  color.ct.argb.red   = qRound(v * USHRT_MAX);
                  color.ct.argb.green = qRound(t * USHRT_MAX);
                  color.ct.argb.blue  = qRound(p * USHRT_MAX);
                  break;
               case 2:
                  color.ct.argb.red   = qRound(p * USHRT_MAX);
                  color.ct.argb.green = qRound(v * USHRT_MAX);
                  color.ct.argb.blue  = qRound(t * USHRT_MAX);
                  break;
               case 4:
                  color.ct.argb.red   = qRound(t * USHRT_MAX);
                  color.ct.argb.green = qRound(p * USHRT_MAX);
                  color.ct.argb.blue  = qRound(v * USHRT_MAX);
                  break;
            }
         }
         break;
      }
      case Hsl: {
         if (ct.ahsl.saturation == 0 || ct.ahsl.hue == USHRT_MAX) {
            // achromatic case
            color.ct.argb.red = color.ct.argb.green = color.ct.argb.blue = ct.ahsl.lightness;
         } else if (ct.ahsl.lightness == 0) {
            // lightness 0
            color.ct.argb.red = color.ct.argb.green = color.ct.argb.blue = 0;
         } else {
            // chromatic case
            const qreal h = ct.ahsl.hue == 36000 ? 0 : ct.ahsl.hue / qreal(36000.);
            const qreal s = ct.ahsl.saturation / qreal(USHRT_MAX);
            const qreal l = ct.ahsl.lightness / qreal(USHRT_MAX);

            qreal temp2;
            if (l < qreal(0.5)) {
               temp2 = l * (qreal(1.0) + s);
            } else {
               temp2 = l + s - (l * s);
            }

            const qreal temp1 = (qreal(2.0) * l) - temp2;
            qreal temp3[3] = { h + (qreal(1.0) / qreal(3.0)),
                               h,
                               h - (qreal(1.0) / qreal(3.0))
                             };

            for (int i = 0; i != 3; ++i) {
               if (temp3[i] < qreal(0.0)) {
                  temp3[i] += qreal(1.0);
               } else if (temp3[i] > qreal(1.0)) {
                  temp3[i] -= qreal(1.0);
               }

               const qreal sixtemp3 = temp3[i] * qreal(6.0);
               if (sixtemp3 < qreal(1.0)) {
                  color.ct.array[i + 1] = qRound((temp1 + (temp2 - temp1) * sixtemp3) * USHRT_MAX);
               } else if ((temp3[i] * qreal(2.0)) < qreal(1.0)) {
                  color.ct.array[i + 1] = qRound(temp2 * USHRT_MAX);
               } else if ((temp3[i] * qreal(3.0)) < qreal(2.0)) {
                  color.ct.array[i + 1] = qRound((temp1 + (temp2 - temp1) * (qreal(2.0) / qreal(3.0) - temp3[i]) * qreal(
                                                     6.0)) * USHRT_MAX);
               } else {
                  color.ct.array[i + 1] = qRound(temp1 * USHRT_MAX);
               }
            }
            color.ct.argb.red = color.ct.argb.red == 1 ? 0 : color.ct.argb.red;
            color.ct.argb.green = color.ct.argb.green == 1 ? 0 : color.ct.argb.green;
            color.ct.argb.blue = color.ct.argb.blue == 1 ? 0 : color.ct.argb.blue;
         }
         break;
      }
      case Cmyk: {
         const qreal c = ct.acmyk.cyan / qreal(USHRT_MAX);
         const qreal m = ct.acmyk.magenta / qreal(USHRT_MAX);
         const qreal y = ct.acmyk.yellow / qreal(USHRT_MAX);
         const qreal k = ct.acmyk.black / qreal(USHRT_MAX);

         color.ct.argb.red   = qRound((qreal(1.0) - (c * (qreal(1.0) - k) + k)) * USHRT_MAX);
         color.ct.argb.green = qRound((qreal(1.0) - (m * (qreal(1.0) - k) + k)) * USHRT_MAX);
         color.ct.argb.blue  = qRound((qreal(1.0) - (y * (qreal(1.0) - k) + k)) * USHRT_MAX);
         break;
      }
      default:
         break;
   }

   return color;
}


#define Q_MAX_3(a, b, c) ( ( a > b && a > c) ? a : (b > c ? b : c) )
#define Q_MIN_3(a, b, c) ( ( a < b && a < c) ? a : (b < c ? b : c) )

QColor QColor::toHsv() const
{
   if (!isValid() || cspec == Hsv) {
      return *this;
   }

   if (cspec != Rgb) {
      return toRgb().toHsv();
   }

   QColor color;
   color.cspec = Hsv;
   color.ct.ahsv.alpha = ct.argb.alpha;
   color.ct.ahsv.pad = 0;

   const qreal r = ct.argb.red   / qreal(USHRT_MAX);
   const qreal g = ct.argb.green / qreal(USHRT_MAX);
   const qreal b = ct.argb.blue  / qreal(USHRT_MAX);
   const qreal max = Q_MAX_3(r, g, b);
   const qreal min = Q_MIN_3(r, g, b);
   const qreal delta = max - min;
   color.ct.ahsv.value = qRound(max * USHRT_MAX);
   if (qFuzzyIsNull(delta)) {
      // achromatic case, hue is undefined
      color.ct.ahsv.hue = USHRT_MAX;
      color.ct.ahsv.saturation = 0;
   } else {
      // chromatic case
      qreal hue = 0;
      color.ct.ahsv.saturation = qRound((delta / max) * USHRT_MAX);
      if (qFuzzyCompare(r, max)) {
         hue = ((g - b) / delta);
      } else if (qFuzzyCompare(g, max)) {
         hue = (qreal(2.0) + (b - r) / delta);
      } else if (qFuzzyCompare(b, max)) {
         hue = (qreal(4.0) + (r - g) / delta);
      } else {
         Q_ASSERT_X(false, "QColor::toHsv", "internal error");
      }
      hue *= qreal(60.0);
      if (hue < qreal(0.0)) {
         hue += qreal(360.0);
      }
      color.ct.ahsv.hue = qRound(hue * 100);
   }

   return color;
}

QColor QColor::toHsl() const
{
   if (!isValid() || cspec == Hsl) {
      return *this;
   }

   if (cspec != Rgb) {
      return toRgb().toHsl();
   }

   QColor color;
   color.cspec = Hsl;
   color.ct.ahsl.alpha = ct.argb.alpha;
   color.ct.ahsl.pad = 0;

   const qreal r = ct.argb.red   / qreal(USHRT_MAX);
   const qreal g = ct.argb.green / qreal(USHRT_MAX);
   const qreal b = ct.argb.blue  / qreal(USHRT_MAX);
   const qreal max = Q_MAX_3(r, g, b);
   const qreal min = Q_MIN_3(r, g, b);
   const qreal delta = max - min;
   const qreal delta2 = max + min;
   const qreal lightness = qreal(0.5) * delta2;
   color.ct.ahsl.lightness = qRound(lightness * USHRT_MAX);
   if (qFuzzyIsNull(delta)) {
      // achromatic case, hue is undefined
      color.ct.ahsl.hue = USHRT_MAX;
      color.ct.ahsl.saturation = 0;
   } else {
      // chromatic case
      qreal hue = 0;
      if (lightness < qreal(0.5)) {
         color.ct.ahsl.saturation = qRound((delta / delta2) * USHRT_MAX);
      } else {
         color.ct.ahsl.saturation = qRound((delta / (qreal(2.0) - delta2)) * USHRT_MAX);
      }
      if (qFuzzyCompare(r, max)) {
         hue = ((g - b) / delta);
      } else if (qFuzzyCompare(g, max)) {
         hue = (qreal(2.0) + (b - r) / delta);
      } else if (qFuzzyCompare(b, max)) {
         hue = (qreal(4.0) + (r - g) / delta);
      } else {
         Q_ASSERT_X(false, "QColor::toHsv", "internal error");
      }
      hue *= qreal(60.0);
      if (hue < qreal(0.0)) {
         hue += qreal(360.0);
      }
      color.ct.ahsl.hue = qRound(hue * 100);
   }

   return color;
}

QColor QColor::toCmyk() const
{
   if (!isValid() || cspec == Cmyk) {
      return *this;
   }
   if (cspec != Rgb) {
      return toRgb().toCmyk();
   }

   QColor color;
   color.cspec = Cmyk;
   color.ct.acmyk.alpha = ct.argb.alpha;

   // rgb -> cmy
   const qreal r = ct.argb.red   / qreal(USHRT_MAX);
   const qreal g = ct.argb.green / qreal(USHRT_MAX);
   const qreal b = ct.argb.blue  / qreal(USHRT_MAX);
   qreal c = qreal(1.0) - r;
   qreal m = qreal(1.0) - g;
   qreal y = qreal(1.0) - b;

   // cmy -> cmyk
   const qreal k = qMin(c, qMin(m, y));

   if (!qFuzzyIsNull(k - 1)) {
      c = (c - k) / (qreal(1.0) - k);
      m = (m - k) / (qreal(1.0) - k);
      y = (y - k) / (qreal(1.0) - k);
   }

   color.ct.acmyk.cyan    = qRound(c * USHRT_MAX);
   color.ct.acmyk.magenta = qRound(m * USHRT_MAX);
   color.ct.acmyk.yellow  = qRound(y * USHRT_MAX);
   color.ct.acmyk.black   = qRound(k * USHRT_MAX);

   return color;
}

QColor QColor::convertTo(QColor::Spec colorSpec) const
{
   if (colorSpec == cspec) {
      return *this;
   }
   switch (colorSpec) {
      case Rgb:
         return toRgb();
      case Hsv:
         return toHsv();
      case Cmyk:
         return toCmyk();
      case Hsl:
         return toHsl();
      case Invalid:
         break;
   }
   return QColor(); // must be invalid
}

QColor QColor::fromRgb(QRgb rgb)
{
   return fromRgb(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

QColor QColor::fromRgba(QRgb rgba)
{
   return fromRgb(qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba));
}

QColor QColor::fromRgb(int r, int g, int b, int a)
{
   if (r < 0 || r > 255
         || g < 0 || g > 255
         || b < 0 || b > 255
         || a < 0 || a > 255) {
      qWarning("QColor::fromRgb: RGB parameters out of range");
      return QColor();
   }

   QColor color;
   color.cspec = Rgb;
   color.ct.argb.alpha = a * 0x101;
   color.ct.argb.red   = r * 0x101;
   color.ct.argb.green = g * 0x101;
   color.ct.argb.blue  = b * 0x101;
   color.ct.argb.pad   = 0;
   return color;
}

QColor QColor::fromRgbF(qreal r, qreal g, qreal b, qreal a)
{
   if (r < qreal(0.0) || r > qreal(1.0)
         || g < qreal(0.0) || g > qreal(1.0)
         || b < qreal(0.0) || b > qreal(1.0)
         || a < qreal(0.0) || a > qreal(1.0)) {
      qWarning("QColor::fromRgbF: RGB parameters out of range");
      return QColor();
   }

   QColor color;
   color.cspec = Rgb;
   color.ct.argb.alpha = qRound(a * USHRT_MAX);
   color.ct.argb.red   = qRound(r * USHRT_MAX);
   color.ct.argb.green = qRound(g * USHRT_MAX);
   color.ct.argb.blue  = qRound(b * USHRT_MAX);
   color.ct.argb.pad   = 0;
   return color;
}

QColor QColor::fromHsv(int h, int s, int v, int a)
{
   if (((h < 0 || h >= 360) && h != -1)
         || s < 0 || s > 255
         || v < 0 || v > 255
         || a < 0 || a > 255) {
      qWarning("QColor::fromHsv: HSV parameters out of range");
      return QColor();
   }

   QColor color;
   color.cspec = Hsv;
   color.ct.ahsv.alpha      = a * 0x101;
   color.ct.ahsv.hue        = h == -1 ? USHRT_MAX : (h % 360) * 100;
   color.ct.ahsv.saturation = s * 0x101;
   color.ct.ahsv.value      = v * 0x101;
   color.ct.ahsv.pad        = 0;
   return color;
}

QColor QColor::fromHsvF(qreal h, qreal s, qreal v, qreal a)
{
   if (((h < qreal(0.0) || h > qreal(1.0)) && h != qreal(-1.0))
         || (s < qreal(0.0) || s > qreal(1.0))
         || (v < qreal(0.0) || v > qreal(1.0))
         || (a < qreal(0.0) || a > qreal(1.0))) {
      qWarning("QColor::fromHsvF: HSV parameters out of range");
      return QColor();
   }

   QColor color;
   color.cspec = Hsv;
   color.ct.ahsv.alpha      = ushort(qRound(a * USHRT_MAX));
   color.ct.ahsv.hue        = ushort(h == qreal(-1.0) ? USHRT_MAX : qRound(h * 36000));
   color.ct.ahsv.saturation = ushort(qRound(s * USHRT_MAX));
   color.ct.ahsv.value      = ushort(qRound(v * USHRT_MAX));
   color.ct.ahsv.pad        = 0;
   return color;
}

QColor QColor::fromHsl(int h, int s, int l, int a)
{
   if (((h < 0 || h >= 360) && h != -1)
         || s < 0 || s > 255
         || l < 0 || l > 255
         || a < 0 || a > 255) {
      qWarning("QColor::fromHsv: HSV parameters out of range");
      return QColor();
   }

   QColor color;
   color.cspec = Hsl;
   color.ct.ahsl.alpha      = ushort(a * 0x101);
   color.ct.ahsl.hue        = h == -1 ? USHRT_MAX : (h % 360) * 100;
   color.ct.ahsl.saturation = ushort(s * 0x101);
   color.ct.ahsl.lightness  = ushort(l * 0x101);
   color.ct.ahsl.pad        = 0;
   return color;
}

QColor QColor::fromHslF(qreal h, qreal s, qreal l, qreal a)
{
   if (((h < qreal(0.0) || h > qreal(1.0)) && h != qreal(-1.0))
         || (s < qreal(0.0) || s > qreal(1.0))
         || (l < qreal(0.0) || l > qreal(1.0))
         || (a < qreal(0.0) || a > qreal(1.0))) {
      qWarning("QColor::fromHsvF: HSV parameters out of range");
      return QColor();
   }

   QColor color;
   color.cspec = Hsl;
   color.ct.ahsl.alpha      = ushort(qRound(a * USHRT_MAX));
   color.ct.ahsl.hue        = ushort((h == qreal(-1.0)) ? USHRT_MAX : qRound(h * 36000));

   if (color.ct.ahsl.hue == 36000) {
      color.ct.ahsl.hue = 0;
   }

   color.ct.ahsl.saturation = ushort(qRound(s * USHRT_MAX));
   color.ct.ahsl.lightness  = ushort(qRound(l * USHRT_MAX));
   color.ct.ahsl.pad        = 0;
   return color;
}

void QColor::getCmyk(int *c, int *m, int *y, int *k, int *a)
{
   if (!c || !m || !y || !k) {
      return;
   }

   if (cspec != Invalid && cspec != Cmyk) {
      toCmyk().getCmyk(c, m, y, k, a);
      return;
   }

   *c = ct.acmyk.cyan >> 8;
   *m = ct.acmyk.magenta >> 8;
   *y = ct.acmyk.yellow >> 8;
   *k = ct.acmyk.black >> 8;

   if (a) {
      *a = ct.acmyk.alpha >> 8;
   }
}

void QColor::getCmykF(qreal *c, qreal *m, qreal *y, qreal *k, qreal *a)
{
   if (!c || !m || !y || !k) {
      return;
   }

   if (cspec != Invalid && cspec != Cmyk) {
      toCmyk().getCmykF(c, m, y, k, a);
      return;
   }

   *c = ct.acmyk.cyan    / qreal(USHRT_MAX);
   *m = ct.acmyk.magenta / qreal(USHRT_MAX);
   *y = ct.acmyk.yellow  / qreal(USHRT_MAX);
   *k = ct.acmyk.black   / qreal(USHRT_MAX);

   if (a) {
      *a = ct.acmyk.alpha / qreal(USHRT_MAX);
   }
}

void QColor::setCmyk(int c, int m, int y, int k, int a)
{
   if (c < 0 || c > 255
         || m < 0 || m > 255
         || y < 0 || y > 255
         || k < 0 || k > 255
         || a < 0 || a > 255) {
      qWarning("QColor::setCmyk: CMYK parameters out of range");
      return;
   }

   cspec = Cmyk;
   ct.acmyk.alpha   = ushort(a * 0x101);
   ct.acmyk.cyan    = ushort(c * 0x101);
   ct.acmyk.magenta = ushort(m * 0x101);
   ct.acmyk.yellow  = ushort(y * 0x101);
   ct.acmyk.black   = ushort(k * 0x101);
}

void QColor::setCmykF(qreal c, qreal m, qreal y, qreal k, qreal a)
{
   if (c < qreal(0.0) || c > qreal(1.0)
         || m < qreal(0.0) || m > qreal(1.0)
         || y < qreal(0.0) || y > qreal(1.0)
         || k < qreal(0.0) || k > qreal(1.0)
         || a < qreal(0.0) || a > qreal(1.0)) {
      qWarning("QColor::setCmykF: CMYK parameters out of range");
      return;
   }

   cspec = Cmyk;
   ct.acmyk.alpha   = ushort(qRound(a * USHRT_MAX));
   ct.acmyk.cyan    = ushort(qRound(c * USHRT_MAX));
   ct.acmyk.magenta = ushort(qRound(m * USHRT_MAX));
   ct.acmyk.yellow  = ushort(qRound(y * USHRT_MAX));
   ct.acmyk.black   = ushort(qRound(k * USHRT_MAX));
}

QColor QColor::fromCmyk(int c, int m, int y, int k, int a)
{
   if (c < 0 || c > 255
         || m < 0 || m > 255
         || y < 0 || y > 255
         || k < 0 || k > 255
         || a < 0 || a > 255) {
      qWarning("QColor::fromCmyk: CMYK parameters out of range");
      return QColor();
   }

   QColor color;
   color.cspec = Cmyk;
   color.ct.acmyk.alpha   = ushort(a * 0x101);
   color.ct.acmyk.cyan    = ushort(c * 0x101);
   color.ct.acmyk.magenta = ushort(m * 0x101);
   color.ct.acmyk.yellow  = ushort(y * 0x101);
   color.ct.acmyk.black   = ushort(k * 0x101);
   return color;
}

QColor QColor::fromCmykF(qreal c, qreal m, qreal y, qreal k, qreal a)
{
   if (c < qreal(0.0) || c > qreal(1.0)
         || m < qreal(0.0) || m > qreal(1.0)
         || y < qreal(0.0) || y > qreal(1.0)
         || k < qreal(0.0) || k > qreal(1.0)
         || a < qreal(0.0) || a > qreal(1.0)) {
      qWarning("QColor::fromCmykF: CMYK parameters out of range");
      return QColor();
   }

   QColor color;
   color.cspec = Cmyk;
   color.ct.acmyk.alpha   = ushort(qRound(a * USHRT_MAX));
   color.ct.acmyk.cyan    = ushort(qRound(c * USHRT_MAX));
   color.ct.acmyk.magenta = ushort(qRound(m * USHRT_MAX));
   color.ct.acmyk.yellow  = ushort(qRound(y * USHRT_MAX));
   color.ct.acmyk.black   = ushort(qRound(k * USHRT_MAX));
   return color;
}


/*!
    \obsolete

    Use lighter(\a factor) instead.
*/
QColor QColor::light(int factor) const
{
   if (factor <= 0) {                              // invalid lightness factor
      return *this;
   } else if (factor < 100) {                    // makes color darker
      return darker(10000 / factor);
   }

   QColor hsv = toHsv();
   int s = hsv.ct.ahsv.saturation;
   int v = hsv.ct.ahsv.value;

   v = (factor * v) / 100;
   if (v > USHRT_MAX) {
      // overflow... adjust saturation
      s -= v - USHRT_MAX;
      if (s < 0) {
         s = 0;
      }
      v = USHRT_MAX;
   }

   hsv.ct.ahsv.saturation = ushort(s);
   hsv.ct.ahsv.value = ushort(v);

   // convert back to same color spec as original color
   return hsv.convertTo(cspec);
}

/*!
    \obsolete

    Use darker(\a factor) instead.
*/
QColor QColor::dark(int factor) const
{
   if (factor <= 0) {                              // invalid darkness factor
      return *this;
   } else if (factor < 100) {                    // makes color lighter
      return lighter(10000 / factor);
   }

   QColor hsv = toHsv();
   hsv.ct.ahsv.value = (hsv.ct.ahsv.value * 100) / factor;

   // convert back to same color spec as original color
   return hsv.convertTo(cspec);
}

QColor &QColor::operator=(const QColor &color)
{
   cspec = color.cspec;
   ct.argb = color.ct.argb;
   return *this;
}

QColor &QColor::operator=(Qt::GlobalColor color)
{
   return operator=(QColor(color));
}

bool QColor::operator==(const QColor &color) const
{
   if (cspec == Hsl && cspec == color.cspec) {
      return (ct.argb.alpha == color.ct.argb.alpha
              && ((((ct.ahsl.hue % 36000) == (color.ct.ahsl.hue % 36000)))
                  || (ct.ahsl.hue == color.ct.ahsl.hue))
              && (qAbs(ct.ahsl.saturation - color.ct.ahsl.saturation) < 50
                  || ct.ahsl.lightness == 0
                  || color.ct.ahsl.lightness == 0
                  || ct.ahsl.lightness == USHRT_MAX
                  || color.ct.ahsl.lightness == USHRT_MAX)
              && (qAbs(ct.ahsl.lightness - color.ct.ahsl.lightness)) < 50);
   } else {
      return (cspec == color.cspec
              && ct.argb.alpha == color.ct.argb.alpha
              && (((cspec == QColor::Hsv)
                   && ((ct.ahsv.hue % 36000) == (color.ct.ahsv.hue % 36000)))
                  || (ct.ahsv.hue == color.ct.ahsv.hue))
              && ct.argb.green == color.ct.argb.green
              && ct.argb.blue  == color.ct.argb.blue
              && ct.argb.pad   == color.ct.argb.pad);
   }
}

bool QColor::operator!=(const QColor &color) const
{
   return !operator==(color);
}

QColor::operator QVariant() const
{
   return QVariant(QVariant::Color, this);
}

#ifdef Q_WS_X11

bool QColor::allowX11ColorNames()
{
   return ::allowX11ColorNames;
}

void QColor::setAllowX11ColorNames(bool enabled)
{
   ::allowX11ColorNames = enabled;
}
#endif

// internal
void QColor::invalidate()
{
   cspec = Invalid;
   ct.argb.alpha = USHRT_MAX;
   ct.argb.red = 0;
   ct.argb.green = 0;
   ct.argb.blue = 0;
   ct.argb.pad = 0;
}

/*****************************************************************************
  QColor stream functions
 *****************************************************************************/

QDebug operator<<(QDebug dbg, const QColor &c)
{
   if (!c.isValid()) {
      dbg.nospace() << "QColor(Invalid)";
   } else if (c.spec() == QColor::Rgb) {
      dbg.nospace() << "QColor(ARGB " << c.alphaF() << ", " << c.redF() << ", " << c.greenF() << ", " << c.blueF() << ')';
   } else if (c.spec() == QColor::Hsv) {
      dbg.nospace() << "QColor(AHSV " << c.alphaF() << ", " << c.hueF() << ", " << c.saturationF() << ", " << c.valueF() <<
                    ')';
   } else if (c.spec() == QColor::Cmyk)
      dbg.nospace() << "QColor(ACMYK " << c.alphaF() << ", " << c.cyanF() << ", " << c.magentaF() << ", " << c.yellowF() <<
                    ", "
                    << c.blackF() << ')';
   else if (c.spec() == QColor::Hsl) {
      dbg.nospace() << "QColor(AHSL " << c.alphaF() << ", " << c.hslHueF() << ", " << c.hslSaturationF() << ", " <<
                    c.lightnessF() << ')';
   }

   return dbg.space();
}

QDataStream &operator<<(QDataStream &stream, const QColor &color)
{
   qint8   s = color.cspec;
   quint16 a = color.ct.argb.alpha;
   quint16 r = color.ct.argb.red;
   quint16 g = color.ct.argb.green;
   quint16 b = color.ct.argb.blue;
   quint16 p = color.ct.argb.pad;

   stream << s;
   stream << a;
   stream << r;
   stream << g;
   stream << b;
   stream << p;

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QColor &color)
{
   qint8 s;
   quint16 a, r, g, b, p;
   stream >> s;
   stream >> a;
   stream >> r;
   stream >> g;
   stream >> b;
   stream >> p;

   color.cspec = QColor::Spec(s);
   color.ct.argb.alpha = a;
   color.ct.argb.red   = r;
   color.ct.argb.green = g;
   color.ct.argb.blue  = b;
   color.ct.argb.pad   = p;

   return stream;
}

