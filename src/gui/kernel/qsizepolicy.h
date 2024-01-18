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

#ifndef QSIZEPOLICY_H
#define QSIZEPOLICY_H

#include <qobject.h>
#include <qhashfunc.h>

class QVariant;
class QSizePolicy;

inline uint qHash(QSizePolicy key, uint seed = 0);

class Q_GUI_EXPORT QSizePolicy
{
   GUI_CS_GADGET(QSizePolicy)

   GUI_CS_ENUM(Policy)

 public:
   enum PolicyFlag {
      GrowFlag   = 0x1,
      ExpandFlag = 0x2,
      ShrinkFlag = 0x4,
      IgnoreFlag = 0x8,
   };

   GUI_CS_REGISTER_ENUM(
      enum Policy {
         Fixed            = 0x0,
         Minimum          = 0x1,
         Maximum          = 0x4,
         Preferred        = 0x5,
         MinimumExpanding = 0x3,
         Expanding        = 0x7,
         Ignored          = 0xD
      };
   )

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

   QSizePolicy(Policy horizontal, Policy vertical, ControlType type = DefaultType)
      : data(0) {
      bits.horPolicy = horizontal;
      bits.verPolicy = vertical;
      setControlType(type);
   }

   Policy horizontalPolicy() const {
      return static_cast<Policy>(bits.horPolicy);
   }
   Policy verticalPolicy() const {
      return static_cast<Policy>(bits.verPolicy);
   }
   ControlType controlType() const;

   void setHorizontalPolicy(Policy policy) {
      bits.horPolicy = policy;
   }

   void setVerticalPolicy(Policy policy) {
      bits.verPolicy = policy;
   }
   void setControlType(ControlType type);

   Qt::Orientations expandingDirections() const {
      return ( (verticalPolicy()   & ExpandFlag) ? Qt::Vertical   : Qt::Orientations() )
         | ( (horizontalPolicy() & ExpandFlag) ? Qt::Horizontal : Qt::Orientations() ) ;
   }

   void setHeightForWidth(bool isHeightForWidth) {
      bits.hfw = isHeightForWidth;
   }

   bool hasHeightForWidth() const {
      return bits.hfw;
   }
   void setWidthForHeight(bool isWidthForHeight) {
      bits.wfh = isWidthForHeight;
   }

   bool hasWidthForHeight() const {
      return bits.wfh;
   }

   bool operator==(const QSizePolicy &other) const {
      return data == other.data;
   }

   bool operator!=(const QSizePolicy &other) const {
      return data != other.data;
   }

   operator QVariant() const;

   friend uint qHash(QSizePolicy key, uint seed) {
      return qHash(key.data, seed);
   }

   int horizontalStretch() const {
      return static_cast<int>(bits.horStretch);
   }
   int verticalStretch() const {
      return static_cast<int>(bits.verStretch);
   }
   void setHorizontalStretch(int stretchFactor) {
      bits.horStretch = static_cast<quint32>(qBound(0, stretchFactor, 255));
   }
   void setVerticalStretch(int stretchFactor) {
      bits.verStretch = static_cast<quint32>(qBound(0, stretchFactor, 255));
   }
   bool retainSizeWhenHidden() const {
      return bits.retainSizeWhenHidden;
   }
   void setRetainSizeWhenHidden(bool retainSize) {
      bits.retainSizeWhenHidden = retainSize;
   }
   void transpose();

 private:
   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QSizePolicy &policy);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QSizePolicy &policy);

   QSizePolicy(int i) : data(i) { }

   struct Bits {
      quint32 horStretch : 8;
      quint32 verStretch : 8;
      quint32 horPolicy : 4;
      quint32 verPolicy : 4;
      quint32 ctype : 5;
      quint32 hfw : 1;
      quint32 wfh : 1;
      quint32 retainSizeWhenHidden : 1;
   };

   union {
      Bits bits;
      quint32 data;
   };
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSizePolicy::ControlTypes)

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QSizePolicy &policy);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QSizePolicy &policy);

Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QSizePolicy &policy);

inline void QSizePolicy::transpose()
{
   Policy hData = horizontalPolicy();
   Policy vData = verticalPolicy();
   int hStretch = horizontalStretch();
   int vStretch = verticalStretch();
   setHorizontalPolicy(vData);
   setVerticalPolicy(hData);
   setHorizontalStretch(vStretch);
   setVerticalStretch(hStretch);
}

#endif // QSIZEPOLICY_H
