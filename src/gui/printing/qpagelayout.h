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

#ifndef QPAGELAYOUT_H
#define QPAGELAYOUT_H

#include <qmargins.h>
#include <qsharedpointer.h>
#include <qstring.h>
#include <qpagesize.h>

class QPageLayoutPrivate;

class Q_GUI_EXPORT QPageLayout
{
 public:
   using Unit = QPageSize::Unit;

   enum Orientation  { Portrait, Landscape };

   enum Mode {
      StandardMode,  // Paint Rect includes margins
      FullPageMode   // Paint Rect excludes margins
   };

   QPageLayout();
   QPageLayout(const QPageSize &pageSize, Orientation orientation, const QMarginsF &margins, Unit units = QPageSize::Unit::Point,
                  const QMarginsF &minMargins = QMarginsF(0, 0, 0, 0));

   QPageLayout(const QPageLayout &other);

   ~QPageLayout();

   QPageLayout &operator=(QPageLayout &&other) {
      swap(other);
      return *this;
   }

   QPageLayout &operator=(const QPageLayout &other);

   friend Q_GUI_EXPORT bool operator==(const QPageLayout &lhs, const QPageLayout &rhs);

   bool isEquivalentTo(const QPageLayout &other) const;

   bool isValid() const;

   void swap(QPageLayout &other) {
      qSwap(d, other.d);
   }

   void setMode(Mode mode);
   Mode mode() const;

   void setPageSize(const QPageSize &pageSize, const QMarginsF &minMargins = QMarginsF(0, 0, 0, 0));
   QPageSize pageSize() const;

   void setOrientation(Orientation orientation);
   Orientation orientation() const;

   void setUnits(QPageSize::Unit units);
   QPageSize::Unit units() const;

   bool setMargins(const QMarginsF &margins);
   bool setLeftMargin(qreal leftMargin);
   bool setRightMargin(qreal rightMargin);
   bool setTopMargin(qreal topMargin);
   bool setBottomMargin(qreal bottomMargin);

   QMarginsF margins() const;
   QMarginsF margins(Unit units) const;

   QMargins marginsPoints() const;
   QMargins marginsPixels(int resolution) const;

   void setMinimumMargins(const QMarginsF &minMargins);
   QMarginsF minimumMargins() const;
   QMarginsF maximumMargins() const;

   QRectF fullRect() const;
   QRectF fullRect(Unit units) const;
   QRect fullRectPoints() const;
   QRect fullRectPixels(int resolution) const;

   QRectF paintRect() const;
   QRectF paintRect(Unit units) const;
   QRect paintRectPoints() const;
   QRect paintRectPixels(int resolution) const;

 private:
   QExplicitlySharedDataPointer<QPageLayoutPrivate> d;

   friend class QPageLayoutPrivate;
};

Q_GUI_EXPORT bool operator==(const QPageLayout &lhs, const QPageLayout &rhs);

inline bool operator!=(const QPageLayout &lhs, const QPageLayout &rhs)
{
   return !operator==(lhs, rhs);
}

Q_GUI_EXPORT QDebug operator<<(QDebug dbg, const QPageLayout &pageLayout);

CS_DECLARE_METATYPE(QPageLayout)

#endif
