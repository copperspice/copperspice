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

#ifndef GSTREAMER_COMMON_H
#define GSTREAMER_COMMON_H

#include <phonon/phonon_export.h>
#include <QtCore/QDebug>

#ifdef DEBUG_IMPLEMENTED
#define IMPLEMENTED qDebug() << "PGst:" << __FUNCTION__ << "(" << __FILE__ << "):"
#else
#define IMPLEMENTED if (1); else qDebug()
#endif

#ifdef DEBUG_HALF_IMPLEMENTED
#define HALF_IMPLEMENTED qDebug() << "PGst: --- HALF IMPLEMENTED:" << __FUNCTION__ << "(" << __FILE__ << "):"
#else
#define HALF_IMPLEMENTED if (1); else qDebug()
#endif

#ifdef DEBUG_NOT_IMPLEMENTED
#define NOT_IMPLEMENTED qDebug() << "PGst: *** NOT IMPLEMENTED:" << __FUNCTION__ << "(" << __FILE__ << "):"
#else
#define NOT_IMPLEMENTED if (1); else qDebug()
#endif

#ifdef DEBUG_IMPLEMENTED_SILENT
#define IMPLEMENTED_SILENT qDebug() << "PGst: (silent)" << __FUNCTION__ << "(" << __FILE__ << "):"
#else
#define IMPLEMENTED_SILENT if (1); else qDebug()
#endif

#define BACKEND_WARNING qDebug() << "PGst: ____ WARNING ____" << Q_FUNC_INFO << ":"

#endif // Phonon_GSTREAMER_COMMON_H
