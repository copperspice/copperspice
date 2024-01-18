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

#ifndef QDECLARATIVEVALUETYPE_P_H
#define QDECLARATIVEVALUETYPE_P_H

#include "qdeclarativeproperty.h"
#include "qdeclarativeproperty_p.h"
#include "qdeclarativenullablevalue_p_p.h"
#include <QtCore/qobject.h>
#include <QtCore/qrect.h>
#include <QtCore/qeasingcurve.h>
#include <QtCore/qvariant.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qmatrix4x4.h>
#include <QtGui/qquaternion.h>
#include <QtGui/qfont.h>

QT_BEGIN_NAMESPACE

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeValueType : public QObject
{
   DECL_CS_OBJECT(QDeclarativeValueType)
 public:
   QDeclarativeValueType(QObject *parent = nullptr);
   virtual void read(QObject *, int) = 0;
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags flags) = 0;
   virtual QVariant value() = 0;
   virtual void setValue(QVariant) = 0;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeValueTypeFactory
{
 public:
   QDeclarativeValueTypeFactory();
   ~QDeclarativeValueTypeFactory();
   static bool isValueType(int);
   static QDeclarativeValueType *valueType(int);

   static void registerValueTypes();
   static void registerValueTypesCompat();

   QDeclarativeValueType *operator[](int idx) const {
      if (idx < 0 || idx >= (int)QVariant::UserType) {
         return 0;
      } else {
         return valueTypes[idx];
      }
   }

 private:
   QDeclarativeValueType *valueTypes[QVariant::UserType - 1];
};

class QDeclarativePointFValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_OBJECT(QDeclarativePointFValueType)
 public:
   QDeclarativePointFValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   qreal x() const;
   qreal y() const;
   void setX(qreal);
   void setY(qreal);

 private:
   QPointF point;
};

class QDeclarativePointValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_OBJECT(QDeclarativePointValueType)
 public:
   QDeclarativePointValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   int x() const;
   int y() const;
   void setX(int);
   void setY(int);

 private:
   QPoint point;
};

class QDeclarativeSizeFValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(width, width)
   DECL_CS_PROPERTY_WRITE(width, setWidth)
   DECL_CS_PROPERTY_READ(height, height)
   DECL_CS_PROPERTY_WRITE(height, setHeight)
   DECL_CS_OBJECT(QDeclarativeSizeFValueType)
 public:
   QDeclarativeSizeFValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   qreal width() const;
   qreal height() const;
   void setWidth(qreal);
   void setHeight(qreal);

 private:
   QSizeF size;
};

class QDeclarativeSizeValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(width, width)
   DECL_CS_PROPERTY_WRITE(width, setWidth)
   DECL_CS_PROPERTY_READ(height, height)
   DECL_CS_PROPERTY_WRITE(height, setHeight)
   DECL_CS_OBJECT(QDeclarativeSizeValueType)
 public:
   QDeclarativeSizeValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   int width() const;
   int height() const;
   void setWidth(int);
   void setHeight(int);

 private:
   QSize size;
};

class QDeclarativeRectFValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_PROPERTY_READ(width, width)
   DECL_CS_PROPERTY_WRITE(width, setWidth)
   DECL_CS_PROPERTY_READ(height, height)
   DECL_CS_PROPERTY_WRITE(height, setHeight)
   DECL_CS_OBJECT(QDeclarativeRectFValueType)
 public:
   QDeclarativeRectFValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   qreal x() const;
   qreal y() const;
   void setX(qreal);
   void setY(qreal);

   qreal width() const;
   qreal height() const;
   void setWidth(qreal);
   void setHeight(qreal);

 private:
   QRectF rect;
};

class QDeclarativeRectValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_PROPERTY_READ(width, width)
   DECL_CS_PROPERTY_WRITE(width, setWidth)
   DECL_CS_PROPERTY_READ(height, height)
   DECL_CS_PROPERTY_WRITE(height, setHeight)
   DECL_CS_OBJECT(QDeclarativeRectValueType)
 public:
   QDeclarativeRectValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   int x() const;
   int y() const;
   void setX(int);
   void setY(int);

   int width() const;
   int height() const;
   void setWidth(int);
   void setHeight(int);

 private:
   QRect rect;
};

class QDeclarativeVector2DValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_OBJECT(QDeclarativeVector2DValueType)
 public:
   QDeclarativeVector2DValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   qreal x() const;
   qreal y() const;
   void setX(qreal);
   void setY(qreal);

 private:
   QVector2D vector;
};

