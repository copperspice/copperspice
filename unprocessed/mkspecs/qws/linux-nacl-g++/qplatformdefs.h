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

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

// Get Qt defines/settings

#define _POSIX_TIMERS

#include "qglobal.h"

// extra disabling.
#ifdef __native_client__
#define QT_NO_FSFILEENGINE
#endif

#define QT_NO_SOCKET_H

#define DIR void *
#define PATH_MAX 256

#include "../../common/posix/qplatformdefs.h"
#include "qfunctions_nacl.h"
#include <pthread.h>

#undef QT_LSTAT
#define QT_LSTAT                QT_STAT


#endif // QPLATFORMDEFS_H
