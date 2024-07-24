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

#include <qscrollerproperties.h>
#include <qscrollerproperties_p.h>

#include <qmath.h>
#include <qobject.h>
#include <qpointer.h>

static QScrollerPropertiesPrivate *userDefaults   = nullptr;
static QScrollerPropertiesPrivate *systemDefaults = nullptr;

QScrollerPropertiesPrivate *QScrollerPropertiesPrivate::defaults()
{
   if (! systemDefaults) {
      QScrollerPropertiesPrivate spp;
      spp.mousePressEventDelay = qreal(0.25);
      spp.dragStartDistance = qreal(5.0 / 1000);
      spp.dragVelocitySmoothingFactor = qreal(0.8);
      spp.axisLockThreshold = qreal(0);
      spp.scrollingCurve.setType(QEasingCurve::OutQuad);
      spp.decelerationFactor = qreal(0.125);
      spp.minimumVelocity = qreal(50.0 / 1000);
      spp.maximumVelocity = qreal(500.0 / 1000);
      spp.maximumClickThroughVelocity = qreal(66.5 / 1000);
      spp.acceleratingFlickMaximumTime = qreal(1.25);
      spp.acceleratingFlickSpeedupFactor = qreal(3.0);
      spp.snapPositionRatio = qreal(0.5);
      spp.snapTime = qreal(0.3);
      spp.overshootDragResistanceFactor = qreal(0.5);
      spp.overshootDragDistanceFactor = qreal(1);
      spp.overshootScrollDistanceFactor = qreal(0.5);
      spp.overshootScrollTime = qreal(0.7);

      spp.hOvershootPolicy = QScrollerProperties::OvershootWhenScrollable;
      spp.vOvershootPolicy = QScrollerProperties::OvershootWhenScrollable;
      spp.frameRate = QScrollerProperties::Standard;

      systemDefaults = new QScrollerPropertiesPrivate(spp);
   }

   return new QScrollerPropertiesPrivate(userDefaults ? *userDefaults : *systemDefaults);
}

QScrollerProperties::QScrollerProperties()
   : d(QScrollerPropertiesPrivate::defaults())
{
}

QScrollerProperties::QScrollerProperties(const QScrollerProperties &sp)
   : d(new QScrollerPropertiesPrivate(*sp.d))
{
}

QScrollerProperties &QScrollerProperties::operator=(const QScrollerProperties &sp)
{
   *d.data() = *sp.d.data();
   return *this;
}

QScrollerProperties::~QScrollerProperties()
{
}

bool QScrollerProperties::operator==(const QScrollerProperties &sp) const
{
   return *d.data() == *sp.d.data();
}

bool QScrollerProperties::operator!=(const QScrollerProperties &sp) const
{
   return !(*d.data() == *sp.d.data());
}

bool QScrollerPropertiesPrivate::operator==(const QScrollerPropertiesPrivate &p) const
{
   bool same = true;
   same &= (mousePressEventDelay == p.mousePressEventDelay);
   same &= (dragStartDistance == p.dragStartDistance);
   same &= (dragVelocitySmoothingFactor == p.dragVelocitySmoothingFactor);
   same &= (axisLockThreshold == p.axisLockThreshold);
   same &= (scrollingCurve == p.scrollingCurve);
   same &= (decelerationFactor == p.decelerationFactor);
   same &= (minimumVelocity == p.minimumVelocity);
   same &= (maximumVelocity == p.maximumVelocity);
   same &= (maximumClickThroughVelocity == p.maximumClickThroughVelocity);
   same &= (acceleratingFlickMaximumTime == p.acceleratingFlickMaximumTime);
   same &= (acceleratingFlickSpeedupFactor == p.acceleratingFlickSpeedupFactor);
   same &= (snapPositionRatio == p.snapPositionRatio);
   same &= (snapTime == p.snapTime);
   same &= (overshootDragResistanceFactor == p.overshootDragResistanceFactor);
   same &= (overshootDragDistanceFactor == p.overshootDragDistanceFactor);
   same &= (overshootScrollDistanceFactor == p.overshootScrollDistanceFactor);
   same &= (overshootScrollTime == p.overshootScrollTime);
   same &= (hOvershootPolicy == p.hOvershootPolicy);
   same &= (vOvershootPolicy == p.vOvershootPolicy);
   same &= (frameRate == p.frameRate);
   return same;
}

void QScrollerProperties::setDefaultScrollerProperties(const QScrollerProperties &sp)
{
   if (! userDefaults) {
      userDefaults = new QScrollerPropertiesPrivate(*sp.d);
   } else {
      *userDefaults = *sp.d;
   }
}

