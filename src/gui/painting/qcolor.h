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

#ifndef QCOLOR_H
#define QCOLOR_H

#include <qrgb.h>
#include <qnamespace.h>
#include <qstringlist.h>

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

   QColor();
   QColor(Qt::GlobalColor color);
   QColor(int r, int g, int b, int a = 255);
   QColor(QRgb rgb);
   QColor(const QString &name);
   QColor(const char *name);
   QColor(const QColor &color);
   QColor(Spec spec);

   inline bool isValid() const;

   QString name() const;
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

   void getRgb(int *r, int *g, int *b, int *a = 0) const;
   void setRgb(int r, int g, int b, int a = 255);

   void getRgbF(qreal *r, qreal *g, qreal *b, qreal *a = 0) const;
   void setRgbF(qreal r, qreal g, qreal b, qreal a = 1.0);

   QRgb rgba() const;
   void setRgba(QRgb rgba);

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

   void getHsv(int *h, int *s, int *v, int *a = 0) const;
   void setHsv(int h, int s, int v, int a = 255);

   void getHsvF(qreal *h, qreal *s, qreal *v, qreal *a = 0) const;
   void setHsvF(qreal h, qreal s, qreal v, qreal a = 1.0);

   int cyan() const;
   int magenta() const;
   int yellow() const;
   int black() const;

   qreal cyanF() const;
   qreal magentaF() const;
   qreal yellowF() const;
   qreal blackF() const;

   void getCmyk(int *c, int *m, int *y, int *k, int *a = 0);
   void setCmyk(int c, int m, int y, int k, int a = 255);

   void getCmykF(qreal *c, qreal *m, qreal *y, qreal *k, qreal *a = 0);
   void setCmykF(qreal c, qreal m, qreal y, qreal k, qreal a = 1.0);

   int hslHue() const; // 0 <= hue < 360
   int hslSaturation() const;
   int lightness() const;

   qreal hslHueF() const; // 0.0 <= hueF < 360.0
   qreal hslSaturationF() const;
   qreal lightnessF() const;

   void getHsl(int *h, int *s, int *l, int *a = 0) const;
   void setHsl(int h, int s, int l, int a = 255);

   void getHslF(qreal *h, qreal *s, qreal *l, qreal *a = 0) const;
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

   static QColor fromHsv(int h, int s, int v, int a = 255);
   static QColor fromHsvF(qreal h, qreal s, qreal v, qreal a = 1.0);

   static QColor fromCmyk(int c, int m, int y, int k, int a = 255);
   static QColor fromCmykF(qreal c, qreal m, qreal y, qreal k, qreal a = 1.0);

   static QColor fromHsl(int h, int s, int l, int a = 255);
   static QColor fromHslF(qreal h, qreal s, qreal l, qreal a = 1.0);

   QColor light(int f = 150) const;
   inline QColor lighter(int f = 150) const;
   QColor dark(int f = 200) const;
   inline QColor darker(int f = 200) const;

   QColor &operator=(const QColor &);
   QColor &operator=(Qt::GlobalColor color);

   bool operator==(const QColor &c) const;
   bool operator!=(const QColor &c) const;

   operator QVariant() const;

#ifdef Q_WS_X11
   static bool allowX11ColorNames();
   static void setAllowX11ColorNames(bool enabled);
#endif



   static bool isValidColor(const QString &name);

 private:

   void invalidate();
   bool setColorFromString(const QString &name);

   Spec cspec;
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

   friend class QColormap;
   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QColor &);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QColor &);

};

inline QColor::QColor()
{
   invalidate();
}

inline QColor::QColor(int r, int g, int b, int a)
{
   setRgb(r, g, b, a);
}

inline QColor::QColor(const char *aname)
{
   setNamedColor(QString::fromLatin1(aname));
}

inline QColor::QColor(const QString &aname)
{
   setNamedColor(aname);
}

inline QColor::QColor(const QColor &acolor)
   : cspec(acolor.cspec)
{
   ct.argb = acolor.ct.argb;
}

inline bool QColor::isValid() const
{
   return cspec != Invalid;
}

inline QColor QColor::lighter(int f) const
{
   return light(f);
}

inline QColor QColor::darker(int f) const
{
   return dark(f);
}

#endif // QCOLOR_H
