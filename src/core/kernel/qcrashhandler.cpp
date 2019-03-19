/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

/*************************************************************************
 *
 * stacktrace.c 1.2 1998/12/21
 *
 * Copyright (c) 1998 by Bjorn Reese <breese@imada.ou.dk>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 ************************************************************************/

#include <qplatformdefs.h>
#include <qcrashhandler_p.h>
#include <qbytearray.h>
#include <qstring.h>

#ifndef QT_NO_CRASHHANDLER

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

QtCrashHandler QSegfaultHandler::callback = 0;

#if defined(__GLIBC__) && (__GLIBC__ >= 2) && ! defined(__UCLIBC__) && ! defined(QT_LINUXBASE)

#include <execinfo.h>

static void print_backtrace(FILE *outb)
{
   void *stack[128];
   int stack_size = backtrace(stack, sizeof(stack) / sizeof(void *));
   char **stack_symbols = backtrace_symbols(stack, stack_size);

   fprintf(outb, "Stack [%d]:\n", stack_size);

   if (FILE *cppfilt = popen("c++filt", "rw")) {
      dup2(fileno(outb), fileno(cppfilt));
      for (int i = stack_size - 1; i >= 0; --i) {
         fwrite(stack_symbols[i], 1, strlen(stack_symbols[i]), cppfilt);
      }
      pclose(cppfilt);
   } else {
      for (int i = stack_size - 1; i >= 0; --i) {
         fprintf(outb, "#%d  %p [%s]\n", i, stack[i], stack_symbols[i]);
      }
   }
}
static void init_backtrace(char **, int)
{
}

#else /* Don't use the GLIBC callback */
/* Code sourced from: */

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

static char *globalProgName = NULL;
static bool backtrace_command(FILE *outb, const char *format, ...)
{
   bool ret = false;
   char buffer[50];

   va_list args;
   char cmd[512];

   va_start(args, format);
   std:vsnprintf(cmd, 512, format, args);
   va_end(args);

   char *foo = cmd;

   if (FILE *inb = popen(foo, "r")) {
      while (!feof(inb)) {
         int len = fread(buffer, 1, sizeof(buffer), inb);

         if (! len) {
            break;
         }

         if (!ret) {
            fwrite("Output from ", 1, strlen("Output from "), outb);
            strtok(cmd, " ");
            fwrite(cmd, 1, strlen(cmd), outb);
            fwrite("\n", 1, 1, outb);
            ret = true;
         }
         fwrite(buffer, 1, len, outb);
      }

      fclose(inb);
   }

   return ret;
}

static void init_backtrace(char **argv, int argc)
{
   if (argc >= 1) {
      globalProgName = argv[0];
   }
}

