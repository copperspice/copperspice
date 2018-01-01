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

#ifndef QHARFBUZZ_P_H
#define QHARFBUZZ_P_H

#include <QtCore/qglobal.h>
#include <harfbuzz-shaper.h>

QT_BEGIN_NAMESPACE

// temporary forward until all the textengine code has been moved to QtCore
Q_CORE_EXPORT void qGetCharAttributes(const HB_UChar16 *string, hb_uint32 stringLength,
                                      const HB_ScriptItem *items, hb_uint32 numItems, HB_CharAttributes *attributes);

Q_CORE_EXPORT HB_Bool qShapeItem(HB_ShaperItem *item);

// ### temporary
Q_CORE_EXPORT HB_Face qHBNewFace(void *font, HB_GetFontTableFunc tableFunc);
Q_CORE_EXPORT void qHBFreeFace(HB_Face);

Q_DECLARE_TYPEINFO(HB_GlyphAttributes, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(HB_FixedPoint, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

#endif
