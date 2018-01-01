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

#ifndef NEWACTIONDIALOG_P_H
#define NEWACTIONDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qdesigner_utils_p.h" //  PropertySheetIconValue

#include <QtGui/QDialog>
#include <QtGui/QKeySequence>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

namespace Ui {
    class NewActionDialog;
}

class ActionEditor;

struct ActionData {

    enum ChangeMask {
        TextChanged = 0x1, NameChanged = 0x2, ToolTipChanged = 0x4,
        IconChanged = 0x8, CheckableChanged = 0x10, KeysequenceChanged = 0x20
    };

    ActionData();
    // Returns a combination of ChangeMask flags
    unsigned compare(const  ActionData &rhs) const;

    QString text;
    QString name;
    QString toolTip;
    PropertySheetIconValue icon;
    bool checkable;
    PropertySheetKeySequenceValue keysequence;
};

inline bool operator==(const ActionData &a1, const ActionData &a2) {  return a1.compare(a2) == 0u; }
inline bool operator!=(const ActionData &a1, const ActionData &a2) {  return a1.compare(a2) != 0u; }

class NewActionDialog: public QDialog
{
    Q_OBJECT
public:
    explicit NewActionDialog(ActionEditor *parent);
    virtual ~NewActionDialog();

    ActionData actionData() const;
    void setActionData(const ActionData &d);

    QString actionText() const;
    QString actionName() const;

private slots:
    void on_editActionText_textEdited(const QString &text);
    void on_editObjectName_textEdited(const QString &text);
    void slotEditToolTip();
    void slotResetKeySequence();

private:
    Ui::NewActionDialog *m_ui;
    ActionEditor *m_actionEditor;
    bool m_auto_update_object_name;

    void updateButtons();
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // NEWACTIONDIALOG_P_H
