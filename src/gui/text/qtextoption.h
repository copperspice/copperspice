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

#ifndef QTEXTOPTION_H
#define QTEXTOPTION_H

#include <qchar.h>
#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qvariant.h>

struct QTextOptionPrivate;

class Q_GUI_EXPORT QTextOption
{
   GUI_CS_GADGET(QTextOption)

   GUI_CS_ENUM(WrapMode)

 public:
   enum TabType {
      LeftTab,
      RightTab,
      CenterTab,
      DelimiterTab
   };

   GUI_CS_REGISTER_ENUM(
      enum WrapMode {
         NoWrap,
         WordWrap,
         ManualWrap,
         WrapAnywhere,
         WrapAtWordBoundaryOrAnywhere
      };
   )

   enum Flag {
      ShowTabsAndSpaces = 0x1,
      ShowLineAndParagraphSeparators = 0x2,
      AddSpaceForLineAndParagraphSeparators = 0x4,
      SuppressColors = 0x8,
      IncludeTrailingSpaces = 0x80000000
   };
   using Flags = QFlags<Flag>;


   struct Q_GUI_EXPORT Tab {
      inline Tab()
         : position(80), type(QTextOption::LeftTab)
      { }

      inline Tab(qreal pos, TabType tabType, QChar delim = QChar())
         : position(pos), type(tabType), delimiter(delim)
      { }

      inline bool operator==(const Tab &other) const {
         return type == other.type
            && qFuzzyCompare(position, other.position)
            && delimiter == other.delimiter;
      }

      inline bool operator!=(const Tab &other) const {
         return !operator==(other);
      }

      qreal position;
      TabType type;
      QChar delimiter;
   };

   QTextOption();
   QTextOption(Qt::Alignment alignment);
   ~QTextOption();

   QTextOption(const QTextOption &other);
   QTextOption &operator=(const QTextOption &other);

   inline void setAlignment(Qt::Alignment alignment);

   Qt::Alignment alignment() const {
      return Qt::Alignment(align);
   }

   void setTextDirection(Qt::LayoutDirection layoutDirection) {
      this->direction = layoutDirection;
   }

   Qt::LayoutDirection textDirection() const {
      return Qt::LayoutDirection(direction);
   }

   void setWrapMode(WrapMode mode) {
      wordWrap = mode;
   }

   WrapMode wrapMode() const {
      return static_cast<WrapMode>(wordWrap);
   }

   inline void setFlags(Flags flags);

   Flags flags() const {
      return Flags(f);
   }

   void setTabStop(qreal tabStop);
   qreal tabStop() const {
      return tab;
   }

   void setTabArray(const QList<qreal> &tabStops);
   QList<qreal> tabArray() const;

   void setTabs(const QList<Tab> &tabStops);
   QList<Tab> tabs() const;

   void setUseDesignMetrics(bool enable) {
      design = enable;
   }

   bool useDesignMetrics() const {
      return design;
   }

 private:
   uint align     : 8;
   uint wordWrap  : 4;
   uint design    : 1;
   uint direction : 2;

   uint f;
   qreal tab;

   QTextOptionPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextOption::Flags)

inline void QTextOption::setAlignment(Qt::Alignment alignment)
{
   align = alignment;
}

inline void QTextOption::setFlags(Flags flags)
{
   f = flags;
}

inline void QTextOption::setTabStop(qreal tabStop)
{
   tab = tabStop;
}

CS_DECLARE_METATYPE(QTextOption)
CS_DECLARE_METATYPE(QTextOption::Tab)

#endif
