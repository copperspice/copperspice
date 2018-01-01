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

#include "helpviewer.h"
#include "helpviewer_p.h"

#include "helpenginewrapper.h"
#include "tracer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QStringBuilder>
#include <QtCore/QTemporaryFile>
#include <QtCore/QUrl>

#include <QtGui/QDesktopServices>
#include <QtGui/QMouseEvent>

#include <QtHelp/QHelpEngineCore>

QT_BEGIN_NAMESPACE

const QString HelpViewer::DocPath = QLatin1String("qthelp://com.trolltech.");

const QString HelpViewer::AboutBlank =
    QCoreApplication::translate("HelpViewer", "<title>about:blank</title>");

const QString HelpViewer::LocalHelpFile = QLatin1String("qthelp://"
    "com.trolltech.com.assistantinternal-1.0.0/assistant/assistant.html");

const QString HelpViewer::PageNotFoundMessage =
    QCoreApplication::translate("HelpViewer", "<title>Error 404...</title><div "
    "align=\"center\"><br><br><h1>The page could not be found</h1><br><h3>'%1'"
    "</h3></div>");

struct ExtensionMap {
    const char *extension;
    const char *mimeType;
} extensionMap[] = {
    { ".bmp", "image/bmp" },
    { ".css", "text/css" },
    { ".gif", "image/gif" },
    { ".html", "text/html" },
    { ".htm", "text/html" },
    { ".ico", "image/x-icon" },
    { ".jpeg", "image/jpeg" },
    { ".jpg", "image/jpeg" },
    { ".js", "application/x-javascript" },
    { ".mng", "video/x-mng" },
    { ".pbm", "image/x-portable-bitmap" },
    { ".pgm", "image/x-portable-graymap" },
    { ".pdf", "application/pdf" },
    { ".png", "image/png" },
    { ".ppm", "image/x-portable-pixmap" },
    { ".rss", "application/rss+xml" },
    { ".svg", "image/svg+xml" },
    { ".svgz", "image/svg+xml" },
    { ".text", "text/plain" },
    { ".tif", "image/tiff" },
    { ".tiff", "image/tiff" },
    { ".txt", "text/plain" },
    { ".xbm", "image/x-xbitmap" },
    { ".xml", "text/xml" },
    { ".xpm", "image/x-xpm" },
    { ".xsl", "text/xsl" },
    { ".xhtml", "application/xhtml+xml" },
    { ".wml", "text/vnd.wap.wml" },
    { ".wmlc", "application/vnd.wap.wmlc" },
    { "about:blank", 0 },
    { 0, 0 }
};

HelpViewer::~HelpViewer()
{
    TRACE_OBJ
    delete d;
}

bool HelpViewer::isLocalUrl(const QUrl &url)
{
    TRACE_OBJ
    const QString &scheme = url.scheme();
    return scheme.isEmpty()
        || scheme == QLatin1String("file")
        || scheme == QLatin1String("qrc")
        || scheme == QLatin1String("data")
        || scheme == QLatin1String("qthelp")
        || scheme == QLatin1String("about");
}

bool HelpViewer::canOpenPage(const QString &url)
{
    TRACE_OBJ
    return !mimeFromUrl(url).isEmpty();
}

QString HelpViewer::mimeFromUrl(const QUrl &url)
{
    TRACE_OBJ
    const QString &path = url.path();
    const int index = path.lastIndexOf(QLatin1Char('.'));
    const QByteArray &ext = path.mid(index).toUtf8().toLower();

    const ExtensionMap *e = extensionMap;
    while (e->extension) {
        if (ext == e->extension)
            return QLatin1String(e->mimeType);
        ++e;
    }
    return QLatin1String("");
}

bool HelpViewer::launchWithExternalApp(const QUrl &url)
{
    TRACE_OBJ
    if (isLocalUrl(url)) {
        const HelpEngineWrapper &helpEngine = HelpEngineWrapper::instance();
        const QUrl &resolvedUrl = helpEngine.findFile(url);
        if (!resolvedUrl.isValid())
            return false;

        const QString& path = resolvedUrl.path();
        if (!canOpenPage(path)) {
            QTemporaryFile tmpTmpFile;
            if (!tmpTmpFile.open())
                return false;

            const QString &extension = QFileInfo(path).completeSuffix();
            QFile actualTmpFile(tmpTmpFile.fileName() % QLatin1String(".")
                % extension);
            if (!actualTmpFile.open(QIODevice::ReadWrite | QIODevice::Truncate))
                return false;

            actualTmpFile.write(helpEngine.fileData(resolvedUrl));
            actualTmpFile.close();
            return QDesktopServices::openUrl(QUrl(actualTmpFile.fileName()));
        }
    } else if (url.scheme() == QLatin1String("http")) {
        return QDesktopServices::openUrl(url);
    }
    return false;
}

// -- public slots

void HelpViewer::home()
{
    TRACE_OBJ
    setSource(HelpEngineWrapper::instance().homePage());
}

// -- private slots

void HelpViewer::setLoadStarted()
{
    d->m_loadFinished = false;
}

void HelpViewer::setLoadFinished(bool ok)
{
    d->m_loadFinished = ok;
    emit sourceChanged(source());
}

// -- private

bool HelpViewer::handleForwardBackwardMouseButtons(QMouseEvent *event)
{
    TRACE_OBJ
    if (event->button() == Qt::XButton1) {
        backward();
        return true;
    }

    if (event->button() == Qt::XButton2) {
        forward();
        return true;
    }

    return false;
}

QT_END_NAMESPACE
