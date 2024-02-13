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

#ifndef QPOINT_H
#define QPOINT_H

#include <qnamespace.h>

class QDataStream;
class QDebug;

class Q_CORE_EXPORT QPoint
{
 public:
   QPoint();
   QPoint(int xPos, int yPos);

   inline bool isNull() const;

   inline int x() const;
   inline int y() const;
   inline void setX(int xPos);
   inline void setY(int yPos);

   int manhattanLength() const;

   inline int &rx();
   inline int &ry();

   inline QPoint &operator+=(const QPoint &point);
   inline QPoint &operator-=(const QPoint &point);

   inline QPoint &operator*=(float factor);
   inline QPoint &operator*=(double factor);
   inline QPoint &operator*=(int factor);
   inline QPoint &operator/=(qreal factor);

 private:
   int xp;
   int yp;

   friend inline bool operator==(const QPoint &point1, const QPoint &point2);
   friend inline bool operator!=(const QPoint &point1, const QPoint &point2);

   friend inline const QPoint operator+(const QPoint &point1, const QPoint &point2);
   friend inline const QPoint operator-(const QPoint &point1, const QPoint &point2);
   friend inline const QPoint operator*(const QPoint &point, float factor);
   friend inline const QPoint operator*(float factor, const QPoint &point);
   friend inline const QPoint operator*(const QPoint &point, double factor);
   friend inline const QPoint operator*(double factor, const QPoint &point);
   friend inline const QPoint operator*(const QPoint &point, int factor);
   friend inline const QPoint operator*(int factor, const QPoint &point);
   friend inline const QPoint operator-(const QPoint &point);
   friend inline const QPoint operator/(const QPoint &point, qreal factor);
   friend class QTransform;
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QPoint &point);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QPoint &point);

inline QPoint::QPoint()
{
   xp = 0;
   yp = 0;
}

inline QPoint::QPoint(int xPos, int yPos)
{
   xp = xPos;
   yp = yPos;
}

inline bool QPoint::isNull() const
{
   return xp == 0 && yp == 0;
}

inline int QPoint::x() const
{
   return xp;
}

inline int QPoint::y() const
{
   return yp;
}

inline void QPoint::setX(int xPos)
{
   xp = xPos;
}

inline void QPoint::setY(int yPos)
{
   yp = yPos;
}

inline int &QPoint::rx()
{
   return xp;
}

inline int &QPoint::ry()
{
   return yp;
}

inline QPoint &QPoint::operator+=(const QPoint &p)
{
   xp += p.xp;
   yp += p.yp;
   return *this;
}

inline QPoint &QPoint::operator-=(const QPoint &p)
{
   xp -= p.xp;
   yp -= p.yp;
   return *this;
}

inline QPoint &QPoint::operator*=(float factor)
{
   xp = qRound(xp * factor);
   yp = qRound(yp * factor);
   return *this;
}

inline QPoint &QPoint::operator*=(double factor)
{
   xp = qRound(xp * factor);
   yp = qRound(yp * factor);
   return *this;
}

inline QPoint &QPoint::operator*=(int factor)
{
   xp = xp * factor;
   yp = yp * factor;
   return *this;
}

inline bool operator==(const QPoint &point1, const QPoint &point2)
{
   return point1.xp == point2.xp && point1.yp == point2.yp;
}

inline bool operator!=(const QPoint &point1, const QPoint &point2)
{
   return point1.xp != point2.xp || point1.yp != point2.yp;
}

inline const QPoint operator+(const QPoint &point1, const QPoint &point2)
{
   return QPoint(point1.xp + point2.xp, point1.yp + point2.yp);
}

inline const QPoint operator-(const QPoint &point1, const QPoint &point2)
{
   return QPoint(point1.xp - point2.xp, point1.yp - point2.yp);
}

inline const QPoint operator*(const QPoint &p, float factor)
{
   return QPoint(qRound(p.xp * factor), qRound(p.yp * factor));
}

inline const QPoint operator*(const QPoint &p, double factor)
{
   return QPoint(qRound(p.xp * factor), qRound(p.yp * factor));
}

inline const QPoint operator*(const QPoint &p, int factor)
{
   return QPoint(p.xp * factor, p.yp * factor);
}

inline const QPoint operator*(float factor, const QPoint &p)
{
   return QPoint(qRound(p.xp * factor), qRound(p.yp * factor));
}

inline const QPoint operator*(double factor, const QPoint &p)
{
   return QPoint(qRound(p.xp * factor), qRound(p.yp * factor));
}

inline const QPoint operator*(int factor, const QPoint &point)
{
   return QPoint(point.xp * factor, point.yp * factor);
}

