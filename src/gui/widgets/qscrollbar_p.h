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

#ifndef QSCROLLBAR_P_H
#define QSCROLLBAR_P_H

#include <qabstractslider_p.h>
#include <qstyle.h>

class QScrollBarPrivate : public QAbstractSliderPrivate
{
   Q_DECLARE_PUBLIC(QScrollBar)

 public:
   QStyle::SubControl pressedControl;
   bool pointerOutsidePressedControl;

   int clickOffset, snapBackPosition;

   void activateControl(uint control, int threshold = 500);
   void stopRepeatAction();
   int pixelPosToRangeValue(int pos) const;
   void init();
   bool updateHoverControl(const QPoint &pos);
   QStyle::SubControl newHoverControl(const QPoint &pos);

   QStyle::SubControl hoverControl;
   QRect hoverRect;

   bool transient;
   void setTransient(bool value);

   bool flashed;
   int flashTimer;
   void flash();
};

#endif
