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

#ifndef QSQL_H
#define QSQL_H

#include <qglobal.h>

namespace QSql {
enum Location {
   BeforeFirstRow = -1,
   AfterLastRow = -2
};

enum ParamTypeFlag {
   In = 0x00000001,
   Out = 0x00000002,
   InOut = In | Out,
   Binary = 0x00000004
};
using ParamType = QFlags<ParamTypeFlag>;

enum TableType {
   Tables = 0x01,
   SystemTables = 0x02,
   Views = 0x04,
   AllTables = 0xff
};

enum NumericalPrecisionPolicy {
   LowPrecisionInt32    = 0x01,
   LowPrecisionInt64    = 0x02,
   LowPrecisionDouble   = 0x04,

   HighPrecision        = 0
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(QSql::ParamType)

#endif // QSQL_H
