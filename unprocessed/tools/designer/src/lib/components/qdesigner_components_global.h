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

#ifndef QDESIGNER_COMPONENTS_GLOBAL_H
#define QDESIGNER_COMPONENTS_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

#define QDESIGNER_COMPONENTS_EXTERN Q_DECL_EXPORT
#define QDESIGNER_COMPONENTS_IMPORT Q_DECL_IMPORT

#ifdef QT_DESIGNER_STATIC
#  define QDESIGNER_COMPONENTS_EXPORT
#elif defined(QDESIGNER_COMPONENTS_LIBRARY)
#  define QDESIGNER_COMPONENTS_EXPORT QDESIGNER_COMPONENTS_EXTERN
#else
#  define QDESIGNER_COMPONENTS_EXPORT QDESIGNER_COMPONENTS_IMPORT
#endif


QT_END_NAMESPACE
QT_END_HEADER

#endif // QDESIGNER_COMPONENTS_GLOBAL_H
