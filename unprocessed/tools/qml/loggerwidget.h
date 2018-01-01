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

#ifndef LOGGERWIDGET_H
#define LOGGERWIDGET_H

#include <QMainWindow>
#include <QMetaType>

QT_BEGIN_NAMESPACE

class QPlainTextEdit;
class QLabel;
class QMenu;
class QAction;

class LoggerWidget : public QMainWindow {
    Q_OBJECT
public:
    LoggerWidget(QWidget *parent=0);

    enum Visibility { ShowWarnings, HideWarnings, AutoShowWarnings };

    Visibility defaultVisibility() const;
    void setDefaultVisibility(Visibility visibility);

    QMenu *preferencesMenu();
    QAction *showAction();

public slots:
    void append(const QString &msg);
    void updateNoWarningsLabel();

private slots:
    void warningsPreferenceChanged(QAction *action);
    void readSettings();
    void saveSettings();

protected:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);

signals:
    void opened();
    void closed();

private:
    void setupPreferencesMenu();

    QMenu *m_preferencesMenu;
    QAction *m_showWidgetAction;
    QPlainTextEdit *m_plainTextEdit;
    QLabel *m_noWarningsLabel;
    enum ConfigOrigin { CommandLineOrigin, SettingsOrigin };
    ConfigOrigin m_visibilityOrigin;
    Visibility m_visibility;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(LoggerWidget::Visibility)

#endif // LOGGERWIDGET_H