class QDeclarativeVector3DValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_PROPERTY_READ(z, z)
   DECL_CS_PROPERTY_WRITE(z, setZ)
   DECL_CS_OBJECT(QDeclarativeVector3DValueType)
 public:
   QDeclarativeVector3DValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   qreal x() const;
   qreal y() const;
   qreal z() const;
   void setX(qreal);
   void setY(qreal);
   void setZ(qreal);

 private:
   QVector3D vector;
};

class QDeclarativeVector4DValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_PROPERTY_READ(z, z)
   DECL_CS_PROPERTY_WRITE(z, setZ)
   DECL_CS_PROPERTY_READ(w, w)
   DECL_CS_PROPERTY_WRITE(w, setW)
   DECL_CS_OBJECT(QDeclarativeVector4DValueType)
 public:
   QDeclarativeVector4DValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   qreal x() const;
   qreal y() const;
   qreal z() const;
   qreal w() const;
   void setX(qreal);
   void setY(qreal);
   void setZ(qreal);
   void setW(qreal);

 private:
   QVector4D vector;
};

class QDeclarativeQuaternionValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(scalar, scalar)
   DECL_CS_PROPERTY_WRITE(scalar, setScalar)
   DECL_CS_PROPERTY_READ(x, x)
   DECL_CS_PROPERTY_WRITE(x, setX)
   DECL_CS_PROPERTY_READ(y, y)
   DECL_CS_PROPERTY_WRITE(y, setY)
   DECL_CS_PROPERTY_READ(z, z)
   DECL_CS_PROPERTY_WRITE(z, setZ)
   DECL_CS_OBJECT(QDeclarativeQuaternionValueType)
 public:
   QDeclarativeQuaternionValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   qreal scalar() const;
   qreal x() const;
   qreal y() const;
   qreal z() const;
   void setScalar(qreal);
   void setX(qreal);
   void setY(qreal);
   void setZ(qreal);

 private:
   QQuaternion quaternion;
};

class QDeclarativeMatrix4x4ValueType : public QDeclarativeValueType
{
   DECL_CS_PROPERTY_READ(m11, m11)
   DECL_CS_PROPERTY_WRITE(m11, setM11)
   DECL_CS_PROPERTY_READ(m12, m12)
   DECL_CS_PROPERTY_WRITE(m12, setM12)
   DECL_CS_PROPERTY_READ(m13, m13)
   DECL_CS_PROPERTY_WRITE(m13, setM13)
   DECL_CS_PROPERTY_READ(m14, m14)
   DECL_CS_PROPERTY_WRITE(m14, setM14)
   DECL_CS_PROPERTY_READ(m21, m21)
   DECL_CS_PROPERTY_WRITE(m21, setM21)
   DECL_CS_PROPERTY_READ(m22, m22)
   DECL_CS_PROPERTY_WRITE(m22, setM22)
   DECL_CS_PROPERTY_READ(m23, m23)
   DECL_CS_PROPERTY_WRITE(m23, setM23)
   DECL_CS_PROPERTY_READ(m24, m24)
   DECL_CS_PROPERTY_WRITE(m24, setM24)
   DECL_CS_PROPERTY_READ(m31, m31)
   DECL_CS_PROPERTY_WRITE(m31, setM31)
   DECL_CS_PROPERTY_READ(m32, m32)
   DECL_CS_PROPERTY_WRITE(m32, setM32)
   DECL_CS_PROPERTY_READ(m33, m33)
   DECL_CS_PROPERTY_WRITE(m33, setM33)
   DECL_CS_PROPERTY_READ(m34, m34)
   DECL_CS_PROPERTY_WRITE(m34, setM34)
   DECL_CS_PROPERTY_READ(m41, m41)
   DECL_CS_PROPERTY_WRITE(m41, setM41)
   DECL_CS_PROPERTY_READ(m42, m42)
   DECL_CS_PROPERTY_WRITE(m42, setM42)
   DECL_CS_PROPERTY_READ(m43, m43)
   DECL_CS_PROPERTY_WRITE(m43, setM43)
   DECL_CS_PROPERTY_READ(m44, m44)
   DECL_CS_PROPERTY_WRITE(m44, setM44)
   DECL_CS_OBJECT(QDeclarativeMatrix4x4ValueType)
 public:
   QDeclarativeMatrix4x4ValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   qreal m11() const {
      return matrix(0, 0);
   }
   qreal m12() const {
      return matrix(0, 1);
   }
   qreal m13() const {
      return matrix(0, 2);
   }
   qreal m14() const {
      return matrix(0, 3);
   }
   qreal m21() const {
      return matrix(1, 0);
   }
   qreal m22() const {
      return matrix(1, 1);
   }
   qreal m23() const {
      return matrix(1, 2);
   }
   qreal m24() const {
      return matrix(1, 3);
   }
   qreal m31() const {
      return matrix(2, 0);
   }
   qreal m32() const {
      return matrix(2, 1);
   }
   qreal m33() const {
      return matrix(2, 2);
   }
   qreal m34() const {
      return matrix(2, 3);
   }
   qreal m41() const {
      return matrix(3, 0);
   }
   qreal m42() const {
      return matrix(3, 1);
   }
   qreal m43() const {
      return matrix(3, 2);
   }
   qreal m44() const {
      return matrix(3, 3);
   }

