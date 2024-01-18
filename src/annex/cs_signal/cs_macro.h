/***********************************************************************
*
* Copyright (c) 2016-2024 Barbara Geller
* Copyright (c) 2016-2024 Ansel Sermersheim
*
* This file is part of CsSignal.
*
* CsSignal is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CsSignal is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#ifndef LIB_CS_MACRO_H
#define LIB_CS_MACRO_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

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