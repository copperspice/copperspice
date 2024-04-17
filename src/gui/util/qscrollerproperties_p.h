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

#ifndef QSCROLLERPROPERTIES_P_H
#define QSCROLLERPROPERTIES_P_H

#include <qeasingcurve.h>
#include <qpointf.h>
#include <qscrollerproperties.h>

class QScrollerPropertiesPrivate
{
 public:
   static QScrollerPropertiesPrivate *defaults();

   bool operator==(const QScrollerPropertiesPrivate &) const;

   qreal mousePressEventDelay;
   qreal dragStartDistance;
   qreal dragVelocitySmoothingFactor;
   qreal axisLockThreshold;
   QEasingCurve scrollingCurve;
   qreal decelerationFactor;
   qreal minimumVelocity;
   qreal maximumVelocity;
   qreal maximumClickThroughVelocity;
   qreal acceleratingFlickMaximumTime;
   qreal acceleratingFlickSpeedupFactor;
   qreal snapPositionRatio;
   qreal snapTime;
   qreal overshootDragResistanceFactor;
   qreal overshootDragDistanceFactor;
   qreal overshootScrollDistanceFactor;
   qreal overshootScrollTime;
   QScrollerProperties::OvershootPolicy hOvershootPolicy;
   QScrollerProperties::OvershootPolicy vOvershootPolicy;
   QScrollerProperties::FrameRates frameRate;
};

#endif // QSCROLLERPROPERTIES_P_H
