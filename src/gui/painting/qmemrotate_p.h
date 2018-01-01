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

#ifndef QMEMROTATE_P_H
#define QMEMROTATE_P_H

#include <qdrawhelper_p.h>

QT_BEGIN_NAMESPACE

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

#ifdef Q_WS_QWS
#define Q_GUI_QWS_EXPORT Q_GUI_EXPORT
#else
#define Q_GUI_QWS_EXPORT
#endif

#define QT_DECL_MEMROTATE(srctype, desttype)                            \
    void Q_GUI_QWS_EXPORT qt_memrotate90(const srctype*, int, int, int, desttype*, int); \
    void Q_GUI_QWS_EXPORT qt_memrotate180(const srctype*, int, int, int, desttype*, int); \
    void Q_GUI_QWS_EXPORT qt_memrotate270(const srctype*, int, int, int, desttype*, int)

void Q_GUI_EXPORT qt_memrotate90(const quint32 *, int, int, int, quint32 *, int);
void Q_GUI_QWS_EXPORT qt_memrotate180(const quint32 *, int, int, int, quint32 *, int);
void Q_GUI_QWS_EXPORT qt_memrotate270(const quint32 *, int, int, int, quint32 *, int);

QT_DECL_MEMROTATE(quint32, quint16);
QT_DECL_MEMROTATE(quint16, quint32);
QT_DECL_MEMROTATE(quint16, quint16);
QT_DECL_MEMROTATE(quint24, quint24);
QT_DECL_MEMROTATE(quint32, quint24);
QT_DECL_MEMROTATE(quint32, quint18);
QT_DECL_MEMROTATE(quint32, quint8);
QT_DECL_MEMROTATE(quint16, quint8);
QT_DECL_MEMROTATE(qrgb444, quint8);
QT_DECL_MEMROTATE(quint8, quint8);

#ifdef QT_QWS_ROTATE_BGR
QT_DECL_MEMROTATE(quint16, qbgr565);
QT_DECL_MEMROTATE(quint32, qbgr565);
QT_DECL_MEMROTATE(qrgb555, qbgr555);
QT_DECL_MEMROTATE(quint32, qbgr555);
#endif

#ifdef QT_QWS_DEPTH_GENERIC
QT_DECL_MEMROTATE(quint32, qrgb_generic16);
QT_DECL_MEMROTATE(quint16, qrgb_generic16);
#endif

#undef QT_DECL_MEMROTATE

QT_END_NAMESPACE

#endif // QMEMROTATE_P_H
