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
#include <qglobal.h>

// Set any POSIX/XOPEN defines at the top of this file to turn on specific APIs

#include <unistd.h>

// We are hot - unistd.h should have turned on the specific APIs we requested

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
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <netinet/in.h>
#ifndef QT_NO_IPV6IFNAME
#include <net/if.h>
#endif

// for htonl
#include <arpa/inet.h>

#define QT_USE_XOPEN_LFS_EXTENSIONS
#define QT_NO_READDIR64
#include "../posix/qplatformdefs.h"

#include <stdlib.h>

#define QT_SNPRINTF             ::snprintf
#define QT_VSNPRINTF            ::vsnprintf


#include <sys/neutrino.h>

#if !defined(_NTO_VERSION) || _NTO_VERSION < 650
// pre-6.5 versions of QNX doesn't have getpagesize()
inline int getpagesize()
{
    return ::sysconf(_SC_PAGESIZE);
}

// pre-6.5 versions of QNX doesn't have strtof()
inline float strtof(const char *b, char **e)
{
    return float(strtod(b, e));
}
#endif

#define QT_QWS_TEMP_DIR QString::fromLocal8Bit(qgetenv("TMPDIR").constData())

#endif // QPLATFORMDEFS_H
