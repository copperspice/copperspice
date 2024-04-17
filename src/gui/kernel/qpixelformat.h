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

#ifndef QPIXELFORMAT_H
#define QPIXELFORMAT_H

#include <qglobal.h>

class QPixelFormat
{
   // QPixelFormat basically is a glorified quint64, split into several fields.
   // We could use bit-fields, but GCC at least generates horrible, horrible code for them,
   // so we do the bit-twiddling ourselves.

   enum FieldWidth {
      ModelFieldWidth = 4,
      FirstFieldWidth = 6,
      SecondFieldWidth = FirstFieldWidth,
      ThirdFieldWidth = FirstFieldWidth,
      FourthFieldWidth = FirstFieldWidth,
      FifthFieldWidth = FirstFieldWidth,
      AlphaFieldWidth = FirstFieldWidth,
      AlphaUsageFieldWidth = 1,
      AlphaPositionFieldWidth = 1,
      PremulFieldWidth = 1,
      TypeInterpretationFieldWidth = 4,
      ByteOrderFieldWidth = 2,
      SubEnumFieldWidth = 6,
      UnusedFieldWidth = 9,

      TotalFieldWidthByWidths = ModelFieldWidth + FirstFieldWidth + SecondFieldWidth + ThirdFieldWidth +
         FourthFieldWidth + FifthFieldWidth + AlphaFieldWidth + AlphaUsageFieldWidth +
         AlphaPositionFieldWidth + PremulFieldWidth + TypeInterpretationFieldWidth +
         ByteOrderFieldWidth + SubEnumFieldWidth + UnusedFieldWidth
   };

   enum Field {
      ModelField = 0,
      // work around bug in old clang versions: when building webkit
      // with XCode 4.6 and older this fails compilation, thus cast to int
      FirstField = ModelField + int(ModelFieldWidth),
      SecondField = FirstField + FirstFieldWidth,
      ThirdField = SecondField + SecondFieldWidth,
      FourthField = ThirdField + ThirdFieldWidth,
      FifthField = FourthField + FourthFieldWidth,
      AlphaField = FifthField + FifthFieldWidth,
      AlphaUsageField = AlphaField + AlphaFieldWidth,
      AlphaPositionField = AlphaUsageField + AlphaUsageFieldWidth,
      PremulField = AlphaPositionField + AlphaPositionFieldWidth,
      TypeInterpretationField = PremulField + PremulFieldWidth,
      ByteOrderField = TypeInterpretationField + TypeInterpretationFieldWidth,
      SubEnumField = ByteOrderField + ByteOrderFieldWidth,
      UnusedField = SubEnumField + SubEnumFieldWidth,

      TotalFieldWidthByOffsets = UnusedField + UnusedFieldWidth
   };

   static_assert(uint(TotalFieldWidthByWidths) == uint(TotalFieldWidthByOffsets), "Type mismatch");
   static_assert(uint(TotalFieldWidthByWidths) == 8 * sizeof(quint64), "Type mismatch");

   constexpr uchar get(Field offset, FieldWidth width) const {
      return uchar((data >> uint(offset)) & ((static_cast<quint64>(1) << uint(width)) - static_cast<quint64>(1)));
   }

   static constexpr quint64 set(Field offset, FieldWidth width, uchar value) {
      return (quint64(value) & ((static_cast<quint64>(1) << uint(width)) - static_cast<quint64>(1))) << uint(offset);
   }

 public:
   enum ColorModel {
      RGB,
      BGR,
      Indexed,
      Grayscale,
      CMYK,
      HSL,
      HSV,
      YUV,
      Alpha
   };

   enum AlphaUsage {
      UsesAlpha,
      IgnoresAlpha
   };

   enum AlphaPosition {
      AtBeginning,
      AtEnd
   };

   enum AlphaPremultiplied {
      NotPremultiplied,
      Premultiplied
   };

   enum TypeInterpretation {
      UnsignedInteger,
      UnsignedShort,
      UnsignedByte,
      FloatingPoint
   };

   enum YUVLayout {
      YUV444,
      YUV422,
      YUV411,
      YUV420P,
      YUV420SP,
      YV12,
      UYVY,
      YUYV,
      NV12,
      NV21,
      IMC1,
      IMC2,
      IMC3,
      IMC4,
      Y8,
      Y16
   };

   enum ByteOrder {
      LittleEndian,
      BigEndian,
      CurrentSystemEndian
   };

   constexpr QPixelFormat()
      : data(0)
   { }

   constexpr inline QPixelFormat(ColorModel colorModel, uchar firstSize, uchar secondSize,
         uchar thirdSize, uchar fourthSize, uchar fifthSize, uchar alphaSize, AlphaUsage alphaUsage,
         AlphaPosition alphaPosition, AlphaPremultiplied premultiplied, TypeInterpretation typeInterpretation,
         ByteOrder byteOrder = CurrentSystemEndian, uchar subEnum = 0);

   constexpr ColorModel colorModel() const   {
      return ColorModel(get(ModelField, ModelFieldWidth));
   }

