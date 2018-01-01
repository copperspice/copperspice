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

#include "scripterrordialog_p.h"

#include <QtGui/QTextEdit>
#include <QtGui/QTextCursor>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPen>
#include <QtCore/QCoreApplication>

QT_BEGIN_NAMESPACE

static void formatError(const QFormScriptRunner::Error &error,
                        QTextCursor &cursor)
{
    const QTextCharFormat oldFormat = cursor.charFormat();
    // Message
    cursor.insertText(QCoreApplication::translate("ScriptErrorDialog", "An error occurred while running the scripts for \"%1\":\n").arg(error.objectName));

    QTextCharFormat format(oldFormat);

    // verbatim listing
    format.setFontFamily(QLatin1String("Courier"));
    cursor.insertText(error.script, format);

    const QString newLine(QLatin1Char('\n'));

    cursor.insertText(newLine);

    // red error
    format = oldFormat;
    format.setTextOutline(QPen(Qt::red));
    cursor.insertText(error.errorMessage, format);
    cursor.insertText(newLine);
    cursor.setCharFormat (oldFormat);
}

namespace qdesigner_internal {

    // ScriptErrorDialog
    ScriptErrorDialog::ScriptErrorDialog(const Errors& errors, QWidget *parent) :
        QDialog(parent),
        m_textEdit(new QTextEdit)
    {
        setWindowTitle(tr("Script errors"));
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setModal(true);

        QVBoxLayout *vboxLayout = new QVBoxLayout(this);

        m_textEdit->setReadOnly(true);
        m_textEdit->setMinimumSize(QSize(600, 400));
        vboxLayout->addWidget(m_textEdit);
        // button box
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(buttonBox , SIGNAL(rejected()), this, SLOT(reject()));
        vboxLayout->addWidget(buttonBox);

        // Generate text
        QTextCursor cursor = m_textEdit->textCursor();
        cursor.movePosition (QTextCursor::End);
        foreach (const QFormScriptRunner::Error error, errors)
            formatError(error, cursor);
    }
} // namespace qdesigner_internal

QT_END_NAMESPACE
