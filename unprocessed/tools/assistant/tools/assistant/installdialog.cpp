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

#include "tracer.h"

#include "installdialog.h"

#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QCryptographicHash>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include <QtHelp/QHelpEngineCore>

#include <QtNetwork/QHttp>

QT_BEGIN_NAMESPACE
#ifndef QT_NO_HTTP

#define QCH_FILENAME  92943
#define QCH_NAMESPACE 92944
#define QCH_CHECKSUM  92945

InstallDialog::InstallDialog(QHelpEngineCore *helpEngine, QWidget *parent,
                             const QString &host, int port)
    : QDialog(parent), m_helpEngine(helpEngine), m_host(host), m_port(port)
{
    TRACE_OBJ
    m_ui.setupUi(this);
    
    m_ui.installButton->setEnabled(false);
    m_ui.cancelButton->setEnabled(false);
    m_ui.pathLineEdit->setText(QFileInfo(m_helpEngine->collectionFile()).absolutePath());
    m_ui.progressBar->hide();

    m_windowTitle = tr("Install Documentation");

    m_http = new QHttp(this);
    connect(m_http, SIGNAL(requestFinished(int,bool)),
            this, SLOT(httpRequestFinished(int,bool)));
    connect(m_http, SIGNAL(dataReadProgress(int,int)),
            this, SLOT(updateDataReadProgress(int,int)));
    connect(m_http, SIGNAL(responseHeaderReceived(QHttpResponseHeader)),
            this, SLOT(readResponseHeader(QHttpResponseHeader)));
    connect(m_ui.installButton, SIGNAL(clicked()), this, SLOT(install()));
    connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelDownload()));
    connect(m_ui.browseButton, SIGNAL(clicked()), this, SLOT(browseDirectories()));

    connect(m_ui.listWidget, SIGNAL(itemChanged(QListWidgetItem*)),
        this, SLOT(updateInstallButton()));

    QTimer::singleShot(0, this, SLOT(init()));
}

InstallDialog::~InstallDialog()
{
    TRACE_OBJ
}

QStringList InstallDialog::installedDocumentations() const
{
    TRACE_OBJ
    return m_installedDocumentations;
}

void InstallDialog::init()
{
    TRACE_OBJ
    m_ui.statusLabel->setText(tr("Downloading documentation info..."));
    m_ui.progressBar->show();
    
    QUrl url(QLatin1String("http://qt.nokia.com/doc/assistantdocs/docs.txt"));
    m_buffer = new QBuffer();
    m_buffer->open(QBuffer::ReadWrite);

    if (m_port > -1)
        m_http->setProxy(m_host, m_port);
    m_http->setHost(url.host());
    m_httpAborted = false;
    m_docInfoId = m_http->get(url.path(), m_buffer);

    m_ui.cancelButton->setEnabled(true);
    m_ui.closeButton->setEnabled(false);    
}

void InstallDialog::updateInstallButton()
{
    TRACE_OBJ
    QListWidgetItem *item = 0;
    for (int i=0; i<m_ui.listWidget->count(); ++i) {
        item = m_ui.listWidget->item(i);
        if (item->checkState() == Qt::Checked
            && item->flags() & Qt::ItemIsEnabled) {
            m_ui.installButton->setEnabled(true);
            return;
        }       
    }
    m_ui.installButton->setEnabled(false);
}

