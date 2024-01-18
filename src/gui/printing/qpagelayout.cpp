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

#include <qpagelayout.h>

#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qdebug.h>

// multiplier for converting units to points.
Q_GUI_EXPORT qreal qt_pointMultiplier(QPageLayout::Unit unit)
{
   switch (unit) {
      case QPageSize::Unit::Millimeter:
         return 2.83464566929;

      case QPageSize::Unit::Point:
         return 1.0;

      case QPageSize::Unit::Inch:
         return 72.0;

      case QPageSize::Unit::Pica:
         return 12;

      case QPageSize::Unit::Didot:
         return 1.065826771;

      case QPageSize::Unit::Cicero:
         return 12.789921252;

      default:
         return 1.0;
   }
}

// Multiplier for converting pixels to points.
extern qreal qt_pixelMultiplier(int resolution);

QPointF qt_convertPoint(const QPointF &xy, QPageLayout::Unit fromUnits, QPageLayout::Unit toUnits)
{
   // If the size have the same units, or are all 0, then don't need to convert
   if (fromUnits == toUnits || xy.isNull()) {
      return xy;
   }

   // If converting to points then convert and round to 0 decimal places
   if (toUnits == QPageSize::Unit::Point) {
      const qreal multiplier = qt_pointMultiplier(fromUnits);
      return QPointF(qRound(xy.x() * multiplier),
            qRound(xy.y() * multiplier));
   }

   // If converting to other units, need to convert to unrounded points first
   QPointF pointXy = (fromUnits == QPageSize::Unit::Point) ? xy : xy * qt_pointMultiplier(fromUnits);

   // Then convert from points to required units rounded to 2 decimal places
   const qreal multiplier = qt_pointMultiplier(toUnits);
   return QPointF(qRound(pointXy.x() * 100 / multiplier) / 100.0,
         qRound(pointXy.y() * 100 / multiplier) / 100.0);
}

Q_GUI_EXPORT QMarginsF qt_convertMargins(const QMarginsF &margins, QPageLayout::Unit fromUnits, QPageLayout::Unit toUnits)
{
   // If the margins have the same units, or are all 0, then don't need to convert
   if (fromUnits == toUnits || margins.isNull()) {
      return margins;
   }

   // If converting to points then convert and round to 0 decimal places
   if (toUnits == QPageSize::Unit::Point) {
      const qreal multiplier = qt_pointMultiplier(fromUnits);
      return QMarginsF(qRound(margins.left()   * multiplier),
            qRound(margins.top()    * multiplier),
            qRound(margins.right()  * multiplier),
            qRound(margins.bottom() * multiplier));
   }

   // If converting to other units, need to convert to unrounded points first
   QMarginsF pointMargins = fromUnits == QPageSize::Unit::Point ? margins : margins * qt_pointMultiplier(fromUnits);

   // Then convert from points to required units rounded to 2 decimal places
   const qreal multiplier = qt_pointMultiplier(toUnits);
   return QMarginsF(qRound(pointMargins.left() * 100 / multiplier) / 100.0,
         qRound(pointMargins.top() * 100 / multiplier) / 100.0,
         qRound(pointMargins.right() * 100 / multiplier) / 100.0,
         qRound(pointMargins.bottom() * 100 / multiplier) / 100.0);
}

class QPageLayoutPrivate : public QSharedData
{
 public:

   QPageLayoutPrivate();
   QPageLayoutPrivate(const QPageSize &pageSize, QPageLayout::Orientation orientation,
      const QMarginsF &margins, QPageLayout::Unit units,
      const QMarginsF &minMargins);
   ~QPageLayoutPrivate();

   bool operator==(const QPageLayoutPrivate &other) const;
   bool isEquivalentTo(const QPageLayoutPrivate &other) const;

   bool isValid() const;

   void clampMargins(const QMarginsF &margins);

   QMarginsF margins(QPageLayout::Unit units) const;
   QMargins marginsPoints() const;
   QMargins marginsPixels(int resolution) const;

   void setDefaultMargins(const QMarginsF &minMargins);

   QSizeF paintSize() const;

   QRectF fullRect() const;
   QRectF fullRect(QPageLayout::Unit units) const;
   QRect fullRectPoints() const;
   QRect fullRectPixels(int resolution) const;

   QRectF paintRect() const;

 private:
   friend class QPageLayout;

   QSizeF fullSizeUnits(QPageLayout::Unit units) const;

