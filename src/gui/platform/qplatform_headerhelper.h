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

#ifndef QPLATFORM_HEADERHELPER_H
#define QPLATFORM_HEADERHELPER_H

#include <QByteArray>
#include <QGuiApplication>

namespace QPlatformHeaderHelper {

template <typename ReturnT, typename FunctionT>
ReturnT callPlatformFunction(const QByteArray &functionName)
{
    FunctionT func = reinterpret_cast<FunctionT>(QGuiApplication::platformFunction(functionName));
    return func ? func() : ReturnT();
}

template <typename ReturnT, typename FunctionT, typename Arg1>
ReturnT callPlatformFunction(const QByteArray &functionName, Arg1 a1)
{
    FunctionT func = reinterpret_cast<FunctionT>(QGuiApplication::platformFunction(functionName));
    return func ? func(a1) : ReturnT();
}

template <typename ReturnT, typename FunctionT, typename Arg1, typename Arg2>
ReturnT callPlatformFunction(const QByteArray &functionName, Arg1 a1, Arg2 a2)
{
    FunctionT func = reinterpret_cast<FunctionT>(QGuiApplication::platformFunction(functionName));
    return func ? func(a1, a2) : ReturnT();
}

template <typename ReturnT, typename FunctionT, typename Arg1, typename Arg2, typename Arg3>
ReturnT callPlatformFunction(const QByteArray &functionName, Arg1 a1, Arg2 a2, Arg3 a3)
{
    FunctionT func = reinterpret_cast<FunctionT>(QGuiApplication::platformFunction(functionName));
    return func ? func(a1, a2, a3) : ReturnT();
}

template <typename ReturnT, typename FunctionT, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
ReturnT callPlatformFunction(const QByteArray &functionName, Arg1 a1, Arg2 a2, Arg3 a3, Arg4 a4)
{
    FunctionT func = reinterpret_cast<FunctionT>(QGuiApplication::platformFunction(functionName));
    return func ? func(a1, a2, a3, a4) : ReturnT();
}

}

#endif
