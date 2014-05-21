/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDECLARATIVEVALUETYPE_P_H
#define QDECLARATIVEVALUETYPE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qdeclarativeproperty.h"
#include "private/qdeclarativeproperty_p.h"
#include "private/qdeclarativenullablevalue_p_p.h"

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
    CS_OBJECT(QDeclarativeValueType)
public:
    QDeclarativeValueType(QObject *parent = 0);
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
        if (idx < 0 || idx >= (int)QVariant::UserType) return 0;
        else return valueTypes[idx];
    }

private:
    QDeclarativeValueType *valueTypes[QVariant::UserType - 1]; 
};

class QDeclarativePointFValueType : public QDeclarativeValueType
{
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_OBJECT(QDeclarativePointFValueType)
public:
    QDeclarativePointFValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_OBJECT(QDeclarativePointValueType)
public:
    QDeclarativePointValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(width, width)
    CS_PROPERTY_WRITE(width, setWidth)
    CS_PROPERTY_READ(height, height)
    CS_PROPERTY_WRITE(height, setHeight)
    CS_OBJECT(QDeclarativeSizeFValueType)
public:
    QDeclarativeSizeFValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(width, width)
    CS_PROPERTY_WRITE(width, setWidth)
    CS_PROPERTY_READ(height, height)
    CS_PROPERTY_WRITE(height, setHeight)
    CS_OBJECT(QDeclarativeSizeValueType)
public:
    QDeclarativeSizeValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_PROPERTY_READ(width, width)
    CS_PROPERTY_WRITE(width, setWidth)
    CS_PROPERTY_READ(height, height)
    CS_PROPERTY_WRITE(height, setHeight)
    CS_OBJECT(QDeclarativeRectFValueType)
public:
    QDeclarativeRectFValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_PROPERTY_READ(width, width)
    CS_PROPERTY_WRITE(width, setWidth)
    CS_PROPERTY_READ(height, height)
    CS_PROPERTY_WRITE(height, setHeight)
    CS_OBJECT(QDeclarativeRectValueType)
public:
    QDeclarativeRectValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_OBJECT(QDeclarativeVector2DValueType)
public:
    QDeclarativeVector2DValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_PROPERTY_READ(z, z)
    CS_PROPERTY_WRITE(z, setZ)
    CS_OBJECT(QDeclarativeVector3DValueType)
public:
    QDeclarativeVector3DValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_PROPERTY_READ(z, z)
    CS_PROPERTY_WRITE(z, setZ)
    CS_PROPERTY_READ(w, w)
    CS_PROPERTY_WRITE(w, setW)
    CS_OBJECT(QDeclarativeVector4DValueType)
public:
    QDeclarativeVector4DValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(scalar, scalar)
    CS_PROPERTY_WRITE(scalar, setScalar)
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_PROPERTY_READ(z, z)
    CS_PROPERTY_WRITE(z, setZ)
    CS_OBJECT(QDeclarativeQuaternionValueType)
public:
    QDeclarativeQuaternionValueType(QObject *parent = 0);

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
    CS_PROPERTY_READ(m11, m11)
    CS_PROPERTY_WRITE(m11, setM11)
    CS_PROPERTY_READ(m12, m12)
    CS_PROPERTY_WRITE(m12, setM12)
    CS_PROPERTY_READ(m13, m13)
    CS_PROPERTY_WRITE(m13, setM13)
    CS_PROPERTY_READ(m14, m14)
    CS_PROPERTY_WRITE(m14, setM14)
    CS_PROPERTY_READ(m21, m21)
    CS_PROPERTY_WRITE(m21, setM21)
    CS_PROPERTY_READ(m22, m22)
    CS_PROPERTY_WRITE(m22, setM22)
    CS_PROPERTY_READ(m23, m23)
    CS_PROPERTY_WRITE(m23, setM23)
    CS_PROPERTY_READ(m24, m24)
    CS_PROPERTY_WRITE(m24, setM24)
    CS_PROPERTY_READ(m31, m31)
    CS_PROPERTY_WRITE(m31, setM31)
    CS_PROPERTY_READ(m32, m32)
    CS_PROPERTY_WRITE(m32, setM32)
    CS_PROPERTY_READ(m33, m33)
    CS_PROPERTY_WRITE(m33, setM33)
    CS_PROPERTY_READ(m34, m34)
    CS_PROPERTY_WRITE(m34, setM34)
    CS_PROPERTY_READ(m41, m41)
    CS_PROPERTY_WRITE(m41, setM41)
    CS_PROPERTY_READ(m42, m42)
    CS_PROPERTY_WRITE(m42, setM42)
    CS_PROPERTY_READ(m43, m43)
    CS_PROPERTY_WRITE(m43, setM43)
    CS_PROPERTY_READ(m44, m44)
    CS_PROPERTY_WRITE(m44, setM44)
    CS_OBJECT(QDeclarativeMatrix4x4ValueType)
public:
    QDeclarativeMatrix4x4ValueType(QObject *parent = 0);

