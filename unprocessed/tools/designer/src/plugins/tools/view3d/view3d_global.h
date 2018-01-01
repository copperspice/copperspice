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

#ifndef VIEW3D_GLOBAL_H
#define VIEW3D_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifdef Q_OS_WIN
#ifdef VIEW3D_LIBRARY
# define VIEW3D_EXPORT
#else
# define VIEW3D_EXPORT
#endif
#else
#define VIEW3D_EXPORT
#endif

#endif // VIEW3D_GLOBAL_H

QT_END_NAMESPACE
