/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#ifndef CS_CATCH_DEFINES_H
#define CS_CATCH_DEFINES_H

#if ( defined(__GNUC__) && ((__GNUC__ == 10 && __GNUC_MINOR__ >= 4) || __GNUC__ >= 11)) ||   \
    ( defined(__apple_build_version__) && (__clang_major__ >= 11) ) ||   \
    ( defined(__clang__) && (__clang_major >= 14) )

#define CS_CHRONO_TYPES_CATCH

#endif


#if ( defined(__GNUC__) && (__GNUC__ >= 11) ) ||   \
    ( defined(__apple_build_version__) && (__clang_major__ >= 11) ) ||   \
    ( defined(__clang__) && (__clang_major >= 14) )

#define CS_CHRONO_TYPES_CATCH_YMD

#endif


#endif