inline const QPoint operator-(const QPoint &point)
{
   return QPoint(-point.xp, -point.yp);
}

inline QPoint &QPoint::operator/=(qreal factor)
{
   xp = qRound(xp / factor);
   yp = qRound(yp / factor);
   return *this;
}

inline const QPoint operator/(const QPoint &p, qreal factor)
{
   return QPoint(qRound(p.xp / factor), qRound(p.yp / factor));
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QPoint &point);

class Q_CORE_EXPORT QPointF
{
 public:
   QPointF();
   QPointF(const QPoint &point);
   QPointF(qreal xPos, qreal yPos);

   qreal manhattanLength() const;

   inline bool isNull() const;

   inline qreal x() const;
   inline qreal y() const;
   inline void setX(qreal xPos);
   inline void setY(qreal yPos);

   inline qreal &rx();
   inline qreal &ry();

   inline QPointF &operator+=(const QPointF &point);
   inline QPointF &operator-=(const QPointF &point);

   inline QPointF &operator*=(qreal factor);
   inline QPointF &operator/=(qreal factor);

   inline QPoint toPoint() const;

 private:
   double xp;
   double yp;

   friend inline bool operator==(const QPointF &point1, const QPointF &point2);
   friend inline bool operator!=(const QPointF &point1, const QPointF &point2);
   friend inline const QPointF operator+(const QPointF &point1, const QPointF &point2);
   friend inline const QPointF operator-(const QPointF &point1, const QPointF &point2);
   friend inline const QPointF operator*(qreal factor, const QPointF &point);
   friend inline const QPointF operator*(const QPointF &point, qreal factor);
   friend inline const QPointF operator-(const QPointF &point);
   friend inline const QPointF operator/(const QPointF &point, qreal factor);

   friend class QMatrix;
   friend class QTransform;
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &stream, const QPointF &pointF);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &stream, QPointF &pointF);

inline QPointF::QPointF()
   : xp(0), yp(0)
{ }

inline QPointF::QPointF(qreal xPos, qreal yPos)
   : xp(xPos), yp(yPos)
{ }

inline QPointF::QPointF(const QPoint &p)
   : xp(p.x()), yp(p.y())
{ }

inline bool QPointF::isNull() const
{
   return qIsNull(xp) && qIsNull(yp);
}

inline qreal QPointF::x() const
{
   return xp;
}

inline qreal QPointF::y() const
{
   return yp;
}

inline void QPointF::setX(qreal xPos)
{
   xp = xPos;
}

inline void QPointF::setY(qreal yPos)
{
   yp = yPos;
}

inline qreal &QPointF::rx()
{
   return xp;
}

inline qreal &QPointF::ry()
{
   return yp;
}

inline QPointF &QPointF::operator+=(const QPointF &p)
{
   xp += p.xp;
   yp += p.yp;
   return *this;
}

inline QPointF &QPointF::operator-=(const QPointF &p)
{
   xp -= p.xp;
   yp -= p.yp;
   return *this;
}

inline QPointF &QPointF::operator*=(qreal factor)
{
   xp *= factor;
   yp *= factor;
   return *this;
}

inline bool operator==(const QPointF &point1, const QPointF &point2)
{
   return qFuzzyIsNull(point1.xp - point2.xp) && qFuzzyIsNull(point1.yp - point2.yp);
}

inline bool operator!=(const QPointF &point1, const QPointF &point2)
{
   return !qFuzzyIsNull(point1.xp - point2.xp) || !qFuzzyIsNull(point1.yp - point2.yp);
}

inline const QPointF operator+(const QPointF &point1, const QPointF &point2)
{
   return QPointF(point1.xp + point2.xp, point1.yp + point2.yp);
}

inline const QPointF operator-(const QPointF &point1, const QPointF &point2)
{
   return QPointF(point1.xp - point2.xp, point1.yp - point2.yp);
}

inline const QPointF operator*(const QPointF &p, qreal factor)
{
   return QPointF(p.xp * factor, p.yp * factor);
}

inline const QPointF operator*(qreal factor, const QPointF &point)
{
   return QPointF(point.xp * factor, point.yp * factor);
}

inline const QPointF operator-(const QPointF &point)
{
   return QPointF(-point.xp, -point.yp);
}

inline QPointF &QPointF::operator/=(qreal factor)
{
   xp /= factor;
   yp /= factor;
   return *this;
}

inline const QPointF operator/(const QPointF &point, qreal factor)
{
   return QPointF(point.xp / factor, point.yp / factor);
}

inline QPoint QPointF::toPoint() const
{
   return QPoint(qRound(xp), qRound(yp));
}

Q_CORE_EXPORT QDebug operator<<(QDebug d, const QPointF &point);

#endif // QPOINT_H
