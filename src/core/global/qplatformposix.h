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

#ifndef Q_POSIX_QPLATFORMDEFS_H
#define Q_POSIX_QPLATFORMDEFS_H

#if defined(QT_USE_XOPEN_LFS_EXTENSIONS) && defined(QT_LARGEFILE_SUPPORT)

#define QT_STATBUF              struct stat64
#define QT_FPOS_T               fpos64_t
#define QT_OFF_T                off64_t

#define QT_STAT                 ::stat64
#define QT_LSTAT                ::lstat64
#define QT_TRUNCATE             ::truncate64

// File I/O
#define QT_OPEN                 ::open64
#define QT_LSEEK                ::lseek64
#define QT_FSTAT                ::fstat64
#define QT_FTRUNCATE            ::ftruncate64

// Standard C89
#define QT_FOPEN                ::fopen64
#define QT_FSEEK                ::fseeko64
#define QT_FTELL                ::ftello64
#define QT_FGETPOS              ::fgetpos64
#define QT_FSETPOS              ::fsetpos64

#define QT_MMAP                 ::mmap64

#else 

#define QT_FPOS_T               fpos_t
#define QT_OFF_T                long

#define QT_FOPEN                ::fopen
#define QT_FSEEK                ::fseek
#define QT_FTELL                ::ftell
#define QT_FGETPOS              ::fgetpos
#define QT_FSETPOS              ::fsetpos

#define QT_STATBUF              struct stat

#define QT_STAT                 ::stat
#define QT_LSTAT                ::lstat
#define QT_TRUNCATE             ::truncate

// File I/O
#define QT_OPEN                 ::open
#define QT_LSEEK                ::lseek
#define QT_FSTAT                ::fstat
#define QT_FTRUNCATE            ::ftruncate

// Posix extensions to C89
#if ! defined(QT_USE_XOPEN_LFS_EXTENSIONS) && ! defined(QT_NO_USE_FSEEKO)

#undef QT_OFF_T
#define QT_OFF_T                off_t

#undef QT_FSEEK
#define QT_FSEEK                ::fseeko

#undef QT_FTELL
#define QT_FTELL                ::ftello

#endif

#define QT_MMAP                 ::mmap

#endif

#define QT_STAT_MASK            S_IFMT
#define QT_STAT_REG             S_IFREG
#define QT_STAT_DIR             S_IFDIR
#define QT_STAT_LNK             S_IFLNK

#define QT_ACCESS               ::access
#define QT_GETCWD               ::getcwd
#define QT_CHDIR                ::chdir
#define QT_MKDIR                ::mkdir
#define QT_RMDIR                ::rmdir

// File I/O
#define QT_CLOSE                ::close
#define QT_READ                 ::read
#define QT_WRITE                ::write

#define QT_OPEN_LARGEFILE       0
#define QT_OPEN_RDONLY          O_RDONLY
#define QT_OPEN_WRONLY          O_WRONLY
#define QT_OPEN_RDWR            O_RDWR
#define QT_OPEN_CREAT           O_CREAT
#define QT_OPEN_TRUNC           O_TRUNC
#define QT_OPEN_APPEND          O_APPEND

// Posix extensions to C89
#define QT_FILENO               fileno

// Directory iteration
#define QT_DIR                  DIR

#define QT_OPENDIR              ::opendir
#define QT_CLOSEDIR             ::closedir

#if defined(QT_LARGEFILE_SUPPORT) && defined(QT_USE_XOPEN_LFS_EXTENSIONS) && ! defined(QT_NO_READDIR64)
#define QT_DIRENT               struct dirent64
#define QT_READDIR              ::readdir64
#define QT_READDIR_R            ::readdir64_r
#else
#define QT_DIRENT               struct dirent
#define QT_READDIR              ::readdir
#define QT_READDIR_R            ::readdir_r
#endif

#define QT_SOCKLEN_T            socklen_t

#define QT_SOCKET_CONNECT       ::connect
#define QT_SOCKET_BIND          ::bind

#define QT_SIGNAL_RETTYPE       void
#define QT_SIGNAL_ARGS          int
#define QT_SIGNAL_IGNORE        SIG_IGN

#endif
