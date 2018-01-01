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

#ifndef SCRIPTERRORDIALOG_H
#define SCRIPTERRORDIALOG_H

#include "shared_global_p.h"
#include "formscriptrunner_p.h"

#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QTextEdit;

namespace qdesigner_internal {

    // Dialog for showing script errors
    class  QDESIGNER_SHARED_EXPORT ScriptErrorDialog : public QDialog {
        Q_OBJECT

    public:
        typedef  QFormScriptRunner::Errors Errors;
        ScriptErrorDialog(const Errors& errors, QWidget *parent);

    private:
        QTextEdit *m_textEdit;

    };
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SCRIPTERRORDIALOG_H
