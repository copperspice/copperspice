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

#ifndef QDESIGNER_APPEARANCEOPTIONS_H
#define QDESIGNER_APPEARANCEOPTIONS_H

#include "designer_enums.h"
#include "qdesigner_toolwindow.h"

#include <QtDesigner/private/abstractoptionspage_p.h>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerSettings;

namespace Ui {
    class AppearanceOptionsWidget;
}

/* AppearanceOptions data */
struct AppearanceOptions {
    AppearanceOptions();
    bool equals(const AppearanceOptions&) const;
    void toSettings(QDesignerSettings &) const;
    void fromSettings(const QDesignerSettings &);

    UIMode uiMode;
    ToolWindowFontSettings toolWindowFontSettings;
};

inline bool operator==(const AppearanceOptions &ao1, const AppearanceOptions &ao2)
{
    return ao1.equals(ao2);
}

inline bool operator!=(const AppearanceOptions &ao1, const AppearanceOptions &ao2)
{
    return !ao1.equals(ao2);
}

/* QDesignerAppearanceOptionsWidget: Let the user edit AppearanceOptions */
class QDesignerAppearanceOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QDesignerAppearanceOptionsWidget(QWidget *parent = 0);
    ~QDesignerAppearanceOptionsWidget();

    AppearanceOptions appearanceOptions() const;
    void setAppearanceOptions(const AppearanceOptions &ao);

signals:
    void uiModeChanged(bool modified);

private slots:
    void slotUiModeComboChanged();

private:
    UIMode uiMode() const;

    Ui::AppearanceOptionsWidget *m_ui;
    UIMode m_initialUIMode;
};

/* The options page for appearance options. Emits a Timer-0 delayed changed
 * signal to allow the preferences dialog to close (and be deleted) before a
 * possible switch from docked mode to top-level mode happens. (The switch
 * would delete the main window, which the preference dialog is a child of
 * -> BOOM) */

class QDesignerAppearanceOptionsPage : public QObject, public QDesignerOptionsPageInterface
{
    Q_OBJECT

public:
    QDesignerAppearanceOptionsPage(QDesignerFormEditorInterface *core);

    QString name() const;
    QWidget *createPage(QWidget *parent);
    virtual void apply();
    virtual void finish();

signals:
    void settingsChangedDelayed();

private:
    QDesignerFormEditorInterface *m_core;
    QPointer<QDesignerAppearanceOptionsWidget> m_widget;
    AppearanceOptions m_initialOptions;
};

QT_END_NAMESPACE

#endif // QDESIGNER_APPEARANCEOPTIONS_H
