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

#include <qvariantanimation.h>
#include <qvariantanimation_p.h>

#ifndef QT_NO_ANIMATION

#include <qcolor.h>
#include <qvector2d.h>
#include <qvector3d.h>
#include <qvector4d.h>
#include <qquaternion.h>

template <>
inline QColor cs_genericFormula(const QColor &from, const QColor &to, double progress)
{
   return QColor( qBound(0, cs_genericFormula(from.red(),   to.red(),   progress), 255),
                  qBound(0, cs_genericFormula(from.green(), to.green(), progress), 255),
                  qBound(0, cs_genericFormula(from.blue(),  to.blue(),  progress), 255),
                  qBound(0, cs_genericFormula(from.alpha(), to.alpha(), progress), 255));
}

template <>
inline QQuaternion cs_genericFormula(const QQuaternion &from, const QQuaternion &to, double progress)
{
   return QQuaternion::slerp(from, to, progress);
}

void cs_addGuiFormulas()
{
   QVariantAnimation::cs_addCustomType<QColor>(cs_variantFormula<QColor>);
   QVariantAnimation::cs_addCustomType<QVector2D>(cs_variantFormula<QVector2D>);
   QVariantAnimation::cs_addCustomType<QVector3D>(cs_variantFormula<QVector3D>);
   QVariantAnimation::cs_addCustomType<QVector4D>(cs_variantFormula<QVector4D>);
   QVariantAnimation::cs_addCustomType<QQuaternion>(cs_variantFormula<QQuaternion>);
}

#endif
