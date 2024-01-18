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

#ifndef QLAYOUTPOLICY_H
#define QLAYOUTPOLICY_H

#include <qobject.h>
#include <qnamespace.h>
#include <qdatastream.h>

class QVariant;

class Q_GUI_EXPORT QLayoutPolicy
{
 public:
   enum PolicyFlag {
      GrowFlag = 1,
      ExpandFlag = 2,
      ShrinkFlag = 4,
      IgnoreFlag = 8
   };

   enum Policy {
      Fixed     = 0,
      Minimum   = GrowFlag,
      Maximum   = ShrinkFlag,
      Preferred = GrowFlag | ShrinkFlag,
      MinimumExpanding = GrowFlag | ExpandFlag,
      Expanding = GrowFlag | ShrinkFlag | ExpandFlag,
      Ignored   = ShrinkFlag | GrowFlag | IgnoreFlag
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

   QLayoutPolicy() : data(0) { }

   QLayoutPolicy(Policy horizontal, Policy vertical, ControlType type = DefaultType)
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

   void setHorizontalPolicy(Policy d) {
      bits.horPolicy = d;
   }
   void setVerticalPolicy(Policy d) {
      bits.verPolicy = d;
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
      bits.hfw = b;
   }
   bool hasHeightForWidth() const {
      return bits.hfw;
   }
   void setWidthForHeight(bool b) {
      bits.wfh = b;
   }
   bool hasWidthForHeight() const {
      return bits.wfh;
   }

   bool operator==(const QLayoutPolicy &s) const {
      return data == s.data;
   }
   bool operator!=(const QLayoutPolicy &s) const {
      return data != s.data;
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

   void transpose();

 private:
   friend QDataStream &operator<<(QDataStream &, const QLayoutPolicy &);
   friend QDataStream &operator>>(QDataStream &, QLayoutPolicy &);

   QLayoutPolicy(int i) : data(i) { }

   union {
      struct {
         quint32 horStretch : 8;
         quint32 verStretch : 8;
         quint32 horPolicy : 4;
         quint32 verPolicy : 4;
         quint32 ctype : 5;
         quint32 hfw : 1;
         quint32 wfh : 1;
         quint32 padding : 1;   // feel free to use
      } bits;
      quint32 data;
   };
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QLayoutPolicy::ControlTypes)

QDataStream &operator<<(QDataStream &, const QLayoutPolicy &);
QDataStream &operator>>(QDataStream &, QLayoutPolicy &);

QDebug operator<<(QDebug dbg, const QLayoutPolicy &);


inline void QLayoutPolicy::transpose()
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

#endif