   constexpr uchar channelCount() const  {
      return (get(FirstField, FirstFieldWidth) > 0) +
         (get(SecondField, SecondFieldWidth) > 0) +
         (get(ThirdField, ThirdFieldWidth) > 0) +
         (get(FourthField, FourthFieldWidth) > 0) +
         (get(FifthField, FifthFieldWidth) > 0) +
         (get(AlphaField, AlphaFieldWidth) > 0);
   }

   constexpr inline uchar redSize() const {
      return get(FirstField, FirstFieldWidth);
   }

   constexpr inline uchar greenSize() const  {
      return get(SecondField, SecondFieldWidth);
   }
   constexpr inline uchar blueSize() const  {
      return get(ThirdField, ThirdFieldWidth);
   }

   constexpr inline uchar cyanSize() const  {
      return get(FirstField, FirstFieldWidth);
   }

   constexpr inline uchar magentaSize() const  {
      return get(SecondField, SecondFieldWidth);
   }

   constexpr inline uchar yellowSize() const  {
      return get(ThirdField, ThirdFieldWidth);
   }

   constexpr inline uchar blackSize() const  {
      return get(FourthField, FourthFieldWidth);
   }

   constexpr inline uchar hueSize() const  {
      return get(FirstField, FirstFieldWidth);
   }

   constexpr inline uchar saturationSize() const  {
      return get(SecondField, SecondFieldWidth);
   }

   constexpr inline uchar lightnessSize() const  {
      return get(ThirdField, ThirdFieldWidth);
   }
   constexpr inline uchar brightnessSize() const  {
      return get(ThirdField, ThirdFieldWidth);
   }

   constexpr inline uchar alphaSize() const  {
      return get(AlphaField, AlphaFieldWidth);
   }

   constexpr inline uchar bitsPerPixel() const  {
      return get(FirstField, FirstFieldWidth) +
         get(SecondField, SecondFieldWidth) +
         get(ThirdField, ThirdFieldWidth) +
         get(FourthField, FourthFieldWidth) +
         get(FifthField, FifthFieldWidth) +
         get(AlphaField, AlphaFieldWidth);
   }

   constexpr inline AlphaUsage alphaUsage() const  {
      return AlphaUsage(get(AlphaUsageField, AlphaUsageFieldWidth));
   }

   constexpr inline AlphaPosition alphaPosition() const  {
      return AlphaPosition(get(AlphaPositionField, AlphaPositionFieldWidth));
   }

   constexpr inline AlphaPremultiplied premultiplied() const  {
      return AlphaPremultiplied(get(PremulField, PremulFieldWidth));
   }

   constexpr inline TypeInterpretation typeInterpretation() const  {
      return TypeInterpretation(get(TypeInterpretationField, TypeInterpretationFieldWidth));
   }

   constexpr inline ByteOrder byteOrder() const  {
      return ByteOrder(get(ByteOrderField, ByteOrderFieldWidth));
   }

   constexpr inline YUVLayout yuvLayout() const  {
      return YUVLayout(get(SubEnumField, SubEnumFieldWidth));
   }

   constexpr inline uchar subEnum() const  {
      return get(SubEnumField, SubEnumFieldWidth);
   }

 private:
   static constexpr inline ByteOrder resolveByteOrder(ByteOrder bo) {
      return bo == CurrentSystemEndian ? Q_BYTE_ORDER == Q_LITTLE_ENDIAN ? LittleEndian : BigEndian : bo ;
   }

 private:
   quint64 data;

   friend constexpr inline bool operator==(QPixelFormat fmt1, QPixelFormat fmt2) {
      return fmt1.data == fmt2.data;
   }

   friend constexpr inline bool operator!=(QPixelFormat fmt1, QPixelFormat fmt2) {
      return !(fmt1 == fmt2);
   }
};

static_assert(sizeof(QPixelFormat) == sizeof(quint64), "Type mismatch");

namespace QtPrivate {

QPixelFormat Q_GUI_EXPORT QPixelFormat_createYUV(QPixelFormat::YUVLayout yuvLayout,
   uchar alphaSize,
   QPixelFormat::AlphaUsage alphaUsage,
   QPixelFormat::AlphaPosition alphaPosition,
   QPixelFormat::AlphaPremultiplied premultiplied,
   QPixelFormat::TypeInterpretation typeInterpretation,
   QPixelFormat::ByteOrder byteOrder);
}

