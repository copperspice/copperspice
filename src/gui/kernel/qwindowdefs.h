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

#ifndef QWINDOWDEFS_H
#define QWINDOWDEFS_H

#include <qglobal.h>
#include <qnamespace.h>
#include <qcontainerfwd.h>
#include <qstringfwd.h>

class QPaintDevice;
class QWidget;
class QWindow;
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

class QTimer;
class QTime;
class QClipboard;
class QByteArray;
class QApplication;


// Window system dependent definitions
#if defined(Q_OS_WIN)
#include <qwindowdefs_win.h>
#endif


using WId = quintptr;

using QWidgetMapper = QHash<WId, QWidget *>;
using QWidgetList   = QList<QWidget *>;
using QWidgetSet    = QSet<QWidget *>;
using QWindowList   = QList<QWindow *>;

#if defined(QT_NEEDS_QMAIN)
#define main qMain
#endif

#endif
