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

#include <QtCore/qvariantanimation.h>
#include <qvariantanimation_p.h>

#ifndef QT_NO_ANIMATION

#include <QtGui/qcolor.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

template<> Q_INLINE_TEMPLATE QColor _q_interpolate(const QColor &f, const QColor &t, qreal progress)
{
   return QColor(qBound(0, _q_interpolate(f.red(), t.red(), progress), 255),
                 qBound(0, _q_interpolate(f.green(), t.green(), progress), 255),
                 qBound(0, _q_interpolate(f.blue(), t.blue(), progress), 255),
                 qBound(0, _q_interpolate(f.alpha(), t.alpha(), progress), 255));
}

template<> Q_INLINE_TEMPLATE QQuaternion _q_interpolate(const QQuaternion &f, const QQuaternion &t, qreal progress)
{
   return QQuaternion::slerp(f, t, progress);
}

static int qRegisterGuiGetInterpolator()
{
   qRegisterAnimationInterpolator<QColor>(_q_interpolateVariant<QColor>);
   qRegisterAnimationInterpolator<QVector2D>(_q_interpolateVariant<QVector2D>);
   qRegisterAnimationInterpolator<QVector3D>(_q_interpolateVariant<QVector3D>);
   qRegisterAnimationInterpolator<QVector4D>(_q_interpolateVariant<QVector4D>);
   qRegisterAnimationInterpolator<QQuaternion>(_q_interpolateVariant<QQuaternion>);
   return 1;
}
Q_CONSTRUCTOR_FUNCTION(qRegisterGuiGetInterpolator)

static int qUnregisterGuiGetInterpolator()
{
   // casts required by Sun CC 5.5
   qRegisterAnimationInterpolator<QColor>(
      (QVariant (*)(const QColor &, const QColor &, qreal))0);
   qRegisterAnimationInterpolator<QVector2D>(
      (QVariant (*)(const QVector2D &, const QVector2D &, qreal))0);
   qRegisterAnimationInterpolator<QVector3D>(
      (QVariant (*)(const QVector3D &, const QVector3D &, qreal))0);
   qRegisterAnimationInterpolator<QVector4D>(
      (QVariant (*)(const QVector4D &, const QVector4D &, qreal))0);
   qRegisterAnimationInterpolator<QQuaternion>(
      (QVariant (*)(const QQuaternion &, const QQuaternion &, qreal))0);

   return 1;
}
Q_DESTRUCTOR_FUNCTION(qUnregisterGuiGetInterpolator)

QT_END_NAMESPACE

#endif //QT_NO_ANIMATION
