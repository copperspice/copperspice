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

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

#include <qglobal.h>

// configure generates the following h file and it contains the defines for have_x_h
#include <cs-config.h>

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SHM_H
#include <sys/shm.h>
#endif

#ifdef HAVE_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif


// ***********

#if defined(Q_OS_WIN)

#if defined(UNICODE) && ! defined(_UNICODE)
#define _UNICODE
#endif

#include <qt_windows.h>

#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <tchar.h>

#if ! defined(_WIN32_WINNT) || (_WIN32_WINNT-0 < 0x0500)

enum EXTENDED_NAME_FORMAT {
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
};
using PEXTENDED_NAME_FORMAT = *EXTENDED_NAME_FORMAT;

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

//  block A
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

#endif  // end block A

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
#define QT_OPEN_TEXT         _O_TEXT
#define QT_OPEN_BINARY       _O_BINARY
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

#define QT_SNPRINTF           ::_snprintf
#define QT_VSNPRINTF          ::_vsnprintf

#define F_OK                  0
#define X_OK                  1
#define W_OK                  2
#define R_OK                  4


// ***********
#elif defined(Q_OS_UNIX) && ! defined(Q_OS_DARWIN)

// may need to reset default environment if _BSD_SOURCE is defined

#define _XOPEN_SOURCE 700

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define QT_USE_XOPEN_LFS_EXTENSIONS
#include <qplatformposix.h>

#undef QT_SOCKLEN_T
#define QT_SOCKLEN_T          socklen_t

#define QT_SNPRINTF           ::snprintf
#define QT_VSNPRINTF          ::vsnprintf


// ***********
#elif defined(Q_OS_DARWIN)

#include <qplatformposix.h>

#undef QT_OPEN_LARGEFILE
#define QT_OPEN_LARGEFILE     0

#undef QT_SOCKLEN_T
#define QT_SOCKLEN_T          socklen_t

#define QT_SNPRINTF           ::snprintf
#define QT_VSNPRINTF          ::vsnprintf


// ***********
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD) || defined(Q_OS_DRAGONFLY)

#include <qplatformposix.h>

#undef QT_OPEN_LARGEFILE
#define QT_OPEN_LARGEFILE    0

#define QT_SNPRINTF          ::snprintf
#define QT_VSNPRINTF         ::vsnprintf

#endif

#endif