   QPageSize m_pageSize;
   QPageLayout::Orientation m_orientation;
   QPageLayout::Mode m_mode;
   QPageLayout::Unit m_units;
   QSizeF m_fullSize;
   QMarginsF m_margins;
   QMarginsF m_minMargins;
   QMarginsF m_maxMargins;
};

QPageLayoutPrivate::QPageLayoutPrivate()
   : m_orientation(QPageLayout::Landscape),
     m_mode(QPageLayout::StandardMode)
{
}

QPageLayoutPrivate::QPageLayoutPrivate(const QPageSize &pageSize, QPageLayout::Orientation orientation,
   const QMarginsF &margins, QPageLayout::Unit units,
   const QMarginsF &minMargins)
   : m_pageSize(pageSize),
     m_orientation(orientation),
     m_mode(QPageLayout::StandardMode),
     m_units(units),
     m_margins(margins)
{
   m_fullSize = fullSizeUnits(m_units);
   setDefaultMargins(minMargins);
}

QPageLayoutPrivate::~QPageLayoutPrivate()
{
}

bool QPageLayoutPrivate::operator==(const QPageLayoutPrivate &other) const
{
   return m_pageSize == other.m_pageSize
      && m_orientation == other.m_orientation
      && m_units == other.m_units
      && m_margins == other.m_margins
      && m_minMargins == other.m_minMargins
      && m_maxMargins == other.m_maxMargins;
}

bool QPageLayoutPrivate::isEquivalentTo(const QPageLayoutPrivate &other) const
{
   return m_pageSize.isEquivalentTo(other.m_pageSize)
      && m_orientation == other.m_orientation
      && qt_convertMargins(m_margins, m_units, QPageSize::Unit::Point)
      == qt_convertMargins(other.m_margins, other.m_units, QPageSize::Unit::Point);
}

bool QPageLayoutPrivate::isValid() const
{
   return m_pageSize.isValid();
}

void QPageLayoutPrivate::clampMargins(const QMarginsF &margins)
{
   m_margins = QMarginsF(qBound(m_minMargins.left(),   margins.left(),   m_maxMargins.left()),
         qBound(m_minMargins.top(),    margins.top(),    m_maxMargins.top()),
         qBound(m_minMargins.right(),  margins.right(),  m_maxMargins.right()),
         qBound(m_minMargins.bottom(), margins.bottom(), m_maxMargins.bottom()));
}

QMarginsF QPageLayoutPrivate::margins(QPageLayout::Unit units) const
{
   return qt_convertMargins(m_margins, m_units, units);
}

QMargins QPageLayoutPrivate::marginsPoints() const
{
   return qt_convertMargins(m_margins, m_units, QPageSize::Unit::Point).toMargins();
}

QMargins QPageLayoutPrivate::marginsPixels(int resolution) const
{
   return marginsPoints() / qt_pixelMultiplier(resolution);
}

void QPageLayoutPrivate::setDefaultMargins(const QMarginsF &minMargins)
{
   m_minMargins = minMargins;
   m_maxMargins = QMarginsF(m_fullSize.width() - m_minMargins.right(),
         m_fullSize.height() - m_minMargins.bottom(),
         m_fullSize.width() - m_minMargins.left(),
         m_fullSize.height() - m_minMargins.top());
   if (m_mode == QPageLayout::StandardMode) {
      clampMargins(m_margins);
   }
}

QSizeF QPageLayoutPrivate::fullSizeUnits(QPageLayout::Unit units) const
{
   QSizeF fullPageSize = m_pageSize.size(QPageSize::Unit(units));
   return m_orientation == QPageLayout::Landscape ? fullPageSize.transposed() : fullPageSize;
}

QRectF QPageLayoutPrivate::fullRect() const
{
   return QRectF(QPointF(0, 0), m_fullSize);
}

QRectF QPageLayoutPrivate::fullRect(QPageLayout::Unit units) const
{
   return units == m_units ? fullRect() : QRectF(QPointF(0, 0), fullSizeUnits(units));
}

QRect QPageLayoutPrivate::fullRectPoints() const
{
   if (m_orientation == QPageLayout::Landscape) {
      return QRect(QPoint(0, 0), m_pageSize.sizePoints().transposed());
   } else {
      return QRect(QPoint(0, 0), m_pageSize.sizePoints());
   }
}

