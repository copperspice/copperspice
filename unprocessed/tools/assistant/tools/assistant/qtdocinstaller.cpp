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

#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDateTime>
#include <QtCore/QFileSystemWatcher>
#include <QtHelp/QHelpEngineCore>
#include "helpenginewrapper.h"
#include "qtdocinstaller.h"

QT_BEGIN_NAMESPACE

QtDocInstaller::QtDocInstaller(const QList<DocInfo> &docInfos)
    : m_abort(false), m_docInfos(docInfos)
{
    TRACE_OBJ
}

QtDocInstaller::~QtDocInstaller()
{
    TRACE_OBJ
    if (!isRunning())
        return;
    m_mutex.lock();
    m_abort = true;
    m_mutex.unlock();
    wait();
}

void QtDocInstaller::installDocs()
{
    TRACE_OBJ
    start(LowPriority);
}

void QtDocInstaller::run()
{
    TRACE_OBJ
    m_qchDir = QLibraryInfo::location(QLibraryInfo::DocumentationPath)
        + QDir::separator() + QLatin1String("qch");
    m_qchFiles = m_qchDir.entryList(QStringList() << QLatin1String("*.qch"));

    bool changes = false;
    foreach (const DocInfo &docInfo, m_docInfos) {
        changes |= installDoc(docInfo);
        m_mutex.lock();
        if (m_abort) {
            m_mutex.unlock();
            return;
        }
        m_mutex.unlock();
    }
    emit docsInstalled(changes);
}

bool QtDocInstaller::installDoc(const DocInfo &docInfo)
{
    TRACE_OBJ
    const QString &component = docInfo.first;
    const QStringList &info = docInfo.second;
    QDateTime dt;
    if (!info.isEmpty() && !info.first().isEmpty())
        dt = QDateTime::fromString(info.first(), Qt::ISODate);

    QString qchFile;
    if (info.count() == 2)
        qchFile = info.last();

    if (m_qchFiles.isEmpty()) {
        emit qchFileNotFound(component);
        return false;
    }
    foreach (const QString &f, m_qchFiles) {
        if (f.startsWith(component)) {
            QFileInfo fi(m_qchDir.absolutePath() + QDir::separator() + f);
            if (dt.isValid() && fi.lastModified().toTime_t() == dt.toTime_t()
                && qchFile == fi.absoluteFilePath())
                return false;
            emit registerDocumentation(component, fi.absoluteFilePath());
            return true;
        }
    }

    emit qchFileNotFound(component);
    return false;
}

QT_END_NAMESPACE
