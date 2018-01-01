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

#ifndef PALETTEEDITORBUTTON_H
#define PALETTEEDITORBUTTON_H

#include "propertyeditor_global.h"

#include <QtGui/QPalette>
#include <QtGui/QToolButton>

#include "abstractformeditor.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QT_PROPERTYEDITOR_EXPORT PaletteEditorButton: public QToolButton
{
    Q_OBJECT
public:
    PaletteEditorButton(QDesignerFormEditorInterface *core, const QPalette &palette, QWidget *parent = 0);
    virtual ~PaletteEditorButton();

    void setSuperPalette(const QPalette &palette);
    inline QPalette palette() const
    { return m_palette; }

signals:
    void paletteChanged(const QPalette &palette);

public slots:
    void setPalette(const QPalette &palette);

private slots:
    void showPaletteEditor();

private:
    QPalette m_palette;
    QPalette m_superPalette;
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // PALETTEEDITORBUTTON_H
