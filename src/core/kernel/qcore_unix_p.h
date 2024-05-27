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

#ifndef QCORE_UNIX_P_H
#define QCORE_UNIX_P_H

#include <qatomic.h>
#include <qbytearray.h>
#include <qhashfunc.h>
#include <qplatformdefs.h>

#ifndef Q_OS_UNIX
# error <qcore_unix_p.h included on a non-Unix system>
#endif

#include <atomic>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

struct sockaddr;

#define EINTR_LOOP(var, cmd)                  \
   do {                                       \
      var = cmd;                              \
   } while (var == -1 && errno == EINTR)

// Internal operator functions for timespecs
inline timespec &normalizedTimespec(timespec &t)
{
   while (t.tv_nsec >= 1000000000) {
      ++t.tv_sec;
      t.tv_nsec -= 1000000000;
   }

   while (t.tv_nsec < 0) {
      --t.tv_sec;
      t.tv_nsec += 1000000000;
   }

   return t;
}

inline timeval &normalizedTimeval(timeval &t)
{
   while (t.tv_usec >= 1000000) {
      ++t.tv_sec;
      t.tv_usec -= 1000000;
   }

   while (t.tv_usec < 0) {
      --t.tv_sec;
      t.tv_usec += 1000000;
   }

   return t;
}

inline bool operator<(const timespec &t1, const timespec &t2)
{
   return t1.tv_sec < t2.tv_sec || (t1.tv_sec == t2.tv_sec && t1.tv_nsec < t2.tv_nsec);
}

inline bool operator==(const timespec &t1, const timespec &t2)
{
   return t1.tv_sec == t2.tv_sec && t1.tv_nsec == t2.tv_nsec;
}

inline bool operator!=(const timespec &t1, const timespec &t2)
{
   return !(t1 == t2);
}

inline timespec &operator+=(timespec &t1, const timespec &t2)
{
   t1.tv_sec += t2.tv_sec;
   t1.tv_nsec += t2.tv_nsec;
   return normalizedTimespec(t1);
}

inline timespec operator+(const timespec &t1, const timespec &t2)
{
   timespec tmp;
   tmp.tv_sec = t1.tv_sec + t2.tv_sec;
   tmp.tv_nsec = t1.tv_nsec + t2.tv_nsec;
   return normalizedTimespec(tmp);
}

inline timespec operator-(const timespec &t1, const timespec &t2)
{
   timespec tmp;
   tmp.tv_sec = t1.tv_sec - (t2.tv_sec - 1);
   tmp.tv_nsec = t1.tv_nsec - (t2.tv_nsec + 1000000000);
   return normalizedTimespec(tmp);
}

inline timespec operator*(const timespec &t1, int mul)
{
   timespec tmp;
   tmp.tv_sec = t1.tv_sec * mul;
   tmp.tv_nsec = t1.tv_nsec * mul;
   return normalizedTimespec(tmp);
}

inline void qt_ignore_sigpipe()
{
   // Set to ignore SIGPIPE once only.
   static std::atomic<bool> atom(false);

   if (! atom.load()) {
      // More than one thread could turn off SIGPIPE at the same time
      // But that's acceptable because they all would be doing the same action
      struct sigaction noaction;
      memset(&noaction, 0, sizeof(noaction));

      noaction.sa_handler = SIG_IGN;
      ::sigaction(SIGPIPE, &noaction, nullptr);

      atom.store(true);
   }
}

// do not call QT_OPEN or ::open
// call qt_safe_open
static inline int qt_safe_open(const char *pathname, int flags, mode_t mode = 0777)
{
#ifdef O_CLOEXEC
   flags |= O_CLOEXEC;
#endif

   int fd;
   EINTR_LOOP(fd, QT_OPEN(pathname, flags, mode));

   // unknown flags are ignored, so we have no way of verifying if
   // O_CLOEXEC was accepted
   if (fd != -1) {
      ::fcntl(fd, F_SETFD, FD_CLOEXEC);
   }

   return fd;
}

#undef QT_OPEN
#define QT_OPEN  qt_safe_open

static inline int qt_safe_pipe(int pipefd[2], int flags = 0)
{
   Q_ASSERT((flags & ~O_NONBLOCK) == 0);

#ifdef QT_THREADSAFE_CLOEXEC
   // use pipe2
   flags |= O_CLOEXEC;
   return ::pipe2(pipefd, flags); // pipe2 is documented not to return EINTR
#else
   int ret = ::pipe(pipefd);

   if (ret == -1) {
      return -1;
   }

   ::fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
   ::fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);

   // set non-block too?
   if (flags & O_NONBLOCK) {
      ::fcntl(pipefd[0], F_SETFL, ::fcntl(pipefd[0], F_GETFL) | O_NONBLOCK);
      ::fcntl(pipefd[1], F_SETFL, ::fcntl(pipefd[1], F_GETFL) | O_NONBLOCK);
   }

   return 0;