void InstallDialog::updateDocItemList()
{
    TRACE_OBJ
    QStringList registeredDocs = m_helpEngine->registeredDocumentations();
    QListWidgetItem *item = 0;
    for (int i=0; i<m_ui.listWidget->count(); ++i) {
        item = m_ui.listWidget->item(i);
        QString ns = item->data(QCH_NAMESPACE).toString();
        if (!ns.isEmpty() && registeredDocs.contains(ns)) {
            item->setFlags(Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
        }
        item->setCheckState(Qt::Unchecked);
    }
}

void InstallDialog::cancelDownload()
{
    TRACE_OBJ
    m_ui.statusLabel->setText(tr("Download canceled."));
    m_httpAborted = true;
    m_itemsToInstall.clear();
    m_http->abort();
    m_ui.cancelButton->setEnabled(false);
    m_ui.closeButton->setEnabled(true);
    updateInstallButton();
}

void InstallDialog::install()
{
    TRACE_OBJ
    QListWidgetItem *item = 0;
    for (int i=0; i<m_ui.listWidget->count(); ++i) {
        item = m_ui.listWidget->item(i);
        if (item->checkState() == Qt::Checked)
            m_itemsToInstall.append(item);
    }
    m_ui.installButton->setEnabled(false);
    downloadNextFile();
}

void InstallDialog::downloadNextFile()
{
    TRACE_OBJ
    if (!m_itemsToInstall.count()) {
        m_ui.cancelButton->setEnabled(false);
        m_ui.closeButton->setEnabled(true);
        m_ui.statusLabel->setText(tr("Done."));
        m_ui.progressBar->hide();
        updateDocItemList();
        updateInstallButton();
        return;
    }

    QListWidgetItem *item = m_itemsToInstall.dequeue();
    m_currentCheckSum = item->data(QCH_CHECKSUM).toString();
    QString fileName = item->data(QCH_FILENAME).toString();
    QString saveFileName = m_ui.pathLineEdit->text() + QDir::separator()
        + fileName;

    if (QFile::exists(saveFileName)
        && QMessageBox::information(this, m_windowTitle,
        tr("The file %1 already exists. Do you want to overwrite it?")
        .arg(saveFileName), QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) == QMessageBox::No) {
        installFile(saveFileName);
        downloadNextFile();
        return;        
    }

    m_file = new QFile(saveFileName);
    if (!m_file->open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        QMessageBox::information(this, m_windowTitle,
            tr("Unable to save the file %1: %2.")
            .arg(saveFileName).arg(m_file->errorString()));
        delete m_file;
        m_file = 0;
        downloadNextFile();
        return;
    }

    m_ui.statusLabel->setText(tr("Downloading %1...").arg(fileName));
    m_ui.progressBar->show();

    QLatin1String urlStr("http://qt.nokia.com/doc/assistantdocs/%1");
    QUrl url(QString(urlStr).arg(fileName));    
    
    m_httpAborted = false;
    m_docId = m_http->get(url.path(), m_file);
    
    m_ui.cancelButton->setEnabled(true);
    m_ui.closeButton->setEnabled(false);    
}

void InstallDialog::httpRequestFinished(int requestId, bool error)
{
    TRACE_OBJ
    if (requestId == m_docInfoId  && m_buffer) {        
        m_ui.progressBar->hide();
        if (error) {
            QMessageBox::information(this, m_windowTitle,
                tr("Download failed: %1.")
                .arg(m_http->errorString()));
        } else if (!m_httpAborted) {
            QStringList registeredDocs = m_helpEngine->registeredDocumentations();
            m_buffer->seek(0);
            while (m_buffer->canReadLine()) {
                QByteArray ba = m_buffer->readLine();
                QStringList lst = QString::fromAscii(ba.constData()).split(QLatin1Char('|'));
                if (lst.count() != 4) {
                    QMessageBox::information(this, m_windowTitle,
                        tr("Documentation info file is corrupt!"));
                } else {
                    QListWidgetItem *item = new QListWidgetItem(m_ui.listWidget);
                    item->setText(lst.at(2).trimmed());
                    item->setData(QCH_FILENAME, lst.first());
                    item->setData(QCH_NAMESPACE, lst.at(1));
                    item->setData(QCH_CHECKSUM, lst.last().trimmed());
                }
            }
            updateDocItemList();
        }
        if (m_buffer)
            m_buffer->close();
        delete m_buffer;
        m_buffer = 0;
        m_ui.statusLabel->setText(tr("Done."));
        m_ui.cancelButton->setEnabled(false);        
        m_ui.closeButton->setEnabled(true);
        updateInstallButton();
    } else if (requestId == m_docId) {        
        m_file->close();
        if (!m_httpAborted) {
            QString checkSum;
            if (m_file->open(QIODevice::ReadOnly)) {                
                QByteArray digest = QCryptographicHash::hash(m_file->readAll(),
                    QCryptographicHash::Md5);
                m_file->close();
                checkSum = QString::fromLatin1(digest.toHex());             
            }            
            if (error) {
                m_file->remove();
                QMessageBox::warning(this, m_windowTitle,
                    tr("Download failed: %1.")
                    .arg(m_http->errorString()));
            } else if (checkSum.isEmpty() || m_currentCheckSum != checkSum) {
                m_file->remove();
                QMessageBox::warning(this, m_windowTitle,
                    tr("Download failed: Downloaded file is corrupted."));
            } else {
                m_ui.statusLabel->setText(tr("Installing documentation %1...")
                    .arg(QFileInfo(m_file->fileName()).fileName()));
                m_ui.progressBar->setMaximum(0);
                m_ui.statusLabel->setText(tr("Done."));
                installFile(m_file->fileName());                
            }
        } else {
            m_file->remove();
        }
        delete m_file;
        m_file = 0;
        downloadNextFile();
    }
}

void InstallDialog::installFile(const QString &fileName)
{
    TRACE_OBJ
    if (m_helpEngine->registerDocumentation(fileName)) {
        m_installedDocumentations
            .append(QHelpEngineCore::namespaceName(fileName));
    } else {
        QMessageBox::information(this, m_windowTitle,
            tr("Error while installing documentation:\n%1")
            .arg(m_helpEngine->error()));
    }
}

void InstallDialog::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
    TRACE_OBJ
    if (responseHeader.statusCode() != 200) {
        QMessageBox::information(this, m_windowTitle,
            tr("Download failed: %1.")
            .arg(responseHeader.reasonPhrase()));
        m_httpAborted = true;
        m_ui.progressBar->hide();
        m_http->abort();
        return;
    }
}

void InstallDialog::updateDataReadProgress(int bytesRead, int totalBytes)
{
    TRACE_OBJ
    if (m_httpAborted)
        return;

    m_ui.progressBar->setMaximum(totalBytes);
    m_ui.progressBar->setValue(bytesRead);
}

void InstallDialog::browseDirectories()
{
    TRACE_OBJ
    QString dir = QFileDialog::getExistingDirectory(this, m_windowTitle,
        m_ui.pathLineEdit->text());
    if (!dir.isEmpty())
        m_ui.pathLineEdit->setText(dir);
}

#endif
QT_END_NAMESPACE
