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

#ifndef QGTKGLOBAL_P_H
#define QGTKGLOBAL_P_H


#include <qglobal.h>

#if !defined(QT_NO_STYLE_GTK)

#undef signals // Collides with GTK symbols

#include <gtk/gtk.h>

typedef unsigned long XID;

#undef GTK_OBJECT_FLAGS
#define GTK_OBJECT_FLAGS(obj)(((GtkObject*)(obj))->flags)

#define QLS(x) QLatin1String(x)

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
#   define QT_RED 3
#   define QT_GREEN 2
#   define QT_BLUE 1
#   define QT_ALPHA 0
#else
#   define QT_RED 0
#   define QT_GREEN 1
#   define QT_BLUE 2
#   define QT_ALPHA 3
#endif
#   define GTK_RED 2
#   define GTK_GREEN 1
#   define GTK_BLUE 0
#   define GTK_ALPHA 3

#endif // !QT_NO_STYLE_GTK
#endif
