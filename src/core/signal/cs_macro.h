/***********************************************************************
*
* Copyright (c) 2015-2018 Barbara Geller
* Copyright (c) 2015-2018 Ansel Sermersheim
* All rights reserved.
*
* This file is part of libCsSignal
*
* libCsSignal is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
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