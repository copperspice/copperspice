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

#ifndef QMEMROTATE_P_H
#define QMEMROTATE_P_H

#include <qdrawhelper_p.h>

#define QT_ROTATION_CACHEDREAD 1
#define QT_ROTATION_CACHEDWRITE 2
#define QT_ROTATION_PACKING 3
#define QT_ROTATION_TILED 4

#ifndef QT_ROTATION_ALGORITHM

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#define QT_ROTATION_ALGORITHM QT_ROTATION_TILED
#else
#define QT_ROTATION_ALGORITHM QT_ROTATION_CACHEDREAD
#endif

#endif

#define QT_DECL_MEMROTATE(type)                            \
    void Q_GUI_EXPORT qt_memrotate90(const type*, int, int, int, type*, int); \
    void Q_GUI_EXPORT qt_memrotate180(const type*, int, int, int, type*, int); \
    void Q_GUI_EXPORT qt_memrotate270(const type*, int, int, int, type*, int)

QT_DECL_MEMROTATE(quint32);
QT_DECL_MEMROTATE(quint16);
QT_DECL_MEMROTATE(quint24);
QT_DECL_MEMROTATE(quint8);

#undef QT_DECL_MEMROTATE

#endif
