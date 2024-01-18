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

#ifndef QCOLOR_H
#define QCOLOR_H

#include <qrgb.h>
#include <qnamespace.h>
#include <qstringlist.h>
#include <qrgba64.h>

class QColor;
class QColormap;
class QVariant;

Q_GUI_EXPORT QDebug operator<<(QDebug, const QColor &);
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QColor &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QColor &);

class Q_GUI_EXPORT QColor
{
 public:
   enum Spec { Invalid, Rgb, Hsv, Cmyk, Hsl };
   enum NameFormat { HexRgb, HexArgb };

   QColor();
   QColor(Qt::GlobalColor color);
   QColor(int r, int g, int b, int a = 255);
   QColor(QRgb rgb);
   QColor(QRgba64 rgba64);
   QColor(const QString &name);
   QColor(const char *name);
   QColor(Spec spec);

   QColor &operator=(Qt::GlobalColor color);

   bool isValid() const;

   QString name(NameFormat format = HexRgb) const;
   void setNamedColor(const QString &name);

   static QStringList colorNames();

   inline Spec spec() const {
      return cspec;
   }

   int alpha() const;
   void setAlpha(int alpha);

   qreal alphaF() const;
   void setAlphaF(qreal alpha);

   int red() const;
   int green() const;
   int blue() const;
   void setRed(int red);
   void setGreen(int green);
   void setBlue(int blue);

   qreal redF() const;
   qreal greenF() const;
   qreal blueF() const;
   void setRedF(qreal red);
   void setGreenF(qreal green);
   void setBlueF(qreal blue);

   void getRgb(int *r, int *g, int *b, int *a = nullptr) const;
   void setRgb(int r, int g, int b, int a = 255);

   void getRgbF(qreal *r, qreal *g, qreal *b, qreal *a = nullptr) const;
   void setRgbF(qreal r, qreal g, qreal b, qreal a = 1.0);

   QRgb rgba() const;
   void setRgba(QRgb rgba);
   QRgba64 rgba64() const;
   void setRgba64(QRgba64 rgba);

   QRgb rgb() const;
   void setRgb(QRgb rgb);

   int hue() const; // 0 <= hue < 360
   int saturation() const;
   int hsvHue() const; // 0 <= hue < 360
   int hsvSaturation() const;
   int value() const;

   qreal hueF() const; // 0.0 <= hueF < 360.0
   qreal saturationF() const;
   qreal hsvHueF() const; // 0.0 <= hueF < 360.0
   qreal hsvSaturationF() const;
   qreal valueF() const;

   void getHsv(int *h, int *s, int *v, int *a = nullptr) const;
   void setHsv(int h, int s, int v, int a = 255);

   void getHsvF(qreal *h, qreal *s, qreal *v, qreal *a = nullptr) const;
   void setHsvF(qreal h, qreal s, qreal v, qreal a = 1.0);

   int cyan() const;
   int magenta() const;
   int yellow() const;
   int black() const;

   qreal cyanF() const;
   qreal magentaF() const;
   qreal yellowF() const;
   qreal blackF() const;

   void getCmyk(int *c, int *m, int *y, int *k, int *a = nullptr);
   void setCmyk(int c, int m, int y, int k, int a = 255);

   void getCmykF(qreal *c, qreal *m, qreal *y, qreal *k, qreal *a = nullptr);
   void setCmykF(qreal c, qreal m, qreal y, qreal k, qreal a = 1.0);

   int hslHue() const; // 0 <= hue < 360
   int hslSaturation() const;
   int lightness() const;

   qreal hslHueF() const; // 0.0 <= hueF < 360.0
   qreal hslSaturationF() const;
   qreal lightnessF() const;

   void getHsl(int *h, int *s, int *l, int *a = nullptr) const;
   void setHsl(int h, int s, int l, int a = 255);

   void getHslF(qreal *h, qreal *s, qreal *l, qreal *a = nullptr) const;
   void setHslF(qreal h, qreal s, qreal l, qreal a = 1.0);

   QColor toRgb() const;
   QColor toHsv() const;
   QColor toCmyk() const;
   QColor toHsl() const;

   QColor convertTo(Spec colorSpec) const;

   static QColor fromRgb(QRgb rgb);
   static QColor fromRgba(QRgb rgba);

   static QColor fromRgb(int r, int g, int b, int a = 255);
   static QColor fromRgbF(qreal r, qreal g, qreal b, qreal a = 1.0);

   static QColor fromRgba64(ushort r, ushort g, ushort b, ushort a = USHRT_MAX);
   static QColor fromRgba64(QRgba64 rgba);
   static QColor fromHsv(int h, int s, int v, int a = 255);
   static QColor fromHsvF(qreal h, qreal s, qreal v, qreal a = 1.0);

   static QColor fromCmyk(int c, int m, int y, int k, int a = 255);
   static QColor fromCmykF(qreal c, qreal m, qreal y, qreal k, qreal a = 1.0);

   static QColor fromHsl(int h, int s, int l, int a = 255);
   static QColor fromHslF(qreal h, qreal s, qreal l, qreal a = 1.0);

   QColor light(int factor = 150) const;
   QColor lighter(int factor = 150) const;
   QColor dark(int factor = 200) const;
   QColor darker(int factor = 200) const;

   bool operator==(const QColor &color) const;
   bool operator!=(const QColor &color) const;

   operator QVariant() const;
   static bool isValidColor(const QString &name);

 private:
   void invalidate();
   bool setColorFromString(const QString &name);

   Spec cspec;

#ifndef CS_DOXYPRESS

   union {
      struct {
         ushort alpha;
         ushort red;
         ushort green;
         ushort blue;
         ushort pad;
      } argb;

      struct {
         ushort alpha;
         ushort hue;
         ushort saturation;
         ushort value;
         ushort pad;
      } ahsv;

      struct {
         ushort alpha;
         ushort cyan;
         ushort magenta;
         ushort yellow;
         ushort black;
      } acmyk;

      struct {
         ushort alpha;
         ushort hue;
         ushort saturation;
         ushort lightness;
         ushort pad;
      } ahsl;

      ushort array[5];
   } ct;
#endif

   friend class QColormap;
   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QColor &color);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QColor &color);

};

inline QColor::QColor()
{
   invalidate();
}

inline QColor::QColor(int r, int g, int b, int a)
{
   setRgb(r, g, b, a);
}

inline QColor::QColor(const char *name)
{
   setNamedColor(QString::fromLatin1(name));
}

inline QColor::QColor(const QString &name)
{
   setNamedColor(name);
}

inline bool QColor::isValid() const
{
   return cspec != Invalid;
}

inline QColor QColor::lighter(int factor) const
{
   return light(factor);
}

inline QColor QColor::darker(int factor) const
{
   return dark(factor);
}

#endif