constexpr QPixelFormat::QPixelFormat(ColorModel mdl, uchar firstSize, uchar secondSize, uchar thirdSize, uchar fourthSize,
      uchar fifthSize, uchar alfa, AlphaUsage usage, AlphaPosition position, AlphaPremultiplied premult,
      TypeInterpretation typeInterp, ByteOrder b_order, uchar s_enum)
   : data(set(ModelField, ModelFieldWidth, uchar(mdl)) |
        set(FirstField, FirstFieldWidth, firstSize) |
        set(SecondField, SecondFieldWidth, secondSize) |
        set(ThirdField, ThirdFieldWidth, thirdSize) |
        set(FourthField, FourthFieldWidth, fourthSize) |
        set(FifthField, FifthFieldWidth, fifthSize) |
        set(AlphaField, AlphaFieldWidth, alfa) |
        set(AlphaUsageField, AlphaUsageFieldWidth, uchar(usage)) |
        set(AlphaPositionField, AlphaPositionFieldWidth, uchar(position)) |
        set(PremulField, PremulFieldWidth, uchar(premult)) |
        set(TypeInterpretationField, TypeInterpretationFieldWidth, uchar(typeInterp)) |
        set(ByteOrderField, ByteOrderFieldWidth, uchar(resolveByteOrder(b_order))) |
        set(SubEnumField, SubEnumFieldWidth, s_enum) |
        set(UnusedField, UnusedFieldWidth, 0))
{
}

constexpr inline QPixelFormat qPixelFormatRgba(uchar red, uchar green, uchar blue, uchar alfa,
      QPixelFormat::AlphaUsage usage, QPixelFormat::AlphaPosition position,
      QPixelFormat::AlphaPremultiplied pmul = QPixelFormat::NotPremultiplied,
      QPixelFormat::TypeInterpretation typeInt = QPixelFormat::UnsignedInteger)
{
   return QPixelFormat(QPixelFormat::RGB, red, green, blue, 0, 0, alfa, usage, position, pmul, typeInt);
}

constexpr inline QPixelFormat qPixelFormatGrayscale(uchar channelSize,
   QPixelFormat::TypeInterpretation typeInt = QPixelFormat::UnsignedInteger)
{
   return QPixelFormat(QPixelFormat::Grayscale,
         channelSize,
         0,
         0,
         0,
         0,
         0,
         QPixelFormat::IgnoresAlpha,
         QPixelFormat::AtBeginning,
         QPixelFormat::NotPremultiplied,
         typeInt);
}

constexpr inline QPixelFormat qPixelFormatAlpha(uchar channelSize,
   QPixelFormat::TypeInterpretation typeInt = QPixelFormat::UnsignedInteger)
{
   return QPixelFormat(QPixelFormat::Alpha,
         0,
         0,
         0,
         0,
         0,
         channelSize,
         QPixelFormat::UsesAlpha,
         QPixelFormat::AtBeginning,
         QPixelFormat::NotPremultiplied,
         typeInt);
}

constexpr inline QPixelFormat qPixelFormatCmyk(uchar channelSize,
   uchar alfa = 0,
   QPixelFormat::AlphaUsage usage = QPixelFormat::IgnoresAlpha,
   QPixelFormat::AlphaPosition position = QPixelFormat::AtBeginning,
   QPixelFormat::TypeInterpretation typeInt = QPixelFormat::UnsignedInteger)
{
   return QPixelFormat(QPixelFormat::CMYK,
         channelSize,
         channelSize,
         channelSize,
         channelSize,
         0,
         alfa,
         usage,
         position,
         QPixelFormat::NotPremultiplied,
         typeInt);
}

constexpr inline QPixelFormat qPixelFormatHsl(uchar channelSize,
   uchar alfa = 0,
   QPixelFormat::AlphaUsage usage = QPixelFormat::IgnoresAlpha,
   QPixelFormat::AlphaPosition position = QPixelFormat::AtBeginning,
   QPixelFormat::TypeInterpretation typeInt = QPixelFormat::FloatingPoint)
{
   return QPixelFormat(QPixelFormat::HSL, channelSize, channelSize, channelSize, 0, 0, alfa, usage,
      position, QPixelFormat::NotPremultiplied, typeInt);
}

constexpr inline QPixelFormat qPixelFormatHsv(uchar channelSize, uchar alfa = 0,
   QPixelFormat::AlphaUsage usage = QPixelFormat::IgnoresAlpha,
   QPixelFormat::AlphaPosition position = QPixelFormat::AtBeginning,
   QPixelFormat::TypeInterpretation typeInt = QPixelFormat::FloatingPoint)
{
   return QPixelFormat(QPixelFormat::HSV, channelSize, channelSize, channelSize, 0, 0, alfa,
         usage, position, QPixelFormat::NotPremultiplied, typeInt);
}

inline QPixelFormat qPixelFormatYuv(QPixelFormat::YUVLayout layout,
   uchar alfa = 0,
   QPixelFormat::AlphaUsage usage = QPixelFormat::IgnoresAlpha,
   QPixelFormat::AlphaPosition position = QPixelFormat::AtBeginning,
   QPixelFormat::AlphaPremultiplied p_mul = QPixelFormat::NotPremultiplied,
   QPixelFormat::TypeInterpretation typeInt = QPixelFormat::UnsignedByte,
   QPixelFormat::ByteOrder b_order = QPixelFormat::LittleEndian)
{
   return QtPrivate::QPixelFormat_createYUV(layout, alfa, usage, position, p_mul, typeInt, b_order);
}

#endif