QRect QPageLayoutPrivate::fullRectPixels(int resolution) const
{
   if (m_orientation == QPageLayout::Landscape) {
      return QRect(QPoint(0, 0), m_pageSize.sizePixels(resolution).transposed());
   } else {
      return QRect(QPoint(0, 0), m_pageSize.sizePixels(resolution));
   }
}

QRectF QPageLayoutPrivate::paintRect() const
{
   return m_mode == QPageLayout::FullPageMode ? fullRect() : fullRect() - m_margins;
}

QPageLayout::QPageLayout()
   : d(new QPageLayoutPrivate())
{
}

QPageLayout::QPageLayout(const QPageSize &pageSize, Orientation orientation,
   const QMarginsF &margins, Unit units,
   const QMarginsF &minMargins)
   : d(new QPageLayoutPrivate(pageSize, orientation, margins, units, minMargins))
{
}

QPageLayout::QPageLayout(const QPageLayout &other)
   : d(other.d)
{
}

QPageLayout::~QPageLayout()
{
}

QPageLayout &QPageLayout::operator=(const QPageLayout &other)
{
   d = other.d;
   return *this;
}

bool operator==(const QPageLayout &lhs, const QPageLayout &rhs)
{
   return lhs.d == rhs.d || *lhs.d == *rhs.d;
}

bool QPageLayout::isEquivalentTo(const QPageLayout &other) const
{
   return d && other.d && d->isEquivalentTo(*other.d);
}

bool QPageLayout::isValid() const
{
   return d->isValid();
}

void QPageLayout::setMode(Mode mode)
{
   d.detach();
   d->m_mode = mode;
}

QPageLayout::Mode QPageLayout::mode() const
{
   return d->m_mode;
}

void QPageLayout::setPageSize(const QPageSize &pageSize, const QMarginsF &minMargins)
{
   if (!pageSize.isValid()) {
      return;
   }
   d.detach();
   d->m_pageSize = pageSize;
   d->m_fullSize = d->fullSizeUnits(d->m_units);
   d->setDefaultMargins(minMargins);
}

QPageSize QPageLayout::pageSize() const
{
   return d->m_pageSize;
}

void QPageLayout::setOrientation(Orientation orientation)
{
   if (orientation != d->m_orientation) {
      d.detach();
      d->m_orientation = orientation;
      d->m_fullSize = d->fullSizeUnits(d->m_units);
      // Adust the max margins to reflect change in max page size
      const qreal change = d->m_fullSize.width() - d->m_fullSize.height();
      d->m_maxMargins.setLeft(d->m_maxMargins.left() + change);
      d->m_maxMargins.setRight(d->m_maxMargins.right() + change);
      d->m_maxMargins.setTop(d->m_maxMargins.top() - change);
      d->m_maxMargins.setBottom(d->m_maxMargins.bottom() - change);
   }
}

QPageLayout::Orientation QPageLayout::orientation() const
{
   return d->m_orientation;
}

void QPageLayout::setUnits(Unit units)
{
   if (units != d->m_units) {
      d.detach();
      d->m_margins = qt_convertMargins(d->m_margins, d->m_units, units);
      d->m_minMargins = qt_convertMargins(d->m_minMargins, d->m_units, units);
      d->m_maxMargins = qt_convertMargins(d->m_maxMargins, d->m_units, units);
      d->m_units = units;
      d->m_fullSize = d->fullSizeUnits(d->m_units);
   }
}

QPageLayout::Unit QPageLayout::units() const
{
   return d->m_units;
}

bool QPageLayout::setMargins(const QMarginsF &margins)
{
   if (d->m_mode == FullPageMode) {
      d.detach();
      d->m_margins = margins;
      return true;
   } else if (margins.left() >= d->m_minMargins.left()
      && margins.right() >= d->m_minMargins.right()
      && margins.top() >= d->m_minMargins.top()
      && margins.bottom() >= d->m_minMargins.bottom()
      && margins.left() <= d->m_maxMargins.left()
      && margins.right() <= d->m_maxMargins.right()
      && margins.top() <= d->m_maxMargins.top()
      && margins.bottom() <= d->m_maxMargins.bottom()) {
      d.detach();
      d->m_margins = margins;
      return true;
   }
   return false;
}

bool QPageLayout::setLeftMargin(qreal leftMargin)
{
   if (d->m_mode == FullPageMode
      || (leftMargin >= d->m_minMargins.left() && leftMargin <= d->m_maxMargins.left())) {
      d.detach();
      d->m_margins.setLeft(leftMargin);
      return true;
   }
   return false;
}

