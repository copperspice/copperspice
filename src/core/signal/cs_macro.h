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

#ifndef LIB_CS_MACRO_H
#define LIB_CS_MACRO_H

#ifdef _WIN32

#ifdef BUILDING_LIB_CS_SIGNAL
# define LIB_SIG_EXPORT     __declspec(dllexport)
#else
# define LIB_SIG_EXPORT     __declspec(dllimport)
#endif

#else
# define LIB_SIG_EXPORT

#endif


// ** signal macros
#define SIGNAL_1(...)  \
   __VA_ARGS__ {
// do not remove the "{", this is required for part two of the macro

#define SIGNAL_2(signalName, ...) \
      activate(*this, &std::remove_reference<decltype(*this)>::type::signalName, ##__VA_ARGS__); \
   }


#endif