void QScrollerProperties::unsetDefaultScrollerProperties()
{
   delete userDefaults;
   userDefaults = nullptr;
}

QVariant QScrollerProperties::scrollMetric(ScrollMetric metric) const
{
   switch (metric) {
      case MousePressEventDelay:
         return d->mousePressEventDelay;

      case DragStartDistance:
         return d->dragStartDistance;

      case DragVelocitySmoothingFactor:
         return d->dragVelocitySmoothingFactor;

      case AxisLockThreshold:
         return d->axisLockThreshold;

      case ScrollingCurve:
         return d->scrollingCurve;

      case DecelerationFactor:
         return d->decelerationFactor;

      case MinimumVelocity:
         return d->minimumVelocity;

      case MaximumVelocity:
         return d->maximumVelocity;

      case MaximumClickThroughVelocity:
         return d->maximumClickThroughVelocity;

      case AcceleratingFlickMaximumTime:
         return d->acceleratingFlickMaximumTime;

      case AcceleratingFlickSpeedupFactor:
         return d->acceleratingFlickSpeedupFactor;

      case SnapPositionRatio:
         return d->snapPositionRatio;

      case SnapTime:
         return d->snapTime;

      case OvershootDragResistanceFactor:
         return d->overshootDragResistanceFactor;

      case OvershootDragDistanceFactor:
         return d->overshootDragDistanceFactor;

      case OvershootScrollDistanceFactor:
         return d->overshootScrollDistanceFactor;

      case OvershootScrollTime:
         return d->overshootScrollTime;

      case HorizontalOvershootPolicy:
         return QVariant::fromValue(d->hOvershootPolicy);

      case VerticalOvershootPolicy:
         return QVariant::fromValue(d->vOvershootPolicy);

      case FrameRate:
         return QVariant::fromValue(d->frameRate);

      case ScrollMetricCount:
         break;
   }

   return QVariant();
}

void QScrollerProperties::setScrollMetric(ScrollMetric metric, const QVariant &value)
{
   switch (metric) {
      case MousePressEventDelay:
         d->mousePressEventDelay = value.toReal();
         break;

      case DragStartDistance:
         d->dragStartDistance = value.toReal();
         break;

      case DragVelocitySmoothingFactor:
         d->dragVelocitySmoothingFactor = qBound(qreal(0), value.toReal(), qreal(1));
         break;

      case AxisLockThreshold:
         d->axisLockThreshold = qBound(qreal(0), value.toReal(), qreal(1));
         break;

      case ScrollingCurve:
         d->scrollingCurve = value.toEasingCurve();
         break;

      case DecelerationFactor:
         d->decelerationFactor = value.toReal();
         break;

      case MinimumVelocity:
         d->minimumVelocity = value.toReal();
         break;

      case MaximumVelocity:
         d->maximumVelocity = value.toReal();
         break;

      case MaximumClickThroughVelocity:
         d->maximumClickThroughVelocity = value.toReal();
         break;

      case AcceleratingFlickMaximumTime:
         d->acceleratingFlickMaximumTime = value.toReal();
         break;

      case AcceleratingFlickSpeedupFactor:
         d->acceleratingFlickSpeedupFactor = value.toReal();
         break;

      case SnapPositionRatio:
         d->snapPositionRatio = qBound(qreal(0), value.toReal(), qreal(1));
         break;

      case SnapTime:
         d->snapTime = value.toReal();
         break;

      case OvershootDragResistanceFactor:
         d->overshootDragResistanceFactor = value.toReal();
         break;

      case OvershootDragDistanceFactor:
         d->overshootDragDistanceFactor = qBound(qreal(0), value.toReal(), qreal(1));
         break;

      case OvershootScrollDistanceFactor:
         d->overshootScrollDistanceFactor = qBound(qreal(0), value.toReal(), qreal(1));
         break;

      case OvershootScrollTime:
         d->overshootScrollTime = value.toReal();
         break;

      case HorizontalOvershootPolicy:
         d->hOvershootPolicy = value.value<QScrollerProperties::OvershootPolicy>();
         break;

      case VerticalOvershootPolicy:
         d->vOvershootPolicy = value.value<QScrollerProperties::OvershootPolicy>();
         break;

      case FrameRate:
         d->frameRate = value.value<QScrollerProperties::FrameRates>();
         break;

      case ScrollMetricCount:
         break;
   }
}
