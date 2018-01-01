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
#include <QtCore/QFileInfo>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtCore/QUrl>

#include <QtGui/QApplication>
#include <QtGui/QDesktopServices>

#include <QtHelp/QHelpEngine>
#include <QtHelp/QHelpSearchEngine>

#include <QtNetwork/QLocalSocket>

#include <QtSql/QSqlDatabase>

#include "../shared/collectionconfiguration.h"
#include "helpenginewrapper.h"
#include "mainwindow.h"
#include "cmdlineparser.h"

// #define TRACING_REQUESTED

QT_USE_NAMESPACE

#if defined(USE_STATIC_SQLITE_PLUGIN)
  #include <QtPlugin>
  Q_IMPORT_PLUGIN(qsqlite)
#endif

namespace {

void
updateLastPagesOnUnregister(QHelpEngineCore& helpEngine, const QString& nsName)
{
    TRACE_OBJ
    int lastPage = CollectionConfiguration::lastTabPage(helpEngine);
    QStringList currentPages = CollectionConfiguration::lastShownPages(helpEngine);
    if (!currentPages.isEmpty()) {
        QStringList zoomList = CollectionConfiguration::lastZoomFactors(helpEngine);
        while (zoomList.count() < currentPages.count())
            zoomList.append(CollectionConfiguration::DefaultZoomFactor);

        for (int i = currentPages.count(); --i >= 0;) {
            if (QUrl(currentPages.at(i)).host() == nsName) {
                zoomList.removeAt(i);
                currentPages.removeAt(i);
                lastPage = (lastPage == (i + 1)) ? 1 : lastPage;
            }
        }

        CollectionConfiguration::setLastShownPages(helpEngine, currentPages);
        CollectionConfiguration::setLastTabPage(helpEngine, lastPage);
        CollectionConfiguration::setLastZoomFactors(helpEngine, zoomList);
    }
}

bool
updateUserCollection(QHelpEngineCore& user, const QHelpEngineCore& caller)
{
    TRACE_OBJ
    if (!CollectionConfiguration::isNewer(caller, user))
        return false;
    CollectionConfiguration::copyConfiguration(caller, user);
    return true;
}

void stripNonexistingDocs(QHelpEngineCore& collection)
{
    TRACE_OBJ
    const QStringList &namespaces = collection.registeredDocumentations();
    foreach (const QString &ns, namespaces) {
        QFileInfo fi(collection.documentationFileName(ns));
        if (!fi.exists() || !fi.isFile())
            collection.unregisterDocumentation(ns);
    }
}

QString indexFilesFolder(const QString &collectionFile)
{
    TRACE_OBJ
    QString indexFilesFolder = QLatin1String(".fulltextsearch");
    if (!collectionFile.isEmpty()) {
        QFileInfo fi(collectionFile);
        indexFilesFolder = QLatin1Char('.') +
            fi.fileName().left(fi.fileName().lastIndexOf(QLatin1String(".qhc")));
    }
    return indexFilesFolder;
}

/*
 * Returns the expected absolute file path of the cached collection file
 * correspondinging to the given collection's file.
 * It may or may not exist yet.
 */
QString constructCachedCollectionFilePath(const QHelpEngineCore &collection)
{
    TRACE_OBJ
    const QString &filePath = collection.collectionFile();
    const QString &fileName = QFileInfo(filePath).fileName();
    const QString &cacheDir = CollectionConfiguration::cacheDir(collection);
    const QString &dir = !cacheDir.isEmpty()
        && CollectionConfiguration::cacheDirIsRelativeToCollection(collection)
            ? QFileInfo(filePath).dir().absolutePath()
                + QDir::separator() + cacheDir
            : MainWindow::collectionFileDirectory(false, cacheDir);
    return dir + QDir::separator() + fileName;
}

bool synchronizeDocs(QHelpEngineCore &collection,
                     QHelpEngineCore &cachedCollection,
                     CmdLineParser &cmd)
{
    TRACE_OBJ
    const QDateTime &lastCollectionRegisterTime =
        CollectionConfiguration::lastRegisterTime(collection);
    if (!lastCollectionRegisterTime.isValid() || lastCollectionRegisterTime
        < CollectionConfiguration::lastRegisterTime(cachedCollection))
        return true;

    const QStringList &docs = collection.registeredDocumentations();
    const QStringList &cachedDocs = cachedCollection.registeredDocumentations();

    /*
     * Ensure that the cached collection contains all docs that
     * the collection contains.
     */
    foreach (const QString &doc, docs) {
        if (!cachedDocs.contains(doc)) {
            const QString &docFile = collection.documentationFileName(doc);
            if (!cachedCollection.registerDocumentation(docFile)) {
                cmd.showMessage(QCoreApplication::translate("Assistant",
                                    "Error registering documentation file '%1': %2").
                                arg(docFile).arg(cachedCollection.error()), true);
                return false;
            }
        }
    }

    CollectionConfiguration::updateLastRegisterTime(cachedCollection);

    return true;
}

bool removeSearchIndex(const QString &collectionFile)
{
    TRACE_OBJ
    QString path = QFileInfo(collectionFile).path();
    path += QLatin1Char('/') + indexFilesFolder(collectionFile);

    QLocalSocket localSocket;
    localSocket.connectToServer(QString(QLatin1String("QtAssistant%1"))
                                .arg(QLatin1String(QT_VERSION_STR)));

    QDir dir(path); // check if there is no other instance ruinning
    if (!dir.exists() || localSocket.waitForConnected())
        return false;

    QStringList lst = dir.entryList(QDir::Files | QDir::Hidden);
    foreach (const QString &item, lst)
        dir.remove(item);
    return true;
}

bool rebuildSearchIndex(QCoreApplication &app, const QString &collectionFile,
                        CmdLineParser &cmd)
{
    TRACE_OBJ
    QHelpEngine engine(collectionFile);
    if (!engine.setupData()) {
        cmd.showMessage(QCoreApplication::translate("Assistant", "Error: %1")
                        .arg(engine.error()), true);
        return false;
    }

    QHelpSearchEngine * const searchEngine = engine.searchEngine();
    QObject::connect(searchEngine, SIGNAL(indexingFinished()), &app,
                     SLOT(quit()));
    searchEngine->reindexDocumentation();
    return app.exec() == 0;
}

bool useGui(int argc, char *argv[])
{
    TRACE_OBJ
    bool gui = true;
#ifndef Q_OS_WIN
    // Look for arguments that imply command-line mode.
    const char * cmdModeArgs[] = {
        "-help", "-register", "-unregister", "-remove-search-index",
        "-rebuild-search-index"
    };
    for (int i = 1; i < argc; ++i) {
        for (size_t j = 0; j < sizeof cmdModeArgs/sizeof *cmdModeArgs; ++j) {
            if(strcmp(argv[i], cmdModeArgs[j]) == 0) {
                gui = false;
                break;
            }
        }
    }
#else
    Q_UNUSED(argc)
    Q_UNUSED(argv)
#endif
    return gui;
}

bool registerDocumentation(QHelpEngineCore &collection, CmdLineParser &cmd,
                           bool printSuccess)
{
    TRACE_OBJ
    if (!collection.registerDocumentation(cmd.helpFile())) {
        cmd.showMessage(QCoreApplication::translate("Assistant",
                     "Could not register documentation file\n%1\n\nReason:\n%2")
                     .arg(cmd.helpFile()).arg(collection.error()), true);
        return false;
    }
    if (printSuccess)
        cmd.showMessage(QCoreApplication::translate("Assistant",
                            "Documentation successfully registered."),
                        false);
    CollectionConfiguration::updateLastRegisterTime(collection);
    return true;
}

bool unregisterDocumentation(QHelpEngineCore &collection,
    const QString &namespaceName, CmdLineParser &cmd, bool printSuccess)
{
    TRACE_OBJ
    if (!collection.unregisterDocumentation(namespaceName)) {
        cmd.showMessage(QCoreApplication::translate("Assistant",
                             "Could not unregister documentation"
                             " file\n%1\n\nReason:\n%2").
                        arg(cmd.helpFile()).arg(collection.error()), true);
        return false;
    }
    updateLastPagesOnUnregister(collection, namespaceName);
    if (printSuccess)
        cmd.showMessage(QCoreApplication::translate("Assistant",
                            "Documentation successfully unregistered."),
                        false);
    return true;
}

void setupTranslation(const QString &fileName, const QString &dir)
{
    QTranslator *translator = new QTranslator(QCoreApplication::instance());
    if (translator->load(fileName, dir)) {
        QCoreApplication::installTranslator(translator);
    } else if (!fileName.endsWith(QLatin1String("en_US"))
            && !fileName.endsWith(QLatin1String("_C"))) {
        qWarning("Could not load translation file %s in directory %s.",
                 qPrintable(fileName), qPrintable(dir));
    }
}

void setupTranslations()
{
    TRACE_OBJ
    const QString& locale = QLocale::system().name();
    const QString &resourceDir
        = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    setupTranslation(QLatin1String("assistant_") + locale, resourceDir);
    setupTranslation(QLatin1String("qt_") + locale, resourceDir);
    setupTranslation(QLatin1String("qt_help_") + locale, resourceDir);
}

} // Anonymous namespace.