bool QPageLayout::setRightMargin(qreal rightMargin)
{
   if (d->m_mode == FullPageMode
      || (rightMargin >= d->m_minMargins.right() && rightMargin <= d->m_maxMargins.right())) {
      d.detach();
      d->m_margins.setRight(rightMargin);
      return true;
   }
   return false;
}

bool QPageLayout::setTopMargin(qreal topMargin)
{
   if (d->m_mode == FullPageMode
      || (topMargin >= d->m_minMargins.top() && topMargin <= d->m_maxMargins.top())) {
      d.detach();
      d->m_margins.setTop(topMargin);
      return true;
   }
   return false;
}

bool QPageLayout::setBottomMargin(qreal bottomMargin)
{
   if (d->m_mode == FullPageMode
      || (bottomMargin >= d->m_minMargins.bottom() && bottomMargin <= d->m_maxMargins.bottom())) {
      d.detach();
      d->m_margins.setBottom(bottomMargin);
      return true;
   }
   return false;
}

QMarginsF QPageLayout::margins() const
{
   return d->m_margins;
}

QMarginsF QPageLayout::margins(Unit units) const
{
   return d->margins(units);
}

QMargins QPageLayout::marginsPoints() const
{
   return d->marginsPoints();
}

QMargins QPageLayout::marginsPixels(int resolution) const
{
   return d->marginsPixels(resolution);
}

void QPageLayout::setMinimumMargins(const QMarginsF &minMargins)
{
   d.detach();
   d->setDefaultMargins(minMargins);
}

QMarginsF QPageLayout::minimumMargins() const
{
   return d->m_minMargins;
}

QMarginsF QPageLayout::maximumMargins() const
{
   return d->m_maxMargins;
}

QRectF QPageLayout::fullRect() const
{
   return isValid() ? d->fullRect() : QRect();
}

QRectF QPageLayout::fullRect(Unit units) const
{
   return isValid() ? d->fullRect(units) : QRect();
}

QRect QPageLayout::fullRectPoints() const
{
   return isValid() ? d->fullRectPoints() : QRect();
}

QRect QPageLayout::fullRectPixels(int resolution) const
{
   return isValid() ? d->fullRectPixels(resolution) : QRect();
}

QRectF QPageLayout::paintRect() const
{
   return isValid() ? d->paintRect() : QRectF();
}

QRectF QPageLayout::paintRect(Unit units) const
{
   if (! isValid()) {
      return QRectF();
   }

   if (units == d->m_units) {
      return d->paintRect();
   }

   return d->m_mode == FullPageMode ? d->fullRect(units)
      : d->fullRect(units) - d->margins(units);
}

QRect QPageLayout::paintRectPoints() const
{
   if (! isValid()) {
      return QRect();
   }
   return d->m_mode == FullPageMode ? d->fullRectPoints()
      : d->fullRectPoints() - d->marginsPoints();
}

QRect QPageLayout::paintRectPixels(int resolution) const
{
   if (! isValid()) {
      return QRect();
   }

   return d->m_mode == FullPageMode ? d->fullRectPixels(resolution)
      : d->fullRectPixels(resolution) - d->marginsPixels(resolution);
}

QDebug operator<<(QDebug dbg, const QPageLayout &layout)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg.noquote();
   dbg << "QPageLayout(";

   if (layout.isValid()) {
      const QMarginsF margins = layout.margins();

      dbg << '"' << layout.pageSize().name() << "\", "
         << (layout.orientation() == QPageLayout::Portrait ? "Portrait" : "Landscape")
         << ", l:" << margins.left() << " r:" << margins.right() << " t:"
         << margins.top() << " b:" << margins.bottom() << ' ';

      switch (layout.units()) {
         case QPageSize::Unit::Millimeter:
            dbg << "mm";
            break;

         case QPageSize::Unit::Point:
            dbg << "pt";
            break;

         case QPageSize::Unit::Inch:
            dbg << "in";
            break;

         case QPageSize::Unit::Pica:
            dbg << "pc";
            break;

         case QPageSize::Unit::Didot:
            dbg << "DD";
            break;

         case QPageSize::Unit::Cicero:
            dbg << "CC";
            break;

         default:
            dbg << "Unknown";
            break;
      }
   }

   dbg << ')';

   return dbg;
}
