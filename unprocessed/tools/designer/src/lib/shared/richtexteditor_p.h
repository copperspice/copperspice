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

#ifndef RICHTEXTEDITOR_H
#define RICHTEXTEDITOR_H

#include <QtGui/QTextEdit>
#include <QtGui/QDialog>
#include "shared_global_p.h"

QT_BEGIN_NAMESPACE

class QTabWidget;
class QToolBar;

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class RichTextEditor;

class QDESIGNER_SHARED_EXPORT RichTextEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RichTextEditorDialog(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    ~RichTextEditorDialog();

    int showDialog();
    void setDefaultFont(const QFont &font);
    void setText(const QString &text);
    QString text(Qt::TextFormat format = Qt::AutoText) const;

private slots:
    void tabIndexChanged(int newIndex);
    void richTextChanged();
    void sourceChanged();

private:
    enum TabIndex { RichTextIndex, SourceIndex };
    enum State { Clean, RichTextChanged, SourceChanged };
    RichTextEditor *m_editor;
    QTextEdit      *m_text_edit;
    QTabWidget     *m_tab_widget;
    State m_state;
    QDesignerFormEditorInterface *m_core;
    int m_initialTab;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // RITCHTEXTEDITOR_H
