/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QFORMAT_H
#define QFORMAT_H

#include <qdebug.h>

#include <format>
#include <stdio.h>

template <typename... Args>
void formatDebug(std::format_string<Args...> fmt, Args &&... args)
{
   std::string tmp = std::format(fmt, std::forward<Args>(args)...);

   QString msg = QString::fromStdString(tmp);
   qt_message_output(QtDebugMsg, msg);
}

template <typename... Args>
void formatCritical(std::format_string<Args...> fmt, Args &&... args)
{
   std::string tmp = std::format(fmt, std::forward<Args>(args)...);

   QString msg = QString::fromStdString(tmp);
   qt_message_output(QtCriticalMsg, msg);
}

template <typename... Args>
void formatFatal(std::format_string<Args...> fmt, Args &&... args)
{
   std::string tmp = std::format(fmt, std::forward<Args>(args)...);

   QString msg = QString::fromStdString(tmp);
   qt_message_output(QtFatalMsg, msg);
}

template <typename... Args>
void formatWarning(std::format_string<Args...> fmt, Args &&... args)
{
   std::string tmp = std::format(fmt, std::forward<Args>(args)...);

   QString msg = QString::fromStdString(tmp);
   qt_message_output(QtWarningMsg, msg);
}

template <typename... Args>
void formatPrint(std::format_string<Args...> fmt, Args &&... args)
{
   std::string tmp = std::format(fmt, std::forward<Args>(args)...);
   fputs(tmp.c_str(), stdout);
}

#endif
