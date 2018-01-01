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

#include "paletteeditorbutton.h"
#include "paletteeditor.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

PaletteEditorButton::PaletteEditorButton(QDesignerFormEditorInterface *core, const QPalette &palette, QWidget *parent)
    : QToolButton(parent),
      m_palette(palette)
{
    m_core = core;
    setFocusPolicy(Qt::NoFocus);
    setText(tr("Change Palette"));
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    connect(this, SIGNAL(clicked()), this, SLOT(showPaletteEditor()));
}

PaletteEditorButton::~PaletteEditorButton()
{
}

void PaletteEditorButton::setPalette(const QPalette &palette)
{
    m_palette = palette;
}

void PaletteEditorButton::setSuperPalette(const QPalette &palette)
{
    m_superPalette = palette;
}

void PaletteEditorButton::showPaletteEditor()
{
    int result;
    QPalette p = QPalette();
    QPalette pal = PaletteEditor::getPalette(m_core, 0, m_palette, m_superPalette, &result);
    if (result == QDialog::Accepted) {
        m_palette = pal;
        emit paletteChanged(m_palette);
    }
}

QT_END_NAMESPACE
