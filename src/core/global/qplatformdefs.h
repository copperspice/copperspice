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

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

#include <qglobal.h>

// ***********

#if defined(Q_OS_WIN)

#ifdef UNICODE

#ifndef _UNICODE
#define _UNICODE
#endif

#endif

#include <tchar.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <qt_windows.h>
#include <limits.h>

#if ! defined(_WIN32_WINNT) || (_WIN32_WINNT-0 < 0x0500)

typedef enum {
    NameUnknown           = 0, 
    NameFullyQualifiedDN  = 1, 
    NameSamCompatible     = 2, 
    NameDisplay           = 3, 
    NameUniqueId          = 6, 
    NameCanonical         = 7, 
    NameUserPrincipal     = 8, 
    NameCanonicalEx       = 9, 
    NameServicePrincipal  = 10, 
    NameDnsDomain         = 12
} EXTENDED_NAME_FORMAT, *PEXTENDED_NAME_FORMAT;
#endif

#ifdef QT_LARGEFILE_SUPPORT

#define QT_STATBUF            struct _stati64      // non-ANSI defs
#define QT_STATBUF4TSTAT      struct _stati64      // non-ANSI defs
#define QT_STAT               ::_stati64
#define QT_FSTAT              ::_fstati64

#else
#define QT_STATBUF            struct _stat         // non-ANSI defs
#define QT_STATBUF4TSTAT      struct _stat         // non-ANSI defs
#define QT_STAT               ::_stat
#define QT_FSTAT              ::_fstat

#endif

#define QT_STAT_REG           _S_IFREG
#define QT_STAT_DIR           _S_IFDIR
#define QT_STAT_MASK          _S_IFMT

#if defined(_S_IFLNK)
#define QT_STAT_LNK           _S_IFLNK
#endif

#define QT_FILENO             _fileno
#define QT_OPEN               ::_open
#define QT_CLOSE              ::_close

#ifdef QT_LARGEFILE_SUPPORT
#define QT_LSEEK              ::_lseeki64

#ifndef UNICODE
#define QT_TSTAT              ::_stati64
#else
#define QT_TSTAT              ::_wstati64
#endif

#else
#define QT_LSEEK              ::_lseek

#ifndef UNICODE
#define QT_TSTAT              ::_stat
#else
#define QT_TSTAT              ::_wstat
#endif

#endif

#define QT_READ               ::_read
#define QT_WRITE              ::_write
#define QT_ACCESS             ::_access
#define QT_GETCWD             ::_getcwd
#define QT_CHDIR              ::_chdir
#define QT_MKDIR              ::_mkdir
#define QT_RMDIR              ::_rmdir

#define QT_OPEN_LARGEFILE     0
#define QT_OPEN_RDONLY        _O_RDONLY
#define QT_OPEN_WRONLY        _O_WRONLY
#define QT_OPEN_RDWR          _O_RDWR
#define QT_OPEN_CREAT         _O_CREAT
#define QT_OPEN_TRUNC         _O_TRUNC
#define QT_OPEN_APPEND        _O_APPEND

#if defined(O_TEXT)
# define QT_OPEN_TEXT         _O_TEXT
# define QT_OPEN_BINARY       _O_BINARY
#endif

#define QT_FPOS_T              fpos_t
#define QT_OFF_T               long

#define QT_FOPEN               ::fopen
#define QT_FSEEK               ::fseek
#define QT_FTELL               ::ftell
#define QT_FGETPOS             ::fgetpos
#define QT_FSETPOS             ::fsetpos

#ifdef QT_LARGEFILE_SUPPORT
#undef QT_FSEEK
#undef QT_FTELL
#undef QT_OFF_T

#define QT_FSEEK              ::fseeko64
#define QT_FTELL              ::ftello64
#define QT_OFF_T              off64_t
#endif

#define QT_VSNPRINTF          ::_vsnprintf
#define QT_SNPRINTF           ::_snprintf

#define F_OK                  0
#define X_OK                  1
#define W_OK                  2
#define R_OK                  4


// ***********
#elif defined(Q_OS_UNIX)

// may need to reset default environment if _BSD_SOURCE is defined

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>

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
#include <qplatformposix.h>

#undef QT_SOCKLEN_T
#define QT_SOCKLEN_T          socklen_t

#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)
#define QT_SNPRINTF           ::snprintf
#define QT_VSNPRINTF          ::vsnprintf
#endif


// ***********
#elif defined(Q_OS_MAC)

#include <unistd.h>

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

#include <qplatformposix.h>

#undef QT_OPEN_LARGEFILE
#undef QT_SOCKLEN_T
                              
#define QT_OPEN_LARGEFILE     0
#define QT_SOCKLEN_T          socklen_t

#define QT_SNPRINTF           ::snprintf
#define QT_VSNPRINTF          ::vsnprintf


// ***********
#elif defined(Q_OS_FREEBSD)

#include <unistd.h>

#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <dlfcn.h>

#include <sys/param.h>
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

#include <qplatformposix.h>

#undef QT_OPEN_LARGEFILE
#define QT_OPEN_LARGEFILE    0

#define QT_SNPRINTF          ::snprintf
#define QT_VSNPRINTF         ::vsnprintf

#endif

#endif
