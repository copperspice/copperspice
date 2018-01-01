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

#ifndef STYLESHEETEDITOR_H
#define STYLESHEETEDITOR_H

#include <QtGui/QTextEdit>
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include "shared_global_p.h"

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;

class QDialogButtonBox;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT StyleSheetEditor : public QTextEdit
{
    Q_OBJECT
public:
    StyleSheetEditor(QWidget *parent = 0);
};

// Edit a style sheet.
class QDESIGNER_SHARED_EXPORT StyleSheetEditorDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode {
        ModeGlobal, // resources are disabled (we don't have current resource set loaded), used e.g. in configuration dialog context
        ModePerForm // resources are available
    };

    StyleSheetEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent, Mode mode = ModePerForm);
    ~StyleSheetEditorDialog();
    QString text() const;
    void setText(const QString &t);

    static bool isStyleSheetValid(const QString &styleSheet);


private slots:
    void validateStyleSheet();
    void slotContextMenuRequested(const QPoint &pos);
    void slotAddResource(const QString &property);
    void slotAddGradient(const QString &property);
    void slotAddColor(const QString &property);
    void slotAddFont();
    void slotRequestHelp();

protected:
    QDialogButtonBox *buttonBox() const;
    void setOkButtonEnabled(bool v);

private:
    void insertCssProperty(const QString &name, const QString &value);

    QDialogButtonBox *m_buttonBox;
    StyleSheetEditor *m_editor;
    QLabel *m_validityLabel;
    QDesignerFormEditorInterface *m_core;
    QAction *m_addResourceAction;
    QAction *m_addGradientAction;
    QAction *m_addColorAction;
    QAction *m_addFontAction;
};

// Edit the style sheet property of the designer selection.
// Provides an "Apply" button.

class QDESIGNER_SHARED_EXPORT StyleSheetPropertyEditorDialog : public StyleSheetEditorDialog
{
    Q_OBJECT
public:
    StyleSheetPropertyEditorDialog(QWidget *parent, QDesignerFormWindowInterface *fw, QWidget *widget);

    static bool isStyleSheetValid(const QString &styleSheet);

private slots:
    void applyStyleSheet();

private:
    QDesignerFormWindowInterface *m_fw;
    QWidget *m_widget;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // STYLESHEETEDITOR_H
