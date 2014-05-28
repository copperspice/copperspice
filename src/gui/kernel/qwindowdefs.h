/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QWINDOWDEFS_H
#define QWINDOWDEFS_H

#include <QtCore/qnamespace.h>

QT_BEGIN_NAMESPACE

class QPaintDevice;
class QWidget;
class QDialog;
class QColor;
class QPalette;
class QCursor;
class QPoint;
class QSize;
class QRect;
class QPolygon;
class QPainter;
class QRegion;
class QFont;
class QFontMetrics;
class QFontInfo;
class QPen;
class QBrush;
class QMatrix;
class QPixmap;
class QBitmap;
class QMovie;
class QImage;
class QPicture;
class QPrinter;
class QTimer;
class QTime;
class QClipboard;
class QString;
class QByteArray;
class QApplication;

template<typename T> class QList;
typedef QList<QWidget *> QWidgetList;

QT_END_NAMESPACE


// Window system dependent definitions

#if defined(Q_OS_MAC) && !defined(Q_WS_QWS)
#include <QtGui/qmacdefines_mac.h>

typedef long WId;

#endif

#if defined(Q_OS_WIN)
#include <QtGui/qwindowdefs_win.h>
#endif

#if defined(Q_WS_X11)
typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;
typedef struct _XGC *GC;
typedef struct _XRegion *Region;
typedef unsigned long  WId;
#endif

#if defined(Q_WS_QWS)
typedef unsigned long  WId;
QT_BEGIN_NAMESPACE
struct QWSEvent;
QT_END_NAMESPACE
#endif

#if defined(Q_WS_QPA)
typedef unsigned long  WId;
#endif

QT_BEGIN_NAMESPACE

template<class K, class V> class QHash;
typedef QHash<WId, QWidget *> QWidgetMapper;

template<class V> class QSet;
typedef QSet<QWidget *> QWidgetSet;

QT_END_NAMESPACE

#if defined(QT_NEEDS_QMAIN)
#define main qMain
#endif

#endif // QWINDOWDEFS_H
