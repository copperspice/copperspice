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

#ifndef TEXTPROPERTYEDITOR_H
#define TEXTPROPERTYEDITOR_H

#include "shared_global_p.h"
#include "shared_enums_p.h"

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

    class PropertyLineEdit;

    // Inline-Editor for text properties. Does escaping of newline characters
    // to '\n' and back and provides validation modes. The interface
    // corresponds to that of QLineEdit.
    class QDESIGNER_SHARED_EXPORT TextPropertyEditor : public QWidget
    {
        TextPropertyEditor(const TextPropertyEditor &);
        TextPropertyEditor& operator=(const TextPropertyEditor &);
        Q_OBJECT
        Q_PROPERTY(QString text READ text WRITE setText USER true)
    public:
        enum EmbeddingMode {
            // Stand-alone widget
            EmbeddingNone,
                // Disable frame
                EmbeddingTreeView,
                // For editing in forms
                EmbeddingInPlace
        };

        enum UpdateMode {
            // Emit textChanged() as the user types
            UpdateAsYouType,
            // Emit textChanged() only when the user finishes (for QUrl, etc.)
            UpdateOnFinished
        };

        explicit TextPropertyEditor(QWidget *parent = 0, EmbeddingMode embeddingMode = EmbeddingNone, TextPropertyValidationMode validationMode = ValidationMultiLine);

        TextPropertyValidationMode textPropertyValidationMode() const { return m_validationMode; }
        void setTextPropertyValidationMode(TextPropertyValidationMode vm);

        UpdateMode updateMode() const                { return m_updateMode; }
        void setUpdateMode(UpdateMode um) { m_updateMode = um; }

        QString text() const;

        virtual QSize sizeHint () const;
        virtual QSize minimumSizeHint () const;

        void setAlignment(Qt::Alignment alignment);

        bool hasAcceptableInput() const;

        // installs an event filter object on the private QLineEdit
        void installEventFilter(QObject *filterObject);

        // Replace newline characters by literal "\n" for inline editing
        // in mode ValidationMultiLine
        static QString stringToEditorString(const QString &s, TextPropertyValidationMode validationMode = ValidationMultiLine);

        // Replace literal "\n"  by actual new lines in mode ValidationMultiLine
        static QString editorStringToString(const QString &s, TextPropertyValidationMode validationMode = ValidationMultiLine);

        // Returns whether newline characters are valid in validationMode.
        static bool multiLine(TextPropertyValidationMode validationMode);

    signals:
        void textChanged(const QString &text);
        void editingFinished();

    public slots:
        void setText(const QString &text);
        void selectAll();
        void clear();

    protected:
        void resizeEvent(QResizeEvent * event );

    private slots:
        void slotTextChanged(const QString &text);
        void slotTextEdited();
        void slotEditingFinished();

    private:
        void setRegExpValidator(const QString &pattern);
        void markIntermediateState();

        TextPropertyValidationMode m_validationMode;
        UpdateMode m_updateMode;
        PropertyLineEdit* m_lineEdit;

        // Cached text containing real newline characters.
        QString m_cachedText;
        bool m_textEdited;
    };
}

QT_END_NAMESPACE

#endif // TEXTPROPERTYEDITOR_H