   void setM11(qreal value) {
      matrix(0, 0) = value;
   }
   void setM12(qreal value) {
      matrix(0, 1) = value;
   }
   void setM13(qreal value) {
      matrix(0, 2) = value;
   }
   void setM14(qreal value) {
      matrix(0, 3) = value;
   }
   void setM21(qreal value) {
      matrix(1, 0) = value;
   }
   void setM22(qreal value) {
      matrix(1, 1) = value;
   }
   void setM23(qreal value) {
      matrix(1, 2) = value;
   }
   void setM24(qreal value) {
      matrix(1, 3) = value;
   }
   void setM31(qreal value) {
      matrix(2, 0) = value;
   }
   void setM32(qreal value) {
      matrix(2, 1) = value;
   }
   void setM33(qreal value) {
      matrix(2, 2) = value;
   }
   void setM34(qreal value) {
      matrix(2, 3) = value;
   }
   void setM41(qreal value) {
      matrix(3, 0) = value;
   }
   void setM42(qreal value) {
      matrix(3, 1) = value;
   }
   void setM43(qreal value) {
      matrix(3, 2) = value;
   }
   void setM44(qreal value) {
      matrix(3, 3) = value;
   }

 private:
   QMatrix4x4 matrix;
};

class QDeclarativeEasingValueType : public QDeclarativeValueType
{
   DECL_CS_OBJECT(QDeclarativeEasingValueType)
   CS_ENUM(Type)

   DECL_CS_PROPERTY_READ(type, type)
   DECL_CS_PROPERTY_WRITE(type, setType)
   DECL_CS_PROPERTY_READ(amplitude, amplitude)
   DECL_CS_PROPERTY_WRITE(amplitude, setAmplitude)
   DECL_CS_PROPERTY_READ(overshoot, overshoot)
   DECL_CS_PROPERTY_WRITE(overshoot, setOvershoot)
   DECL_CS_PROPERTY_READ(period, period)
   DECL_CS_PROPERTY_WRITE(period, setPeriod)
 public:
   enum Type {
      Linear = QEasingCurve::Linear,
      InQuad = QEasingCurve::InQuad, OutQuad = QEasingCurve::OutQuad,
      InOutQuad = QEasingCurve::InOutQuad, OutInQuad = QEasingCurve::OutInQuad,
      InCubic = QEasingCurve::InCubic, OutCubic = QEasingCurve::OutCubic,
      InOutCubic = QEasingCurve::InOutCubic, OutInCubic = QEasingCurve::OutInCubic,
      InQuart = QEasingCurve::InQuart, OutQuart = QEasingCurve::OutQuart,
      InOutQuart = QEasingCurve::InOutQuart, OutInQuart = QEasingCurve::OutInQuart,
      InQuint = QEasingCurve::InQuint, OutQuint = QEasingCurve::OutQuint,
      InOutQuint = QEasingCurve::InOutQuint, OutInQuint = QEasingCurve::OutInQuint,
      InSine = QEasingCurve::InSine, OutSine = QEasingCurve::OutSine,
      InOutSine = QEasingCurve::InOutSine, OutInSine = QEasingCurve::OutInSine,
      InExpo = QEasingCurve::InExpo, OutExpo = QEasingCurve::OutExpo,
      InOutExpo = QEasingCurve::InOutExpo, OutInExpo = QEasingCurve::OutInExpo,
      InCirc = QEasingCurve::InCirc, OutCirc = QEasingCurve::OutCirc,
      InOutCirc = QEasingCurve::InOutCirc, OutInCirc = QEasingCurve::OutInCirc,
      InElastic = QEasingCurve::InElastic, OutElastic = QEasingCurve::OutElastic,
      InOutElastic = QEasingCurve::InOutElastic, OutInElastic = QEasingCurve::OutInElastic,
      InBack = QEasingCurve::InBack, OutBack = QEasingCurve::OutBack,
      InOutBack = QEasingCurve::InOutBack, OutInBack = QEasingCurve::OutInBack,
      InBounce = QEasingCurve::InBounce, OutBounce = QEasingCurve::OutBounce,
      InOutBounce = QEasingCurve::InOutBounce, OutInBounce = QEasingCurve::OutInBounce,
      InCurve = QEasingCurve::InCurve, OutCurve = QEasingCurve::OutCurve,
      SineCurve = QEasingCurve::SineCurve, CosineCurve = QEasingCurve::CosineCurve
   };

   QDeclarativeEasingValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   Type type() const;
   qreal amplitude() const;
   qreal overshoot() const;
   qreal period() const;
   void setType(Type);
   void setAmplitude(qreal);
   void setOvershoot(qreal);
   void setPeriod(qreal);

 private:
   QEasingCurve easing;
};

class QDeclarativeFontValueType : public QDeclarativeValueType
{
   DECL_CS_OBJECT(QDeclarativeFontValueType)
   CS_ENUM(FontWeight)
   CS_ENUM(Capitalization)

   DECL_CS_PROPERTY_READ(family, family)
   DECL_CS_PROPERTY_WRITE(family, setFamily)
   DECL_CS_PROPERTY_READ(bold, bold)
   DECL_CS_PROPERTY_WRITE(bold, setBold)
   DECL_CS_PROPERTY_READ(weight, weight)
   DECL_CS_PROPERTY_WRITE(weight, setWeight)
   DECL_CS_PROPERTY_READ(italic, italic)
   DECL_CS_PROPERTY_WRITE(italic, setItalic)
   DECL_CS_PROPERTY_READ(underline, underline)
   DECL_CS_PROPERTY_WRITE(underline, setUnderline)
   DECL_CS_PROPERTY_READ(overline, overline)
   DECL_CS_PROPERTY_WRITE(overline, setOverline)
   DECL_CS_PROPERTY_READ(strikeout, strikeout)
   DECL_CS_PROPERTY_WRITE(strikeout, setStrikeout)
   DECL_CS_PROPERTY_READ(pointSize, pointSize)
   DECL_CS_PROPERTY_WRITE(pointSize, setPointSize)
   DECL_CS_PROPERTY_READ(pixelSize, pixelSize)
   DECL_CS_PROPERTY_WRITE(pixelSize, setPixelSize)
   DECL_CS_PROPERTY_READ(capitalization, capitalization)
   DECL_CS_PROPERTY_WRITE(capitalization, setCapitalization)
   DECL_CS_PROPERTY_READ(letterSpacing, letterSpacing)
   DECL_CS_PROPERTY_WRITE(letterSpacing, setLetterSpacing)
   DECL_CS_PROPERTY_READ(wordSpacing, wordSpacing)
   DECL_CS_PROPERTY_WRITE(wordSpacing, setWordSpacing)

 public:
   enum FontWeight { Light = QFont::Light,
                     Normal = QFont::Normal,
                     DemiBold = QFont::DemiBold,
                     Bold = QFont::Bold,
                     Black = QFont::Black
                   };
   enum Capitalization { MixedCase = QFont::MixedCase,
                         AllUppercase = QFont::AllUppercase,
                         AllLowercase = QFont::AllLowercase,
                         SmallCaps = QFont::SmallCaps,
                         Capitalize = QFont::Capitalize
                       };

   QDeclarativeFontValueType(QObject *parent = nullptr);

   virtual void read(QObject *, int);
   virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
   virtual QVariant value();
   virtual void setValue(QVariant value);

   QString family() const;
   void setFamily(const QString &);

   bool bold() const;
   void setBold(bool b);

   FontWeight weight() const;
   void setWeight(FontWeight);

   bool italic() const;
   void setItalic(bool b);

   bool underline() const;
   void setUnderline(bool b);

   bool overline() const;
   void setOverline(bool b);

   bool strikeout() const;
   void setStrikeout(bool b);

   qreal pointSize() const;
   void setPointSize(qreal size);

   int pixelSize() const;
   void setPixelSize(int size);

   Capitalization capitalization() const;
   void setCapitalization(Capitalization);

   qreal letterSpacing() const;
   void setLetterSpacing(qreal spacing);

   qreal wordSpacing() const;
   void setWordSpacing(qreal spacing);

 private:
   QFont font;
   bool pixelSizeSet;
   bool pointSizeSet;
   mutable QDeclarativeNullableValue<int> dpi;
};

QT_END_NAMESPACE

#endif  // QDECLARATIVEVALUETYPE_P_H
