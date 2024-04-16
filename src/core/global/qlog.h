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

#ifndef QLOG_H
#define QLOG_H

#include <qexport.h>
#include <qstring8.h>

class QDebug;
class QNoDebug;

// enable gcc warnings for printf-style functions
#if defined(Q_CC_GNU) && ! defined(__INSURE__)

#  if defined(Q_CC_MINGW) && ! defined(Q_CC_CLANG)
#    define Q_ATTRIBUTE_FORMAT_PRINTF(A, B) \
        __attribute__((format(gnu_printf, (A), (B))))
#  else
#    define Q_ATTRIBUTE_FORMAT_PRINTF(A, B) \
        __attribute__((format(printf, (A), (B))))
#  endif

#else
#  define Q_ATTRIBUTE_FORMAT_PRINTF(A, B)

#endif

enum QtMsgType {
   QtDebugMsg,
   QtWarningMsg,
   QtCriticalMsg,
   QtFatalMsg,
   QtSystemMsg = QtCriticalMsg
};

using QtMsgHandler = void (*)(QtMsgType, QStringView);

Q_CORE_EXPORT QtMsgHandler csInstallMsgHandler(QtMsgHandler handler);
Q_CORE_EXPORT QtMsgHandler qInstallMsgHandler(QtMsgHandler handler);

Q_CORE_EXPORT QString      qt_error_string(int errorCode = -1);
Q_CORE_EXPORT void         qt_message_output(QtMsgType, QStringView str);
Q_CORE_EXPORT void         qErrnoWarning(int code, const char *msg, ...);
Q_CORE_EXPORT void         qErrnoWarning(const char *msg, ...);

Q_CORE_EXPORT void qDebug(const char *msg, ...)     Q_ATTRIBUTE_FORMAT_PRINTF(1,2);
Q_CORE_EXPORT void qCritical(const char *msg, ...)  Q_ATTRIBUTE_FORMAT_PRINTF(1,2);
Q_CORE_EXPORT void qFatal(const char *msg, ...)     Q_ATTRIBUTE_FORMAT_PRINTF(1,2);
Q_CORE_EXPORT void qWarning(const char *msg, ...)   Q_ATTRIBUTE_FORMAT_PRINTF(1,2);

// forward declarations
inline QDebug qDebug();
inline QDebug qCritical();
inline QDebug qWarning();

#define QT_NO_QDEBUG_MACRO   while (false) qDebug
#define QT_NO_QWARNING_MACRO while (false) qWarning

#endif
