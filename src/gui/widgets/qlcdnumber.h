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

#ifndef QLCDNUMBER_H
#define QLCDNUMBER_H

#include <qframe.h>

#ifndef QT_NO_LCDNUMBER

class QLCDNumberPrivate;

// LCD number widget
class Q_GUI_EXPORT QLCDNumber : public QFrame
{
   GUI_CS_OBJECT(QLCDNumber)

   GUI_CS_ENUM(Mode)
   GUI_CS_ENUM(SegmentStyle)

   GUI_CS_PROPERTY_READ(smallDecimalPoint, smallDecimalPoint)
   GUI_CS_PROPERTY_WRITE(smallDecimalPoint, setSmallDecimalPoint)

   GUI_CS_PROPERTY_READ(digitCount, digitCount)
   GUI_CS_PROPERTY_WRITE(digitCount, setDigitCount)

   GUI_CS_PROPERTY_READ(mode, mode)
   GUI_CS_PROPERTY_WRITE(mode, setMode)

   GUI_CS_PROPERTY_READ(segmentStyle, segmentStyle)
   GUI_CS_PROPERTY_WRITE(segmentStyle, setSegmentStyle)

   GUI_CS_PROPERTY_READ(value, value)
   GUI_CS_PROPERTY_WRITE(value, cs_displayD)

   GUI_CS_PROPERTY_READ(intValue, intValue)
   GUI_CS_PROPERTY_WRITE(intValue, cs_displayI)

 public:
   GUI_CS_REGISTER_ENUM(
      enum Mode {
         Hex,
         Dec,
         Oct,
         Bin
      };
   )

   GUI_CS_REGISTER_ENUM(
      enum SegmentStyle {
         Outline,
         Filled,
         Flat
      };
   )

   explicit QLCDNumber(QWidget *parent = nullptr);
   explicit QLCDNumber(uint numDigits, QWidget *parent = nullptr);

   QLCDNumber(const QLCDNumber &) = delete;
   QLCDNumber &operator=(const QLCDNumber &) = delete;

   ~QLCDNumber();

   bool smallDecimalPoint() const;
   int digitCount() const;
   void setDigitCount(int nDigits);

   bool checkOverflow(double num) const;
   bool checkOverflow(int num) const;

   Mode mode() const;
   void setMode(Mode);

   SegmentStyle segmentStyle() const;
   void setSegmentStyle(SegmentStyle style);

   double value() const;
   int intValue() const;

   QSize sizeHint() const override;

   // wrapper for overloaded method
   inline void cs_displayD(double num);

   // wrapper for overloaded method
   inline void cs_displayI(int num);

   GUI_CS_SLOT_1(Public, void display(const QString &str))
   GUI_CS_SLOT_OVERLOAD(display, (const QString &))

   GUI_CS_SLOT_1(Public, void display(int num))
   GUI_CS_SLOT_OVERLOAD(display, (int))

   GUI_CS_SLOT_1(Public, void display(double num))
   GUI_CS_SLOT_OVERLOAD(display, (double))

   GUI_CS_SLOT_1(Public, void setHexMode())
   GUI_CS_SLOT_2(setHexMode)

   GUI_CS_SLOT_1(Public, void setDecMode())
   GUI_CS_SLOT_2(setDecMode)

   GUI_CS_SLOT_1(Public, void setOctMode())
   GUI_CS_SLOT_2(setOctMode)

   GUI_CS_SLOT_1(Public, void setBinMode())
   GUI_CS_SLOT_2(setBinMode)

   GUI_CS_SLOT_1(Public, void setSmallDecimalPoint(bool smallDecimalPoint))
   GUI_CS_SLOT_2(setSmallDecimalPoint)

   GUI_CS_SIGNAL_1(Public, void overflow())
   GUI_CS_SIGNAL_2(overflow)

 protected:
   bool event(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QLCDNumber)
};

void QLCDNumber::cs_displayD(double num)
{
   display(num);
}

void QLCDNumber::cs_displayI(int num)
{
   display(num);
}

#endif // QT_NO_LCDNUMBER

#endif
