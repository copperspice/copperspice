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

#ifndef Q_AIX_QPLATFORMDEFS_H
#define Q_AIX_QPLATFORMDEFS_H

// Get Qt defines/settings

#include "qglobal.h"

// Set any POSIX/XOPEN defines at the top of this file to turn on specific APIs

#include <unistd.h>


// We are hot - unistd.h should have turned on the specific APIs we requested


// uncomment if you have problems with <sys/proc.h> because your gcc
// hasn't been built on exactly the same OS version your are using now.
// typedef int crid_t;
// typedef unsigned int class_id_t;
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <dlfcn.h>
#include <strings.h> // AIX X11 headers define FD_ZERO using bzero()

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#ifndef QT_NO_IPV6IFNAME
#include <net/if.h>
#endif

// Only AIX 4.3 and better support 64-bit

#define QT_USE_XOPEN_LFS_EXTENSIONS
#include "../posix/qplatformdefs.h"

#undef QT_SOCKLEN_T

#ifdef _AIX43
// AIX 4.3 and better
#define QT_SOCKLEN_T            socklen_t
#elif _AIX42
// AIX 4.2
#define QT_SOCKLEN_T            size_t
#else
// AIX 4.1
#define QT_SOCKLEN_T            size_t
// override
#define QT_SOCKOPTLEN_T         int
#endif

#ifdef QT_LARGEFILE_SUPPORT
#undef QT_DIR
#undef QT_OPENDIR
#undef QT_CLOSEDIR

#define QT_DIR                  DIR64
#define QT_OPENDIR              ::opendir64
#define QT_CLOSEDIR             ::closedir64
#endif

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE-0 >= 500)
// AIX 4.3 and better
#define QT_SNPRINTF             ::snprintf
#define QT_VSNPRINTF            ::vsnprintf
#endif

#endif // include guard
