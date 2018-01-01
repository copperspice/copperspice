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

#ifndef STRINGLISTEDITORBUTTON_H
#define STRINGLISTEDITORBUTTON_H

#include "propertyeditor_global.h"

#include <QtCore/QStringList>
#include <QtGui/QToolButton>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QT_PROPERTYEDITOR_EXPORT StringListEditorButton: public QToolButton
{
    Q_OBJECT
public:
    explicit StringListEditorButton(const QStringList &stringList, QWidget *parent = 0);
    virtual ~StringListEditorButton();

    inline QStringList stringList() const
    { return m_stringList; }

signals:
    void stringListChanged(const QStringList &stringList);

public slots:
    void setStringList(const QStringList &stringList);

private slots:
    void showStringListEditor();

private:
    QStringList m_stringList;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // STRINGLISTEDITORBUTTON_H
