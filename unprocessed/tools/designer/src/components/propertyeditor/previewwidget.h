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

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "ui_previewwidget.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class PreviewWidget: public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent);
    virtual ~PreviewWidget();

private:
    Ui::PreviewWidget ui;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // PREVIEWWIDGET_H
