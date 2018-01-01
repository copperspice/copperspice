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

#ifndef QCORECMDLINEARGS_P_H
#define QCORECMDLINEARGS_P_H

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

#if defined(Q_OS_WIN32)

QT_BEGIN_INCLUDE_NAMESPACE
# include <QtCore/qvector.h>
# if defined(Q_OS_WIN32)
#  include <qt_windows.h>
# endif
QT_END_INCLUDE_NAMESPACE

// template implementation of the parsing algorithm
// this is used from qcoreapplication_win.cpp and the tools (rcc, uic...)

template<typename Char>
static QVector<Char *> qWinCmdLine(Char *cmdParam, int length, int &argc)
{
   QVector<Char *> argv(8);
   Char *p = cmdParam;
   Char *p_end = p + length;

   argc = 0;

   while (*p && p < p_end) {                                // parse cmd line arguments
      while (QChar((short)(*p)).isSpace()) {                // skip white space
         p++;
      }
      if (*p && p < p_end) {                                // arg starts
         int quote;
         Char *start, *r;
         if (*p == Char('\"') || *p == Char('\'')) {        // " or ' quote
            quote = *p;
            start = ++p;
         } else {
            quote = 0;
            start = p;
         }
         r = start;
         while (*p && p < p_end) {
            if (quote) {
               if (*p == quote) {
                  p++;
                  if (QChar((short)(*p)).isSpace()) {
                     break;
                  }
                  quote = 0;
               }
            }
            if (*p == '\\') {                // escape char?
               if (*(p + 1) == quote) {
                  p++;
               }
            } else {
               if (!quote && (*p == Char('\"') || *p == Char('\''))) {        // " or ' quote
                  quote = *p++;
                  continue;
               } else if (QChar((short)(*p)).isSpace() && !quote) {
                  break;
               }
            }
            if (*p) {
               *r++ = *p++;
            }
         }
         if (*p && p < p_end) {
            p++;
         }
         *r = Char('\0');

         if (argc >= (int)argv.size() - 1) {    // expand array
            argv.resize(argv.size() * 2);
         }
         argv[argc++] = start;
      }
   }
   argv[argc] = 0;

   return argv;
}

#if defined(Q_OS_WIN32)
static inline QStringList qWinCmdArgs(QString cmdLine) // not const-ref: this might be modified
{
   QStringList args;

   int argc = 0;
   QVector<wchar_t *> argv = qWinCmdLine<wchar_t>((wchar_t *)cmdLine.utf16(), cmdLine.length(), argc);
   for (int a = 0; a < argc; ++a) {
      args << QString::fromWCharArray(argv[a]);
   }

   return args;
}

static inline QStringList qCmdLineArgs(int argc, char *argv[])
{
   Q_UNUSED(argc)
   Q_UNUSED(argv)
   QString cmdLine = QString::fromWCharArray(GetCommandLine());
   return qWinCmdArgs(cmdLine);
}
#endif

#else 

static inline QStringList qCmdLineArgs(int argc, char *argv[])
{
   QStringList args;
   for (int i = 0; i != argc; ++i) {
      args += QString::fromLocal8Bit(argv[i]);
   }
   return args;
}

#endif // Q_OS_WIN

QT_END_NAMESPACE

#endif // QCORECMDLINEARGS_WIN_P_H
