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

#ifndef INSTALLDIALOG_H
#define INSTALLDIALOG_H

#include <QtCore/QQueue>
#include <QtGui/QDialog>
#include <QtNetwork/QHttpResponseHeader>
#include "ui_installdialog.h"

#ifndef QT_NO_HTTP

QT_BEGIN_NAMESPACE

class QHttp;
class QBuffer;
class QFile;
class QHelpEngineCore;

class InstallDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InstallDialog(QHelpEngineCore *helpEngine, QWidget *parent = 0,
        const QString &host = QString(), int port = -1);
    ~InstallDialog();

    QStringList installedDocumentations() const;

private slots:
    void init();
    void cancelDownload();
    void install();
    void httpRequestFinished(int requestId, bool error);
    void readResponseHeader(const QHttpResponseHeader &responseHeader);
    void updateDataReadProgress(int bytesRead, int totalBytes);
    void updateInstallButton();
    void browseDirectories();

private:
    void downloadNextFile();
    void updateDocItemList();
    void installFile(const QString &fileName);

    Ui::InstallDialog m_ui;
    QHelpEngineCore *m_helpEngine;
    QHttp *m_http;
    QBuffer *m_buffer;
    QFile *m_file;
    bool m_httpAborted;
    int m_docInfoId;
    int m_docId;
    QQueue<QListWidgetItem*> m_itemsToInstall;
    QString m_currentCheckSum;
    QString m_windowTitle;
    QStringList m_installedDocumentations;
    QString m_host;
    int m_port;
};

QT_END_NAMESPACE

#endif

#endif // INSTALLDIALOG_H
