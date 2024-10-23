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

#ifndef QCppCastingHelper_P_H
#define QCppCastingHelper_P_H

#include <qglobal.h>

namespace QPatternist {

template<typename TSubClass>
class CppCastingHelper
{
 public:

   template<typename TCastTarget>
   const TCastTarget *as() const {
#if defined(Patternist_DEBUG)
      Q_ASSERT_X(dynamic_cast<const TCastTarget *>(static_cast<const TSubClass *>(this)), Q_FUNC_INFO,
            "Cast is invalid on some compilers. This class does not inherit the cast target.");
#endif
      return static_cast<const TCastTarget *>(static_cast<const TSubClass *>(this));
   }

   template<typename TCastTarget>
   TCastTarget *as() {

#if defined(Patternist_DEBUG)
      Q_ASSERT_X(dynamic_cast<TCastTarget *>(static_cast<TSubClass *>(this)), Q_FUNC_INFO,
            "Cast is invalid on some compilers. This class does not inherit the cast target.");
#endif
      return static_cast<TCastTarget *>(static_cast<TSubClass *>(this));
   }

 protected:
   CppCastingHelper()
   { }
};
}

#endif
