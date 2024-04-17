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

#ifndef QRGBA64_H
#define QRGBA64_H

#include <qglobal.h>

class QRgba64
{
   quint64 rgba;

   // Make sure that the representation always has the order: red green blue alpha, independent
   // of byte order. This way, vector operations that assume 4 16-bit values see the correct ones.
   enum {

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
      RedShift = 48,
      GreenShift = 32,
      BlueShift = 16,
      AlphaShift = 0

#else // little endian:
      RedShift = 0,
      GreenShift = 16,
      BlueShift = 32,
      AlphaShift = 48
#endif

   };

   explicit constexpr QRgba64(quint64 c)
      : rgba(c)
   { }

 public:
   constexpr QRgba64()
      : rgba(0)
   { }

   static constexpr QRgba64 fromRgba64(quint64 c) {
      return QRgba64(c);
   }

   static constexpr QRgba64 fromRgba64(quint16 red, quint16 green, quint16 blue, quint16 alpha) {
      return fromRgba64(quint64(red)   << RedShift
            | quint64(green) << GreenShift
            | quint64(blue)  << BlueShift
            | quint64(alpha) << AlphaShift);
   }

   static constexpr QRgba64 fromRgba(quint8 red, quint8 green, quint8 blue, quint8 alpha) {
      QRgba64 rgb64 = fromRgba64(red, green, blue, alpha);
      // Expand the range so that 0x00 maps to 0x0000 and 0xff maps to 0xffff.
      rgb64.rgba |= rgb64.rgba << 8;
      return rgb64;
   }

   static constexpr QRgba64 fromArgb32(uint rgb) {
      return fromRgba(quint8(rgb >> 16), quint8(rgb >> 8), quint8(rgb), quint8(rgb >> 24));
   }

   constexpr bool isOpaque() const {
      return (rgba & alphaMask()) == alphaMask();
   }

   constexpr bool isTransparent() const {
      return (rgba & alphaMask()) == 0;
   }

   constexpr quint16 red()   const {
      return quint16(rgba >> RedShift);
   }

   constexpr quint16 green() const {
      return quint16(rgba >> GreenShift);
   }

   constexpr quint16 blue()  const {
      return quint16(rgba >> BlueShift);
   }

   constexpr quint16 alpha() const {
      return quint16(rgba >> AlphaShift);
   }

   void setRed(quint16 red)     {
      rgba = (rgba & ~(Q_UINT64_C(0xffff) << RedShift))   | (quint64(red) << RedShift);
   }

   void setGreen(quint16 green) {
      rgba = (rgba & ~(Q_UINT64_C(0xffff) << GreenShift)) | (quint64(green) << GreenShift);
   }

   void setBlue(quint16 blue)   {
      rgba = (rgba & ~(Q_UINT64_C(0xffff) << BlueShift))  | (quint64(blue) << BlueShift);
   }

   void setAlpha(quint16 alpha) {
      rgba = (rgba & ~(Q_UINT64_C(0xffff) << AlphaShift)) | (quint64(alpha) << AlphaShift);
   }

   constexpr quint8 red8()   const {
      return div_257(red());
   }

   constexpr quint8 green8() const {
      return div_257(green());
   }

   constexpr quint8 blue8()  const {
      return div_257(blue());
   }

   constexpr quint8 alpha8() const {
      return div_257(alpha());
   }

   constexpr uint toArgb32() const {
      return uint((alpha8() << 24) | (red8() << 16) | (green8() << 8) | blue8());
   }

   constexpr ushort toRgb16() const {
      return ushort((red() & 0xf800) | ((green() >> 10) << 5) | (blue() >> 11));
   }

   constexpr QRgba64 premultiplied() const {
      const quint32 a = alpha();
      const quint16 r = div_65535(red()   * a);
      const quint16 g = div_65535(green() * a);
      const quint16 b = div_65535(blue()  * a);

      return fromRgba64(r, g, b, quint16(a));
   }

   constexpr QRgba64 unpremultiplied() const {
#if Q_PROCESSOR_WORDSIZE < 8
      return unpremultiplied_32bit();
#else
      return unpremultiplied_64bit();
#endif
   }

   constexpr operator quint64() const {
      return rgba;
   }

   QRgba64 operator=(quint64 rgba64) {
      rgba = rgba64;
      return *this;
   }

 private:
   static constexpr quint64 alphaMask() {
      return Q_UINT64_C(0xffff) << AlphaShift;
   }

   static constexpr quint8 div_257_floor(uint x) {
      return quint8((x - (x >> 8)) >> 8);
   }

   static constexpr quint8 div_257(quint16 x) {
      return div_257_floor(x + 128U);
   }

   static constexpr quint16 div_65535(uint x) {
      return quint16((x + (x >> 16) + 0x8000U) >> 16);
   }

   constexpr inline QRgba64 unpremultiplied_32bit() const {
      if (isOpaque() || isTransparent()) {
         return *this;
      }
      const quint32 a = alpha();
      const quint16 r = quint16((red()   * 0xffff + a / 2) / a);
      const quint16 g = quint16((green() * 0xffff + a / 2) / a);
      const quint16 b = quint16((blue()  * 0xffff + a / 2) / a);

      return fromRgba64(r, g, b, quint16(a));
   }

   constexpr QRgba64 unpremultiplied_64bit() const {
      if (isOpaque() || isTransparent()) {
         return *this;
      }

      const quint64 a = alpha();
      const quint64 fa = (Q_UINT64_C(0xffff00008000) + a / 2) / a;
      const quint16 r = quint16((red()   * fa + 0x80000000) >> 32);
      const quint16 g = quint16((green() * fa + 0x80000000) >> 32);
      const quint16 b = quint16((blue()  * fa + 0x80000000) >> 32);

      return fromRgba64(r, g, b, quint16(a));
   }
};

constexpr inline QRgba64 qRgba64(quint16 r, quint16 g, quint16 b, quint16 a)
{
   return QRgba64::fromRgba64(r, g, b, a);
}

constexpr inline QRgba64 qRgba64(quint64 c)
{
   return QRgba64::fromRgba64(c);
}

constexpr inline QRgba64 qPremultiply(QRgba64 c)
{
   return c.premultiplied();
}

constexpr inline QRgba64 qUnpremultiply(QRgba64 c)
{
   return c.unpremultiplied();
}

constexpr inline uint qRed(QRgba64 rgb)
{
   return rgb.red8();
}

constexpr inline uint qGreen(QRgba64 rgb)
{
   return rgb.green8();
}

constexpr inline uint qBlue(QRgba64 rgb)
{
   return rgb.blue8();
}

constexpr inline uint qAlpha(QRgba64 rgb)
{
   return rgb.alpha8();
}

#endif