#endif
}

// don't call dup or fcntl(F_DUPFD)
static inline int qt_safe_dup(int oldfd, int atleast = 0, int flags = FD_CLOEXEC)
{
   Q_ASSERT(flags == FD_CLOEXEC || flags == 0);

#ifdef F_DUPFD_CLOEXEC
   int cmd = F_DUPFD;

   if (flags & FD_CLOEXEC) {
      cmd = F_DUPFD_CLOEXEC;
   }

   return ::fcntl(oldfd, cmd, atleast);

#else
   // use F_DUPFD
   int ret = ::fcntl(oldfd, F_DUPFD, atleast);

   if (flags && ret != -1) {
      ::fcntl(ret, F_SETFD, flags);
   }

   return ret;
#endif
}

// do not call dup2, call qt_safe_dup2
static inline int qt_safe_dup2(int oldfd, int newfd, int flags = FD_CLOEXEC)
{
   Q_ASSERT(flags == FD_CLOEXEC || flags == 0);

   int ret;

#ifdef QT_THREADSAFE_CLOEXEC
   // use dup3
   EINTR_LOOP(ret, ::dup3(oldfd, newfd, flags ? O_CLOEXEC : 0));
   return ret;
#else

   EINTR_LOOP(ret, ::dup2(oldfd, newfd));

   if (ret == -1) {
      return -1;
   }

   if (flags) {
      ::fcntl(newfd, F_SETFD, flags);
   }

   return 0;
#endif
}

static inline qint64 qt_safe_read(int fd, void *data, qint64 maxlen)
{
   qint64 ret = 0;
   EINTR_LOOP(ret, QT_READ(fd, data, maxlen));
   return ret;
}

#undef QT_READ
#define QT_READ qt_safe_read

static inline qint64 qt_safe_write(int fd, const void *data, qint64 len)
{
   qint64 ret = 0;
   EINTR_LOOP(ret, QT_WRITE(fd, data, len));
   return ret;
}

#undef QT_WRITE
#define QT_WRITE qt_safe_write

static inline qint64 qt_safe_write_nosignal(int fd, const void *data, qint64 len)
{
   qt_ignore_sigpipe();
   return qt_safe_write(fd, data, len);
}

static inline int qt_safe_close(int fd)
{
   int ret;
   EINTR_LOOP(ret, QT_CLOSE(fd));
   return ret;
}

#undef QT_CLOSE
#define QT_CLOSE qt_safe_close

static inline int qt_safe_execve(const char *filename, char *const argv[],
      char *const envp[])
{
   int ret;
   EINTR_LOOP(ret, ::execve(filename, argv, envp));
   return ret;
}

static inline int qt_safe_execv(const char *path, char *const argv[])
{
   int ret;
   EINTR_LOOP(ret, ::execv(path, argv));
   return ret;
}

static inline int qt_safe_execvp(const char *file, char *const argv[])
{
   int ret;
   EINTR_LOOP(ret, ::execvp(file, argv));
   return ret;
}

static inline pid_t qt_safe_waitpid(pid_t pid, int *status, int options)
{
   int ret;
   EINTR_LOOP(ret, ::waitpid(pid, status, options));
   return ret;
}

#if ! defined(_POSIX_MONOTONIC_CLOCK)
#  define _POSIX_MONOTONIC_CLOCK -1
#endif

// in qelapsedtimer_mac.cpp or qtimestamp_unix.cpp
timespec qt_gettime();

void qt_nanosleep(timespec amount);

Q_CORE_EXPORT int qt_safe_select(int nfds, fd_set *fdread, fd_set *fdwrite, fd_set *fdexcept,
      const struct timespec *tv);

int qt_select_msecs(int nfds, fd_set *fdread, fd_set *fdwrite, int timeout);

// according to X/OPEN we have to define semun ourselves
// we use prefix as on some systems sem.h will have it
struct semid_ds;
union qt_semun {
   int val;                    /* value for SETVAL */
   struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
   unsigned short *array;      /* array for GETALL, SETALL */
};

#ifndef QT_POSIX_IPC
#ifndef QT_NO_SHAREDMEMORY

static inline key_t qt_safe_ftok(const QByteArray &filename, int proj_id)
{
   // Unfortunately ftok can return colliding keys even for different files.
   // Try to add some more entropy via qHash.
   return ::ftok(filename.constData(), qHash(filename, proj_id));
}

#endif // QT_NO_SHAREDMEMORY
#endif // QT_POSIX_IPC

#endif