static void print_backtrace(FILE *outb)
{
   /*
    * In general dbx seems to do a better job than gdb.
    *
    * Different dbx implementations require different flags/commands.
    */

#if defined(Q_OS_FREEBSD)
   /*
    * FreeBSD insists on sending a SIGSTOP to the process we
    * attach to, so we let the debugger send a SIGCONT to that
    * process after we have detached.
    */
   if (backtrace_command(outb, "gdb -q %s %d 2>/dev/null <<EOF\n"
                         "set prompt\n"
                         "where\n"
                         "detach\n"
                         "shell kill -CONT %d\n"
                         "quit\n"
                         "EOF\n",
                         globalProgName, (int)getpid(), (int)getpid())) {
      return;
   }
#elif defined(Q_OS_HPUX)
   /*
    * HP decided to call their debugger xdb.
    *
    * This does not seem to work properly yet. The debugger says
    * "Note: Stack traces may not be possible until you are
    *  stopped in user code." on HP-UX 09.01
    *
    * -L = line-oriented interface.
    * "T [depth]" gives a stacktrace with local variables.
    * The final "y" is confirmation to the quit command.
    */
   if (backtrace_command(outb, "xdb -P %d -L %s 2>&1 <<EOF\n"
                         "T 50\n"
                         "q\ny\n"
                         "EOF\n",
                         (int)getpid(), globalProgName)) {
      return;
   }
   if (backtrace_command(outb, "gdb -q %s %d 2>/dev/null <<EOF\n"
                         "set prompt\n"
                         "where\n"
                         "detach\n"
                         "quit\n"
                         "EOF\n",
                         globalProgName, (int)getpid())) {
      return;
   }

#elif defined(Q_OS_SOLARIS)
   if (backtrace_command(outb, "dbx %s %d 2>/dev/null <<EOF\n"
                         "where\n"
                         "detach\n"
                         "EOF\n",
                         globalProgName, (int)getpid())) {
      return;
   }
   if (backtrace_command(outb, "gdb -q %s %d 2>/dev/null <<EOF\n"
                         "set prompt\n"
                         "where\n"
                         "echo ---\\n\n"
                         "frame 5\n"      /* Skip signal handler frames */
                         "set \\$x = 50\n"
                         "while (\\$x)\n" /* Print local variables for each frame */
                         "info locals\n"
                         "up\n"
                         "set \\$x--\n"
                         "end\n"
                         "echo ---\\n\n"
                         "detach\n"
                         "quit\n"
                         "EOF\n",
                         globalProgName, (int)getpid())) {
      return;
   }
   if (backtrace_command(outb, "/usr/proc/bin/pstack %d",
                         (int)getpid())) {
      return;
   }
   /*
    * Other Unices (AIX, HPUX, SCO) also have adb, but
    * they seem unable to attach to a running process.)
    */
   if (backtrace_command(outb, "adb %s 2>&1 <<EOF\n"
                         "0t%d:A\n" /* Attach to pid */
                         "\\$c\n"   /* print stacktrace */
                         ":R\n"     /* Detach */
                         "\\$q\n"   /* Quit */
                         "EOF\n",
                         globalProgName, (int)getpid())) {
      return;
   }

#else /* All other platforms */
   /*
    * TODO: SCO/UnixWare 7 must be something like (not tested)
    *  debug -i c <pid> <<EOF\nstack -f 4\nquit\nEOF\n
    */
# if !defined(__GNUC__)
   if (backtrace_command(outb, "dbx %s %d 2>/dev/null <<EOF\n"
                         "where\n"
                         "detach\n"
                         "EOF\n",
                         globalProgName, (int)getpid())) {
      return;
   }
# endif
   if (backtrace_command(outb, "gdb -q %s %d 2>/dev/null <<EOF\n"
                         "set prompt\n"
                         "where\n"
                         "detach\n"
                         "quit\n"
                         "EOF\n",
                         globalProgName, (int)getpid())) {
      return;
   }
#endif
   const char debug_err[] = "No debugger found\n";
   fwrite(debug_err, strlen(debug_err), 1, outb);
}
/* end of copied code */
#endif


void qt_signal_handler(int sig)
{
   signal(sig, SIG_DFL);
   if (QSegfaultHandler::callback) {
      (*QSegfaultHandler::callback)();
      _exit(1);
   }

   FILE *outb = stderr;
   if (char *crash_loc = ::getenv("QT_CRASH_OUTPUT")) {
      if (FILE *new_outb = fopen(crash_loc, "w")) {
         fprintf(stderr, "Crash (backtrace written to %s)\n", crash_loc);
         outb = new_outb;
      }
   } else {
      fprintf(outb, "Crash\n");
   }

   print_backtrace(outb);

   if (outb != stderr) {
      fclose(outb);
   }

   _exit(1);
}


void
QSegfaultHandler::initialize(char **argv, int argc)
{
   init_backtrace(argv, argc);

   struct sigaction SignalAction;
   SignalAction.sa_flags = 0;
   SignalAction.sa_handler = qt_signal_handler;
   sigemptyset(&SignalAction.sa_mask);
   sigaction(SIGSEGV, &SignalAction, NULL);
   sigaction(SIGBUS, &SignalAction, NULL);
}

#endif // QT_NO_CRASHHANDLER
