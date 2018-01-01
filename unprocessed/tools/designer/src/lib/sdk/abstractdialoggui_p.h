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

#ifndef ABSTRACTDIALOGGUI_H
#define ABSTRACTDIALOGGUI_H

#include <QtDesigner/sdk_global.h>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QWidget;

class QDESIGNER_SDK_EXPORT QDesignerDialogGuiInterface
{
    Q_DISABLE_COPY(QDesignerDialogGuiInterface)
public:
    QDesignerDialogGuiInterface();
    virtual ~QDesignerDialogGuiInterface();

    enum Message { FormLoadFailureMessage, UiVersionMismatchMessage, ResourceLoadFailureMessage,
                   TopLevelSpacerMessage, PropertyEditorMessage, SignalSlotEditorMessage, FormEditorMessage,
                   PreviewFailureMessage, PromotionErrorMessage, ResourceEditorMessage,
                   ScriptDialogMessage, SignalSlotDialogMessage, OtherMessage, FileChangedMessage };

    virtual QMessageBox::StandardButton
        message(QWidget *parent, Message context, QMessageBox::Icon icon,
                const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) = 0;

    virtual QMessageBox::StandardButton
        message(QWidget *parent, Message context, QMessageBox::Icon icon,
                const QString &title, const QString &text, const QString &informativeText,
                QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) = 0;

    virtual QMessageBox::StandardButton
        message(QWidget *parent, Message context, QMessageBox::Icon icon,
                const QString &title, const QString &text, const QString &informativeText, const QString &detailedText,
                QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) = 0;

    virtual QString getExistingDirectory(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), QFileDialog::Options options = QFileDialog::ShowDirsOnly)= 0;
    virtual QString getOpenFileName(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = 0, QFileDialog::Options options = 0)= 0;
    virtual QString getOpenImageFileName(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = 0, QFileDialog::Options options = 0);
    virtual QStringList getOpenFileNames(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = 0, QFileDialog::Options options = 0)= 0;
    virtual QStringList getOpenImageFileNames(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = 0, QFileDialog::Options options = 0);
    virtual QString getSaveFileName(QWidget *parent = 0, const QString &caption = QString(), const QString &dir = QString(), const QString &filter = QString(), QString *selectedFilter = 0, QFileDialog::Options options = 0)= 0;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // ABSTRACTDIALOGGUI_H
