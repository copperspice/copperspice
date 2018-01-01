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

#ifndef QNUMERIC_H
#define QNUMERIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

Q_CORE_EXPORT bool qIsInf(double d);
Q_CORE_EXPORT bool qIsNaN(double d);
Q_CORE_EXPORT bool qIsFinite(double d);
Q_CORE_EXPORT bool qIsInf(float f);
Q_CORE_EXPORT bool qIsNaN(float f);
Q_CORE_EXPORT bool qIsFinite(float f);
Q_CORE_EXPORT double qSNaN();
Q_CORE_EXPORT double qQNaN();
Q_CORE_EXPORT double qInf();

#define Q_INFINITY (QT_PREPEND_NAMESPACE(qInf)())
#define Q_SNAN (QT_PREPEND_NAMESPACE(qSNaN)())
#define Q_QNAN (QT_PREPEND_NAMESPACE(qQNaN)())

QT_END_NAMESPACE

#endif // QNUMERIC_H