int main(int argc, char *argv[])
{
    TRACE_OBJ
    QApplication a(argc, argv, useGui(argc, argv));
    a.addLibraryPath(a.applicationDirPath() + QLatin1String("/plugins"));
    setupTranslations();

    // Parse arguments.
    CmdLineParser cmd(a.arguments());
    CmdLineParser::Result res = cmd.parse();
    if (res == CmdLineParser::Help)
        return 0;
    else if (res == CmdLineParser::Error)
        return -1;

    /*
     * Create the collection objects that we need. We always have the
     * cached collection file. Depending on whether the user specified
     * one, we also may have an input collection file.
     */
    const QString collectionFile = cmd.collectionFile();
    const bool collectionFileGiven = !collectionFile.isEmpty();
    QScopedPointer<QHelpEngineCore> collection;
    if (collectionFileGiven) {
        collection.reset(new QHelpEngineCore(collectionFile));
        if (!collection->setupData()) {
            cmd.showMessage(QCoreApplication::translate("Assistant",
                                "Error reading collection file '%1': %2.").
                arg(collectionFile).arg(collection->error()), true);
            return EXIT_FAILURE;
        }
    }
    const QString &cachedCollectionFile = collectionFileGiven
        ? constructCachedCollectionFilePath(*collection)
        : MainWindow::defaultHelpCollectionFileName();
    if (collectionFileGiven && !QFileInfo(cachedCollectionFile).exists()
        && !collection->copyCollectionFile(cachedCollectionFile)) {
        cmd.showMessage(QCoreApplication::translate("Assistant",
                            "Error creating collection file '%1': %2.").
                arg(cachedCollectionFile).arg(collection->error()), true);
        return EXIT_FAILURE;
    }
    QHelpEngineCore cachedCollection(cachedCollectionFile);
    if (!cachedCollection.setupData()) {
        cmd.showMessage(QCoreApplication::translate("Assistant",
                            "Error reading collection file '%1': %2.").
                        arg(cachedCollectionFile).
                        arg(cachedCollection.error()), true);
        return EXIT_FAILURE;
    }

    stripNonexistingDocs(cachedCollection);
    if (collectionFileGiven) {
        if (CollectionConfiguration::isNewer(*collection, cachedCollection))
            CollectionConfiguration::copyConfiguration(*collection,
                                                       cachedCollection);
        if (!synchronizeDocs(*collection, cachedCollection, cmd))
            return EXIT_FAILURE;
    }

    if (cmd.registerRequest() != CmdLineParser::None) {
        const QStringList &cachedDocs =
            cachedCollection.registeredDocumentations();
        const QString &namespaceName =
            QHelpEngineCore::namespaceName(cmd.helpFile());
        if (cmd.registerRequest() == CmdLineParser::Register) {
            if (collectionFileGiven
                && !registerDocumentation(*collection, cmd, true))
                return EXIT_FAILURE;
            if (!cachedDocs.contains(namespaceName)
                && !registerDocumentation(cachedCollection, cmd, !collectionFileGiven))
                return EXIT_FAILURE;
            return EXIT_SUCCESS;
        }
        if (cmd.registerRequest() == CmdLineParser::Unregister) {
            if (collectionFileGiven
                && !unregisterDocumentation(*collection, namespaceName, cmd, true))
                return EXIT_FAILURE;
            if (cachedDocs.contains(namespaceName)
                && !unregisterDocumentation(cachedCollection, namespaceName,
                                            cmd, !collectionFileGiven))
                return EXIT_FAILURE;
            return EXIT_SUCCESS;
        }
    }

    if (cmd.removeSearchIndex()) {
        return removeSearchIndex(cachedCollectionFile)
            ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (cmd.rebuildSearchIndex()) {
        return rebuildSearchIndex(a, cachedCollectionFile, cmd)
            ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (!QSqlDatabase::isDriverAvailable(QLatin1String("QSQLITE"))) {
        cmd.showMessage(QCoreApplication::translate("Assistant",
                            "Cannot load sqlite database driver!"),
                        true);
        return EXIT_FAILURE;
    }

    if (!cmd.currentFilter().isEmpty()) {
        if (collectionFileGiven)
            collection->setCurrentFilter(cmd.currentFilter());
        cachedCollection.setCurrentFilter(cmd.currentFilter());
    }

    if (collectionFileGiven)
        cmd.setCollectionFile(cachedCollectionFile);

    MainWindow *w = new MainWindow(&cmd);
    w->show();
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    /*
     * We need to be careful here: The main window has to be deleted before
     * the help engine wrapper, which has to be deleted before the
     * QApplication.
     */
    const int retval = a.exec();
    delete w;
    HelpEngineWrapper::removeInstance();
    return retval;
}
