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

#ifndef QDESIGNER_ABTRACT_LANGUAGE_H
#define QDESIGNER_ABTRACT_LANGUAGE_H

#include <QtDesigner/extension.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDialog;
class QWidget;
class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QDesignerResourceBrowserInterface;

class QDesignerLanguageExtension
{
public:
    virtual ~QDesignerLanguageExtension() {}

    virtual QDialog *createFormWindowSettingsDialog(QDesignerFormWindowInterface *formWindow, QWidget *parentWidget) = 0;
    virtual QDesignerResourceBrowserInterface *createResourceBrowser(QWidget *parentWidget) = 0;

    virtual QDialog *createPromotionDialog(QDesignerFormEditorInterface *formEditor, QWidget *parentWidget = 0) = 0;

    virtual QDialog *createPromotionDialog(QDesignerFormEditorInterface *formEditor,
                                           const QString &promotableWidgetClassName,
                                           QString *promoteToClassName,
                                           QWidget *parentWidget = 0) = 0;

    virtual bool isLanguageResource(const QString &path) const = 0;

    virtual QString classNameOf(QObject *object) const = 0;

    virtual bool signalMatchesSlot(const QString &signal, const QString &slot) const = 0;

    virtual QString widgetBoxContents() const = 0;

    virtual QString uiExtension() const = 0;
};

Q_DECLARE_EXTENSION_INTERFACE(QDesignerLanguageExtension, "com.trolltech.Qt.Designer.Language.3")

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDESIGNER_ABTRACT_LANGUAGE_H
