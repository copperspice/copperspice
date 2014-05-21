/***********************************************************************
*
* Copyright (c) 2012-2013 Barbara Geller
* Copyright (c) 2012-2013 Ansel Sermersheim
* Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef __PRODUCT_INCLUDE__
#  ifdef __QT_PRODUCT_INCLUDE_IS_LOWERCASE__
#    define __PRODUCT_INCLUDE__ <variant/symbian_os.hrh>
#  else
#    define __PRODUCT_INCLUDE__ <variant/Symbian_OS.hrh>
#  endif
#endif

#ifndef __QT_SYMBIAN_RESOURCE__
#  if defined(__ARMCC__) || defined(__CC_ARM)
#    ifdef __QT_RVCT_HEADER_IS_2_2__
#      include <rvct2_2.h>
#    else
#      include <rvct.h>
#    endif
#  endif
#endif
