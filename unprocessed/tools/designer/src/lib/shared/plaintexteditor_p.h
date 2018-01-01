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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef PLAINTEXTEDITOR_H
#define PLAINTEXTEDITOR_H

#include <QtGui/QDialog>
#include "shared_global_p.h"

QT_BEGIN_NAMESPACE

class QPlainTextEdit;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT PlainTextEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PlainTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    ~PlainTextEditorDialog();

    int showDialog();

    void setDefaultFont(const QFont &font);

    void setText(const QString &text);
    QString text() const;

private:
    QPlainTextEdit *m_editor;
    QDesignerFormEditorInterface *m_core;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // RITCHTEXTEDITOR_H
