/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#ifndef QHARFBUZZ_GUI_P_H
#define QHARFBUZZ_GUI_P_H

#include <qharfbuzz_core_p.h>

#include <hb.h>
#include <hb-ot.h>

class QFontEngine;

using qt_destroy_func_ptr        = void (*)(void *);
using qt_get_font_table_func_ptr = bool (*)(void *, uint, uchar *, uint *);

// font
Q_GUI_EXPORT hb_font_funcs_t *cs_get_font_funcs();

Q_GUI_EXPORT hb_face_t *cs_face_get_for_engine(QFontEngine *fe);
Q_GUI_EXPORT hb_font_t *cs_font_get_for_engine(QFontEngine *fe);

Q_GUI_EXPORT void cs_font_set_use_design_metrics(hb_font_t *font, uint value);
Q_GUI_EXPORT uint cs_font_get_use_design_metrics(hb_font_t *font);

#endif
