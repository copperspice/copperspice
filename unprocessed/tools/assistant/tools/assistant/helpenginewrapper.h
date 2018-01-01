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

#ifndef HELPENGINEWRAPPER_H
#define HELPENGINEWRAPPER_H

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>

QT_BEGIN_NAMESPACE

class QFileSystemWatcher;
class QHelpContentModel;
class QHelpContentWidget;
class QHelpIndexModel;
class QHelpIndexWidget;
class QHelpSearchEngine;

enum {
    ShowHomePage = 0,
    ShowBlankPage = 1,
    ShowLastPages = 2
};

class HelpEngineWrapperPrivate;
class TimeoutForwarder;

class HelpEngineWrapper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HelpEngineWrapper)
    friend class TimeoutForwarder;
public:
    static HelpEngineWrapper &instance(const QString &collectionFile = QString());
    static void removeInstance();

    // Forwarded help engine member functions, possibly enriched.
    QHelpSearchEngine *searchEngine() const;
    QHelpContentModel *contentModel() const;
    QHelpIndexModel *indexModel() const;
    QHelpContentWidget *contentWidget();
    QHelpIndexWidget *indexWidget();
    bool setupData();
    const QStringList registeredDocumentations() const;
    const QString collectionFile() const;
    bool registerDocumentation(const QString &docFile);
    bool unregisterDocumentation(const QString &namespaceName);
    bool addCustomFilter(const QString &filterName,
                         const QStringList &attributes);
    bool removeCustomFilter(const QString &filterName);
    void setCurrentFilter(const QString &filterName);
    const QString currentFilter() const;
    const QStringList customFilters() const;
    QUrl findFile(const QUrl &url) const;
    QByteArray fileData(const QUrl &url) const;
    QMap<QString, QUrl>	linksForIdentifier(const QString &id) const;
    const QStringList filterAttributes() const;
    const QStringList filterAttributes(const QString &filterName) const;
    QString	error() const;   

    /*
     * To be called after assistant has finished looking for new documentation.
     * This will mainly cause the search index to be updated, if necessary.
     */
    void initialDocSetupDone();

    const QStringList qtDocInfo(const QString &component) const;
    void setQtDocInfo(const QString &component, const QStringList &doc);

    const QString homePage() const;
    void setHomePage(const QString &page);
    const QString defaultHomePage() const;
    void setDefaultHomePage(const QString &page);

    int lastTabPage() const;
    void setLastTabPage(int lastPage);

    // TODO: Don't allow last pages and zoom factors to be set in isolation
    //       Perhaps also fill up missing elements automatically or assert.
    const QStringList lastShownPages() const;
    void setLastShownPages(const QStringList &lastShownPages);
    const QStringList lastZoomFactors() const;
    void setLastZoomFactors(const QStringList &lastZoomFactors);

    const QString cacheDir() const;
    bool cacheDirIsRelativeToCollection() const;
    void setCacheDir(const QString &cacheDir, bool relativeToCollection);

    bool filterFunctionalityEnabled() const;
    void setFilterFunctionalityEnabled(bool enabled);

    bool filterToolbarVisible() const;
    void setFilterToolbarVisible(bool visible);

    bool addressBarEnabled() const;
    void setAddressBarEnabled(bool enabled);

    bool addressBarVisible() const;
    void setAddressBarVisible(bool visible);

    bool documentationManagerEnabled() const;
    void setDocumentationManagerEnabled(bool enabled);

    const QByteArray aboutMenuTexts() const;
    void setAboutMenuTexts(const QByteArray &texts);
    const QByteArray aboutTexts() const;
    void setAboutTexts(const QByteArray &texts);
    const QByteArray aboutIcon() const;
    void setAboutIcon(const QByteArray &icon);
    const QByteArray aboutImages() const;
    void setAboutImages(const QByteArray &images);

    const QString windowTitle() const;
    void setWindowTitle(const QString &windowTitle);

    const QByteArray applicationIcon() const;
    void setApplicationIcon(const QByteArray &icon);

    const QByteArray mainWindow() const;
    void setMainWindow(const QByteArray &mainWindow);
    const QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geometry);

    const QByteArray bookmarks() const;
    void setBookmarks(const QByteArray &bookmarks);

    int startOption() const;
    void setStartOption(int option);

    bool hasFontSettings() const;
    bool usesAppFont() const;
    void setUseAppFont(bool useAppFont);
    bool usesBrowserFont() const;
    void setUseBrowserFont(bool useBrowserFont);
    const QFont appFont() const;
    void setAppFont(const QFont &font);
    QFontDatabase::WritingSystem appWritingSystem() const;
    void setAppWritingSystem(QFontDatabase::WritingSystem system);
    const QFont browserFont() const;
    void setBrowserFont(const QFont &font);
    QFontDatabase::WritingSystem browserWritingSystem() const;
    void setBrowserWritingSystem(QFontDatabase::WritingSystem system);

    bool showTabs() const;
    void setShowTabs(bool show);

    static const QString TrUnfiltered;

    bool fullTextSearchFallbackEnabled() const;

signals:

    // For asynchronous doc updates triggered by external actions.
    void documentationRemoved(const QString &namespaceName);
    void documentationUpdated(const QString &namespaceName);

    // Forwarded from QHelpEngineCore.
    void currentFilterChanged(const QString &currentFilter);
    void setupFinished();

private slots:
    void handleCurrentFilterChanged(const QString &filter);

private:
    HelpEngineWrapper(const QString &collectionFile);
    ~HelpEngineWrapper();

    static HelpEngineWrapper *helpEngineWrapper;

    HelpEngineWrapperPrivate *d;
};

QT_END_NAMESPACE

#endif // HELPENGINEWRAPPER_H
