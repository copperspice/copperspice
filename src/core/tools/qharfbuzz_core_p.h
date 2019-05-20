/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QHARFBUZZ_CORE_P_H
#define QHARFBUZZ_CORE_P_H

#include <qchar.h>
#include <hb.h>

using glyph_t  = quint32;
using HB_Bool  = hb_bool_t;
using HB_Glyph = hb_codepoint_t;

using qt_destroy_func_ptr        = void (*)(void *);
using qt_get_font_table_func_ptr = bool (*)(void *, uint, uchar *, uint *);

Q_CORE_EXPORT hb_script_t  cs_script_to_hb_script(QChar::Script script);
Q_CORE_EXPORT hb_unicode_funcs_t *cs_get_unicode_funcs();

#endif
