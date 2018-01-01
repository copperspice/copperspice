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

#ifndef QMARGINS_H
#define QMARGINS_H

#include <qnamespace.h>

class QDebug;

class QMargins
{
 public:
   QMargins();
   QMargins(int left, int top, int right, int bottom);

   bool isNull() const;

   int left() const;
   int top() const;
   int right() const;
   int bottom() const;

   void setLeft(int left);
   void setTop(int top);
   void setRight(int right);
   void setBottom(int bottom);

 private:
   int m_left;
   int m_top;
   int m_right;
   int m_bottom;
};

Q_DECLARE_TYPEINFO(QMargins, Q_MOVABLE_TYPE);


inline QMargins::QMargins()
{
   m_top = m_bottom = m_left = m_right = 0;
}

inline QMargins::QMargins(int aleft, int atop, int aright, int abottom)
   : m_left(aleft), m_top(atop), m_right(aright), m_bottom(abottom) {}

inline bool QMargins::isNull() const
{
   return m_left == 0 && m_top == 0 && m_right == 0 && m_bottom == 0;
}

inline int QMargins::left() const
{
   return m_left;
}

inline int QMargins::top() const
{
   return m_top;
}

inline int QMargins::right() const
{
   return m_right;
}

inline int QMargins::bottom() const
{
   return m_bottom;
}

inline void QMargins::setLeft(int aleft)
{
   m_left = aleft;
}

inline void QMargins::setTop(int atop)
{
   m_top = atop;
}

inline void QMargins::setRight(int aright)
{
   m_right = aright;
}

inline void QMargins::setBottom(int abottom)
{
   m_bottom = abottom;
}

inline bool operator==(const QMargins &m1, const QMargins &m2)
{
   return
      m1.left()   == m2.left()  &&
      m1.top()    == m2.top()   &&
      m1.right()  == m2.right() &&
      m1.bottom() == m2.bottom();
}

inline bool operator!=(const QMargins &m1, const QMargins &m2)
{
   return
      m1.left()   != m2.left()   ||
      m1.top()    != m2.top()    ||
      m1.right()  != m2.right()  ||
      m1.bottom() != m2.bottom();
}

Q_CORE_EXPORT QDebug operator<<(QDebug, const QMargins &);

#endif