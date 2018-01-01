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

#ifndef QTEXTOPTION_H
#define QTEXTOPTION_H

#include <QtCore/qnamespace.h>
#include <QtCore/qchar.h>
#include <QtCore/qmetatype.h>

QT_BEGIN_NAMESPACE

template <typename T> class QList;
struct QTextOptionPrivate;

class Q_GUI_EXPORT QTextOption
{
 public:
   enum TabType {
      LeftTab,
      RightTab,
      CenterTab,
      DelimiterTab
   };

   struct Q_GUI_EXPORT Tab {
      inline Tab() : position(80), type(QTextOption::LeftTab) { }
      inline Tab(qreal pos, TabType tabType, QChar delim = QChar())
         : position(pos), type(tabType), delimiter(delim) {}

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

   QTextOption(const QTextOption &o);
   QTextOption &operator=(const QTextOption &o);

   inline void setAlignment(Qt::Alignment alignment);
   inline Qt::Alignment alignment() const {
      return Qt::Alignment(align);
   }

   inline void setTextDirection(Qt::LayoutDirection aDirection) {
      this->direction = aDirection;
   }
   inline Qt::LayoutDirection textDirection() const {
      return Qt::LayoutDirection(direction);
   }

   enum WrapMode {
      NoWrap,
      WordWrap,
      ManualWrap,
      WrapAnywhere,
      WrapAtWordBoundaryOrAnywhere
   };
   inline void setWrapMode(WrapMode wrap) {
      wordWrap = wrap;
   }
   inline WrapMode wrapMode() const {
      return static_cast<WrapMode>(wordWrap);
   }

   enum Flag {
      ShowTabsAndSpaces = 0x1,
      ShowLineAndParagraphSeparators = 0x2,
      AddSpaceForLineAndParagraphSeparators = 0x4,
      SuppressColors = 0x8,
      IncludeTrailingSpaces = 0x80000000
   };
   using Flags = QFlags<Flag>;
   inline void setFlags(Flags flags);
   inline Flags flags() const {
      return Flags(f);
   }

   inline void setTabStop(qreal tabStop);
   inline qreal tabStop() const {
      return tab;
   }

   void setTabArray(QList<qreal> tabStops);
   QList<qreal> tabArray() const;

   void setTabs(QList<Tab> tabStops);
   QList<Tab> tabs() const;

   void setUseDesignMetrics(bool b) {
      design = b;
   }
   bool useDesignMetrics() const {
      return design;
   }

 private:
   uint align : 8;
   uint wordWrap : 4;
   uint design : 1;
   uint direction : 2;
   uint unused : 18;
   uint f;
   qreal tab;
   QTextOptionPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextOption::Flags)

inline void QTextOption::setAlignment(Qt::Alignment aalignment)
{
   align = aalignment;
}

inline void QTextOption::setFlags(Flags aflags)
{
   f = aflags;
}

inline void QTextOption::setTabStop(qreal atabStop)
{
   tab = atabStop;
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE( QTextOption::Tab )

#endif // QTEXTOPTION_H