    virtual void read(QObject *, int);
    virtual void write(QObject *, int, QDeclarativePropertyPrivate::WriteFlags);
    virtual QVariant value();
    virtual void setValue(QVariant value);

    qreal m11() const { return matrix(0, 0); }
    qreal m12() const { return matrix(0, 1); }
    qreal m13() const { return matrix(0, 2); }
    qreal m14() const { return matrix(0, 3); }
    qreal m21() const { return matrix(1, 0); }
    qreal m22() const { return matrix(1, 1); }
    qreal m23() const { return matrix(1, 2); }
    qreal m24() const { return matrix(1, 3); }
    qreal m31() const { return matrix(2, 0); }
    qreal m32() const { return matrix(2, 1); }
    qreal m33() const { return matrix(2, 2); }
    qreal m34() const { return matrix(2, 3); }
    qreal m41() const { return matrix(3, 0); }
    qreal m42() const { return matrix(3, 1); }
    qreal m43() const { return matrix(3, 2); }
    qreal m44() const { return matrix(3, 3); }

    void setM11(qreal value) { matrix(0, 0) = value; }
    void setM12(qreal value) { matrix(0, 1) = value; }
    void setM13(qreal value) { matrix(0, 2) = value; }
    void setM14(qreal value) { matrix(0, 3) = value; }
    void setM21(qreal value) { matrix(1, 0) = value; }
    void setM22(qreal value) { matrix(1, 1) = value; }
    void setM23(qreal value) { matrix(1, 2) = value; }
    void setM24(qreal value) { matrix(1, 3) = value; }
    void setM31(qreal value) { matrix(2, 0) = value; }
    void setM32(qreal value) { matrix(2, 1) = value; }
    void setM33(qreal value) { matrix(2, 2) = value; }
    void setM34(qreal value) { matrix(2, 3) = value; }
    void setM41(qreal value) { matrix(3, 0) = value; }
    void setM42(qreal value) { matrix(3, 1) = value; }
    void setM43(qreal value) { matrix(3, 2) = value; }
    void setM44(qreal value) { matrix(3, 3) = value; }

private:
    QMatrix4x4 matrix;
};

class QDeclarativeEasingValueType : public QDeclarativeValueType
{
    CS_OBJECT(QDeclarativeEasingValueType)
    CS_ENUM(Type)

    CS_PROPERTY_READ(type, type)
    CS_PROPERTY_WRITE(type, setType)
    CS_PROPERTY_READ(amplitude, amplitude)
    CS_PROPERTY_WRITE(amplitude, setAmplitude)
    CS_PROPERTY_READ(overshoot, overshoot)
    CS_PROPERTY_WRITE(overshoot, setOvershoot)
    CS_PROPERTY_READ(period, period)
    CS_PROPERTY_WRITE(period, setPeriod)
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

    QDeclarativeEasingValueType(QObject *parent = 0);

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
    CS_OBJECT(QDeclarativeFontValueType)
    CS_ENUM(FontWeight)
    CS_ENUM(Capitalization)

    CS_PROPERTY_READ(family, family)
    CS_PROPERTY_WRITE(family, setFamily)
    CS_PROPERTY_READ(bold, bold)
    CS_PROPERTY_WRITE(bold, setBold)
    CS_PROPERTY_READ(weight, weight)
    CS_PROPERTY_WRITE(weight, setWeight)
    CS_PROPERTY_READ(italic, italic)
    CS_PROPERTY_WRITE(italic, setItalic)
    CS_PROPERTY_READ(underline, underline)
    CS_PROPERTY_WRITE(underline, setUnderline)
    CS_PROPERTY_READ(overline, overline)
    CS_PROPERTY_WRITE(overline, setOverline)
    CS_PROPERTY_READ(strikeout, strikeout)
    CS_PROPERTY_WRITE(strikeout, setStrikeout)
    CS_PROPERTY_READ(pointSize, pointSize)
    CS_PROPERTY_WRITE(pointSize, setPointSize)
    CS_PROPERTY_READ(pixelSize, pixelSize)
    CS_PROPERTY_WRITE(pixelSize, setPixelSize)
    CS_PROPERTY_READ(capitalization, capitalization)
    CS_PROPERTY_WRITE(capitalization, setCapitalization)
    CS_PROPERTY_READ(letterSpacing, letterSpacing)
    CS_PROPERTY_WRITE(letterSpacing, setLetterSpacing)
    CS_PROPERTY_READ(wordSpacing, wordSpacing)
    CS_PROPERTY_WRITE(wordSpacing, setWordSpacing)

public:
    enum FontWeight { Light = QFont::Light,
                       Normal = QFont::Normal,
                       DemiBold = QFont::DemiBold,
                       Bold = QFont::Bold,
                       Black = QFont::Black };
    enum Capitalization { MixedCase = QFont::MixedCase,
                           AllUppercase = QFont::AllUppercase,
                           AllLowercase = QFont::AllLowercase,
                           SmallCaps = QFont::SmallCaps,
                           Capitalize = QFont::Capitalize };

    QDeclarativeFontValueType(QObject *parent = 0);

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
