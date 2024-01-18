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

#ifndef QTRANSLATOR_P_H
#define QTRANSLATOR_P_H

enum class TranslatorCategory {
   Invalid      = 0,
   Contexts     = 0x2f,
   Hashes       = 0x42,
   Messages     = 0x69,
   CountRules   = 0x88,
   Dependencies = 0x96
};

enum class TranslatorTag {
   End           = 1,
   Obsolete1     = 2,
   SourceText    = 3,
   Context       = 4,
   Comment       = 5,
   Translation   = 6,
};

enum class CountGuide  {
   Equal            = 0x01,
   LessThan         = 0x02,
   LessThanEqual    = 0x03,
   Between          = 0x04,

   Not              = 0x08,
   Remainder_10     = 0x10,
   Remainder_100    = 0x20,
   Divide_1000      = 0x40,

   And              = 0xFD,
   Or               = 0xFE,
   LastEntry        = 0xFF,

   OperatorMask     = 0x07,
   OperatorInvalid  = 0x80,

   NotEqual         = Not | Equal,
   GreaterThan      = Not | LessThanEqual,
   GreaterThanEqual = Not | LessThan,
   NotBetween       = Not | Between
};

inline constexpr CountGuide operator|(CountGuide a, CountGuide b)
{
   using T = std::underlying_type_t<CountGuide>;

   return static_cast<CountGuide>( static_cast<T>(a) | static_cast<T>(b) );
}

inline constexpr auto operator&(CountGuide a, CountGuide b)
{
   using T = std::underlying_type_t<CountGuide>;

   return static_cast<T>(a) & static_cast<T>(b);
}

#endif
