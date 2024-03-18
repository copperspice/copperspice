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

#include <qfreelist_p.h>

// default sizes and offsets (no need to define these when customizing)
enum OffsetSizes {
   Offset0 = 0x00000000,
   Offset1 = 0x00008000,
   Offset2 = 0x00080000,
   Offset3 = 0x00800000,

   Size0 = Offset1  - Offset0,
   Size1 = Offset2  - Offset1,
   Size2 = Offset3  - Offset2,
   Size3 = QFreeListDefaultConstants::MaxIndex - Offset3
};

const int QFreeListDefaultConstants::Sizes[QFreeListDefaultConstants::BlockCount] = {
   Size0,
   Size1,
   Size2,
   Size3
};
