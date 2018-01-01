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

#include "qglobal.h"

// Set any POSIX/XOPEN defines at the top of this file to turn on specific APIs

// 1) need to reset default environment if _BSD_SOURCE is defined
// 2) need to specify POSIX thread interfaces explicitly in glibc 2.0
// 3) it seems older glibc need this to include the X/Open stuff
#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <unistd.h>


// We are hot - unistd.h should have turned on the specific APIs we requested
#ifdef HAVE_FEATURES_H
#include <features.h>
#endif

#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#ifndef QT_NO_IPV6IFNAME
#include <net/if.h>
#endif

#define QT_USE_XOPEN_LFS_EXTENSIONS
#include "../common/posix/qplatformdefs.h"

#undef QT_SOCKLEN_T

#if defined(__GLIBC__) && (__GLIBC__ >= 2)
#define QT_SOCKLEN_T            socklen_t
#else
#define QT_SOCKLEN_T            int
#endif

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)
#define QT_SNPRINTF		::snprintf
#define QT_VSNPRINTF		::vsnprintf
#endif

#endif // QPLATFORMDEFS_H
