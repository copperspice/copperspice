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

#ifndef QEFFECTS_P_H
#define QEFFECTS_P_H

#include <qnamespace.h>

#ifndef QT_NO_EFFECTS


class QWidget;

struct QEffects {
   enum Direction {
      LeftScroll  = 0x0001,
      RightScroll = 0x0002,
      UpScroll    = 0x0004,
      DownScroll  = 0x0008
   };

   typedef uint DirFlags;
};

extern void Q_GUI_EXPORT qScrollEffect(QWidget *, QEffects::DirFlags dir = QEffects::DownScroll, int time = -1);
extern void Q_GUI_EXPORT qFadeEffect(QWidget *, int time = -1);



#endif // QT_NO_EFFECTS

#endif // QEFFECTS_P_H
