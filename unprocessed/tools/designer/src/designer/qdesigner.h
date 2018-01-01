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

#ifndef QDESIGNER_H
#define QDESIGNER_H

#include <QtCore/QPointer>
#include <QtGui/QApplication>

QT_BEGIN_NAMESPACE

#define qDesigner \
    (static_cast<QDesigner*>(QCoreApplication::instance()))

class QDesignerWorkbench;
class QDesignerToolWindow;
class MainWindowBase;
class QDesignerServer;
class QDesignerClient;
class QErrorMessage;

class QDesigner: public QApplication
{
    Q_OBJECT
public:
    QDesigner(int &argc, char **argv);
    virtual ~QDesigner();

    QDesignerWorkbench *workbench() const;
    QDesignerServer *server() const;
    MainWindowBase *mainWindow() const;
    void setMainWindow(MainWindowBase *tw);

protected:
    bool event(QEvent *ev);

signals:
    void initialized();

public slots:
    void showErrorMessage(const char *message);

private slots:
    void initialize();
    void callCreateForm();

private:
    bool parseCommandLineArgs(QStringList &fileNames, QString &resourceDir);
    void showErrorMessageBox(const QString &);

    QDesignerServer *m_server;
    QDesignerClient *m_client;
    QDesignerWorkbench *m_workbench;
    QPointer<MainWindowBase> m_mainWindow;
    QPointer<QErrorMessage> m_errorMessageDialog;

    QString m_initializationErrors;
    QString m_lastErrorMessage;
    bool m_suppressNewFormShow;
};

QT_END_NAMESPACE

#endif // QDESIGNER_H
