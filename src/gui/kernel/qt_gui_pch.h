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

#ifndef Qt_Gui_Pch_H
#define Qt_Gui_Pch_H

// from core/global/qt_pch.h
#if defined __cplusplus
#include <qglobal.h>

#ifdef Q_OS_WIN
# define _POSIX_
# include <limits.h>
# undef _POSIX_
#endif

#include <qcoreapplication.h>
#include <qlist.h>
#include <qvariant.h>
#include <qobject.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qwidget.h>

#include <stdlib.h>

#endif

#endif
