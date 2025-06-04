/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QCORECMDLINEARGS_P_H
#define QCORECMDLINEARGS_P_H

#include <qstring.h>
#include <qstringlist.h>
#include <qvector.h>

#if defined(Q_OS_WIN)

#include <qt_windows.h>

template <typename CharType>
static QVector<CharType *> qWinCmdLine(CharType *cmdParam, int length, int &argc)
{
   // called from qcoreapplication_win.cpp and the tools (rcc, uic, etc)

   QVector<CharType *> retval;

   CharType *p     = cmdParam;
   CharType *p_end = p + length;

   while (*p && p < p_end) {
      // parse cmd line arguments

      while (QChar((char16_t)(*p)).isSpace()) {
         // skip white space
         p++;
      }

      if (*p && p < p_end) {
         int quote;

         CharType *start;
         CharType *r;

         if (*p == CharType('\"') || *p == CharType('\'')) {
            // " or ' quote

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
                  ++p;

                  if (QChar((char16_t)(*p)).isSpace()) {
                     break;
                  }

                  quote = 0;
               }
            }

            if (*p == '\\') {
               // escape char?

               if (*(p + 1) == quote) {
                  ++p;
               }

            } else {
               if (! quote && (*p == CharType('\"') || *p == CharType('\''))) {
                  // " or ' quote
                  quote = *p++;
                  continue;

               } else if (QChar((char16_t)(*p)).isSpace() && ! quote) {
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

         *r = CharType('\0');

         retval.append(start);
      }
   }

   retval.append(nullptr);
   argc = retval.size() - 1;

   return retval;
}

static inline QStringList qCmdLineArgs(int, char *[])
{
   QStringList argList;
   int argc = 0;

   std::wstring tmp(GetCommandLine());
   QVector<wchar_t *> argv = qWinCmdLine<wchar_t>(&tmp[0], tmp.length(), argc);

   for (int index = 0; index < argc; ++index) {
      argList.append(QString::fromStdWString(std::wstring(argv[index])));
   }

   return argList;
}

#else
// platforms other than windows

static inline QStringList qCmdLineArgs(int argc, char *argv[])
{
   QStringList argList;

   for (int index = 0; index < argc; ++index) {
      argList.append(QString::fromUtf8(argv[index]));
   }

   return argList;
}

#endif

#endif
