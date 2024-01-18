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

#ifndef QLAYOUTENGINE_P_H
#define QLAYOUTENGINE_P_H

#include <qlayoutitem.h>
#include <qstyle.h>

template <typename T>
class QVector;

struct QLayoutStruct {
   inline void init(int stretchFactor = 0, int minSize = 0) {
      stretch = stretchFactor;
      minimumSize = sizeHint = minSize;
      maximumSize = QLAYOUTSIZE_MAX;
      expansive = false;
      empty = true;
      spacing = 0;
   }

   int smartSizeHint() {
      return (stretch > 0) ? minimumSize : sizeHint;
   }

   int effectiveSpacer(int uniformSpacer) const {
      Q_ASSERT(uniformSpacer >= 0 || spacing >= 0);
      return (uniformSpacer >= 0) ? uniformSpacer : spacing;
   }

   // parameters
   int stretch;
   int sizeHint;
   int maximumSize;
   int minimumSize;
   bool expansive;
   bool empty;
   int spacing;

   // temporary storage
   bool done;

   // result
   int pos;
   int size;
};


Q_GUI_EXPORT void qGeomCalc(QVector<QLayoutStruct> &chain, int start, int count,
   int pos, int space, int spacer = -1);

Q_GUI_EXPORT QSize qSmartMinSize(const QSize &sizeHint, const QSize &minSizeHint,
   const QSize &minSize, const QSize &maxSize, const QSizePolicy &sizePolicy);

Q_GUI_EXPORT QSize qSmartMinSize(const QWidgetItem *i);
Q_GUI_EXPORT QSize qSmartMinSize(const QWidget *w);

Q_GUI_EXPORT QSize qSmartMaxSize(const QSize &sizeHint, const QSize &minSize, const QSize &maxSize,
   const QSizePolicy &sizePolicy, Qt::Alignment align = Qt::EmptyFlag);

Q_GUI_EXPORT QSize qSmartMaxSize(const QWidgetItem *i, Qt::Alignment align = Qt::EmptyFlag);
Q_GUI_EXPORT QSize qSmartMaxSize(const QWidget *w, Qt::Alignment align = Qt::EmptyFlag);
Q_GUI_EXPORT int qSmartSpacing(const QLayout *layout, QStyle::PixelMetric pm);

/*
  Modify total maximum (max), total expansion (exp), and total empty
  when adding boxmax/boxexp.

  Expansive boxes win over non-expansive boxes.
  Non-empty boxes win over empty boxes.
*/
static inline void qMaxExpCalc(int &max, bool &exp, bool &empty, int boxmax, bool boxexp, bool boxempty)
{
   if (exp) {
      if (boxexp) {
         max = qMax(max, boxmax);
      }

   } else {
      if (boxexp || (empty && (!boxempty || max == 0))) {
         max = boxmax;
      } else if (empty == boxempty) {
         max = qMin(max, boxmax);
      }
   }

   exp = exp || boxexp;
   empty = empty && boxempty;
}

#endif
