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

#ifndef FORMEDITOR_H
#define FORMEDITOR_H

#include "formeditor_global.h"

#include <QtDesigner/QDesignerFormEditorInterface>

QT_BEGIN_NAMESPACE

class QObject;

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT FormEditor: public QDesignerFormEditorInterface
{
    Q_OBJECT
public:
    FormEditor(QObject *parent = 0);
    virtual ~FormEditor();
public slots:
    void slotQrcFileChangedExternally(const QString &path);
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMEDITOR_H
