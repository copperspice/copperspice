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

#include <QtGui/qapplication.h>
#include <QtGui/qmainwindow.h>
#include <QtGui/qtabwidget.h>
#include <QtDBus/qdbusconnection.h>
#include "qdbusviewer.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow mw;
#ifndef Q_WS_MAC
    app.setWindowIcon(QIcon(QLatin1String(":/trolltech/qdbusviewer/images/qdbusviewer.png")));
#else
    mw.setWindowTitle(qApp->translate("QtDBusViewer", "Qt D-Bus Viewer"));
#endif


    QTabWidget *mainWidget = new QTabWidget;
    mw.setCentralWidget(mainWidget);
    QDBusViewer *sessionBusViewer = new QDBusViewer(QDBusConnection::sessionBus());
    QDBusViewer *systemBusViewer = new QDBusViewer(QDBusConnection::systemBus());
    mainWidget->addTab(sessionBusViewer, QObject::tr("Session Bus"));
    mainWidget->addTab(systemBusViewer, QObject::tr("System Bus"));

    QMenu *fileMenu = mw.menuBar()->addMenu(QObject::tr("&File"));
    QAction *quitAction = fileMenu->addAction(QObject::tr("&Quit"), &mw, SLOT(close()));
    Q_UNUSED(quitAction);

    QMenu *helpMenu = mw.menuBar()->addMenu(QObject::tr("&Help"));
    QAction *aboutAction = helpMenu->addAction(QObject::tr("&About"));
    aboutAction->setMenuRole(QAction::AboutRole);
    QObject::connect(aboutAction, SIGNAL(triggered()), sessionBusViewer, SLOT(about()));

    QAction *aboutQtAction = helpMenu->addAction(QObject::tr("About &Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    QObject::connect(aboutQtAction, SIGNAL(triggered()), &app, SLOT(aboutQt()));

    mw.show();

    return app.exec();
}

