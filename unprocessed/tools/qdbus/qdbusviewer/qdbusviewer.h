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

#ifndef QDBUSVIEWER_H
#define QDBUSVIEWER_H

#include <QtGui/QtGui>
#include <QtDBus/QtDBus>

QT_FORWARD_DECLARE_CLASS(QTreeView);
QT_FORWARD_DECLARE_CLASS(QDomDocument);
QT_FORWARD_DECLARE_CLASS(QDomElement);

struct BusSignature
{
    QString mService, mPath, mInterface, mName;
    QString mTypeSig;
};

class QDBusViewer: public QWidget
{
    Q_OBJECT
public:
    QDBusViewer(const QDBusConnection &connection, QWidget *parent = 0);

public slots:
    void refresh();
    void about();

private slots:
    void serviceChanged(const QModelIndex &index);
    void showContextMenu(const QPoint &);
    void connectionRequested(const BusSignature &sig);
    void callMethod(const BusSignature &sig);
    void getProperty(const BusSignature &sig);
    void setProperty(const BusSignature &sig);
    void dumpMessage(const QDBusMessage &msg);
    void refreshChildren();

    void serviceRegistered(const QString &service);
    void serviceUnregistered(const QString &service);
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);

    void activate(const QModelIndex &item);

    void logError(const QString &msg);
    void anchorClicked(const QUrl &url);

private:
    void logMessage(const QString &msg);

    QDBusConnection c;
    QString currentService;
    QTreeView *tree;
    QAction *refreshAction;
    QTreeWidget *services;
    QStringListModel *servicesModel;
    QSortFilterProxyModel *servicesFilterModel;
    QLineEdit *serviceFilterLine;
    QListView *servicesView;
    QTextBrowser *log;
    QRegExp objectPathRegExp;
};

#endif
