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

#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class HelpEngineWrapper;
class MainWindow;

class RemoteControl : public QObject
{
    Q_OBJECT

public:
    RemoteControl(MainWindow *mainWindow);

private slots:
    void receivedData();
    void handleCommandString(const QString &cmdString);
    void applyCache();

private:
    void clearCache();
    void splitInputString(const QString &input, QString &cmd, QString &arg);
    void handleDebugCommand(const QString &arg);
    void handleShowOrHideCommand(const QString &arg, bool show);
    void handleSetSourceCommand(const QString &arg);
    void handleSyncContentsCommand();
    void handleActivateKeywordCommand(const QString &arg);
    void handleActivateIdentifierCommand(const QString &arg);
    void handleExpandTocCommand(const QString &arg);
    void handleSetCurrentFilterCommand(const QString &arg);
    void handleRegisterCommand(const QString &arg);
    void handleUnregisterCommand(const QString &arg);

private:
    MainWindow *m_mainWindow;
    bool m_debug;

    bool m_caching;
    QUrl m_setSource;
    bool m_syncContents;
    QString m_activateKeyword;
    QString m_activateIdentifier;
    int m_expandTOC;
    QString m_currentFilter;
    HelpEngineWrapper &helpEngine;
};

QT_END_NAMESPACE

#endif
