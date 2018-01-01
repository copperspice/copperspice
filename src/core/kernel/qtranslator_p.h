/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QTRANSLATOR_P_H
#define QTRANSLATOR_P_H

enum {
   Q_EQ          = 0x01,
   Q_LT          = 0x02,
   Q_LEQ         = 0x03,
   Q_BETWEEN     = 0x04,

   Q_NOT         = 0x08,
   Q_MOD_10      = 0x10,
   Q_MOD_100     = 0x20,
   Q_LEAD_1000   = 0x40,

   Q_AND         = 0xFD,
   Q_OR          = 0xFE,
   Q_NEWRULE     = 0xFF,

   Q_OP_MASK     = 0x07,

   Q_NEQ         = Q_NOT | Q_EQ,
   Q_GT          = Q_NOT | Q_LEQ,
   Q_GEQ         = Q_NOT | Q_LT,
   Q_NOT_BETWEEN = Q_NOT | Q_BETWEEN
};

#endif
