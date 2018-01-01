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

#ifndef QSIZEPOLICY_H
#define QSIZEPOLICY_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QVariant;

class Q_GUI_EXPORT QSizePolicy
{
   GUI_CS_GADGET(QSizePolicy)
   GUI_CS_ENUM(Policy)

   enum SizePolicyMasks {
      HSize = 4,
      HMask = 0x0f,
      VMask = HMask << HSize,
      CTShift = 9,
      CTSize = 5,
      CTMask = ((0x1 << CTSize) - 1) << CTShift,
      WFHShift = CTShift + CTSize,
      UnusedShift = WFHShift + 1,
      UnusedSize = 1
   };

 public:
   enum PolicyFlag {
      GrowFlag = 1,
      ExpandFlag = 2,
      ShrinkFlag = 4,
      IgnoreFlag = 8
   };

   enum Policy {
      Fixed = 0,
      Minimum = GrowFlag,
      Maximum = ShrinkFlag,
      Preferred = GrowFlag | ShrinkFlag,
      MinimumExpanding = GrowFlag | ExpandFlag,
      Expanding = GrowFlag | ShrinkFlag | ExpandFlag,
      Ignored = ShrinkFlag | GrowFlag | IgnoreFlag
   };

   enum ControlType {
      DefaultType      = 0x00000001,
      ButtonBox        = 0x00000002,
      CheckBox         = 0x00000004,
      ComboBox         = 0x00000008,
      Frame            = 0x00000010,
      GroupBox         = 0x00000020,
      Label            = 0x00000040,
      Line             = 0x00000080,
      LineEdit         = 0x00000100,
      PushButton       = 0x00000200,
      RadioButton      = 0x00000400,
      Slider           = 0x00000800,
      SpinBox          = 0x00001000,
      TabWidget        = 0x00002000,
      ToolButton       = 0x00004000
   };
   using ControlTypes = QFlags<ControlType>;

   QSizePolicy() : data(0) { }

   // ### Qt5/merge these two constructors (with type == DefaultType)
   QSizePolicy(Policy horizontal, Policy vertical)
      : data(horizontal | (vertical << HSize)) { }
   QSizePolicy(Policy horizontal, Policy vertical, ControlType type)
      : data(horizontal | (vertical << HSize)) {
      setControlType(type);
   }

   Policy horizontalPolicy() const {
      return static_cast<Policy>(data & HMask);
   }
   Policy verticalPolicy() const {
      return static_cast<Policy>((data & VMask) >> HSize);
   }
   ControlType controlType() const;

   void setHorizontalPolicy(Policy d) {
      data = (data & ~HMask) | d;
   }
   void setVerticalPolicy(Policy d) {
      data = (data & ~(HMask << HSize)) | (d << HSize);
   }
   void setControlType(ControlType type);

   Qt::Orientations expandingDirections() const {
      Qt::Orientations result;
      if (verticalPolicy() & ExpandFlag) {
         result |= Qt::Vertical;
      }
      if (horizontalPolicy() & ExpandFlag) {
         result |= Qt::Horizontal;
      }
      return result;
   }

   void setHeightForWidth(bool b) {
      data = b ? (data | (1 << 2 * HSize)) : (data & ~(1 << 2 * HSize));
   }
   bool hasHeightForWidth() const {
      return data & (1 << 2 * HSize);
   }
   void setWidthForHeight(bool b) {
      data = b ? (data | (1 << (WFHShift))) : (data & ~(1 << (WFHShift)));
   }
   bool hasWidthForHeight() const {
      return data & (1 << (WFHShift));
   }

   bool operator==(const QSizePolicy &s) const {
      return data == s.data;
   }
   bool operator!=(const QSizePolicy &s) const {
      return data != s.data;
   }
   operator QVariant() const; // implemented in qabstractlayout.cpp

   int horizontalStretch() const {
      return data >> 24;
   }
   int verticalStretch() const {
      return (data >> 16) & 0xff;
   }
   void setHorizontalStretch(uchar stretchFactor) {
      data = (data & 0x00ffffff) | (uint(stretchFactor) << 24);
   }
   void setVerticalStretch(uchar stretchFactor) {
      data = (data & 0xff00ffff) | (uint(stretchFactor) << 16);
   }

   void transpose();

 private:

#ifndef QT_NO_DATASTREAM
   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSizePolicy &);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSizePolicy &);
#endif
   QSizePolicy(int i) : data(i) { }

   quint32 data;
   /*  Qt5/Use bit flags instead, keep it here for improved readability for now.
       We can maybe change it for Qt4, but we'd have to be careful, since the behaviour
       is implementation defined. It usually varies between little- and big-endian compilers, but
       it might also not vary.
       quint32 horzPolicy : 4;
       quint32 vertPolicy : 4;
       quint32 hfw : 1;
       quint32 ctype : 5;
       quint32 wfh : 1;
       quint32 padding : 1;   // we cannot use the highest bit
       quint32 verStretch : 8;
       quint32 horStretch : 8;
   */

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSizePolicy::ControlTypes)

#ifndef QT_NO_DATASTREAM
// implemented in qlayout.cpp
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QSizePolicy &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QSizePolicy &);
#endif

inline void QSizePolicy::transpose()
{
   Policy hData = horizontalPolicy();
   Policy vData = verticalPolicy();
   uchar hStretch = uchar(horizontalStretch());
   uchar vStretch = uchar(verticalStretch());
   setHorizontalPolicy(vData);
   setVerticalPolicy(hData);
   setHorizontalStretch(vStretch);
   setVerticalStretch(hStretch);
}

QT_END_NAMESPACE

#endif // QSIZEPOLICY_H
