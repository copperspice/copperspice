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

#ifndef QTDOCINSTALLER
#define QTDOCINSTALLER

#include <QtCore/QDir>
#include <QtCore/QMutex>
#include <QtCore/QPair>
#include <QtCore/QStringList>
#include <QtCore/QThread>

QT_BEGIN_NAMESPACE

class HelpEngineWrapper;

class QtDocInstaller : public QThread
{
    Q_OBJECT

public:
    typedef QPair<QString, QStringList> DocInfo;
    QtDocInstaller(const QList<DocInfo> &docInfos);
    ~QtDocInstaller();
    void installDocs();

signals:
    void qchFileNotFound(const QString &component);
    void registerDocumentation(const QString &component,
                               const QString &absFileName);
    void docsInstalled(bool newDocsInstalled);

private:
    void run();
    bool installDoc(const DocInfo &docInfo);

    bool m_abort;
    QMutex m_mutex;
    QStringList m_qchFiles;
    QDir m_qchDir;
    QList<DocInfo> m_docInfos;
};

QT_END_NAMESPACE

#endif
