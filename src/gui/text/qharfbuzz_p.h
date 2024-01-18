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

#ifndef QHARFBUZZ_P_H
#define QHARFBUZZ_P_H

#include <qchar.h>

#include <hb.h>
#include <hb-ot.h>

class QFontEngine;

using glyph_t  = uint32_t;
using HB_Bool  = hb_bool_t;
using HB_Glyph = hb_codepoint_t;

using cs_fontTable_func_ptr = bool (*)(void *, uint, uchar *, uint *);

// script
hb_script_t cs_script_to_hb_script(QChar::Script script);
hb_unicode_funcs_t *cs_get_unicode_funcs();

// font
Q_GUI_EXPORT std::shared_ptr<hb_face_t> cs_face_get_for_engine(QFontEngine *fe);
Q_GUI_EXPORT std::shared_ptr<hb_font_t> cs_font_get_for_engine(QFontEngine *fe);

Q_GUI_EXPORT void cs_font_set_use_design_metrics(hb_font_t *font, uint value);
Q_GUI_EXPORT uint cs_font_get_use_design_metrics(hb_font_t *font);

#endif
