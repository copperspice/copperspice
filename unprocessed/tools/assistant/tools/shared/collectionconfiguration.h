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

#ifndef COLLECTIONCONFIGURATION_H
#define COLLECTIONCONFIGURATION_H

#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QHelpEngineCore;

class CollectionConfiguration
{
public:
    static const QString windowTitle(const QHelpEngineCore &helpEngine);
    static void setWindowTitle(QHelpEngineCore &helpEngine,
                               const QString &windowTitle);

    static const QString cacheDir(const QHelpEngineCore &helpEngine);
    static bool cacheDirIsRelativeToCollection(const QHelpEngineCore &helpEngine);
    static void setCacheDir(QHelpEngineCore &helpEngine,
                            const QString &cacheDir, bool relativeToCollection);

    static uint creationTime(const QHelpEngineCore &helpEngine);
    static void setCreationTime(QHelpEngineCore &helpEngine, uint time);

    static bool filterFunctionalityEnabled(const QHelpEngineCore &helpEngine);
    static void setFilterFunctionalityEnabled(QHelpEngineCore &helpEngine,
                                              bool enabled);

    static bool filterToolbarVisible(const QHelpEngineCore &helpEngine);
    static void setFilterToolbarVisible(QHelpEngineCore &helpEngine,
                                        bool visible);

    static bool addressBarEnabled(const QHelpEngineCore &helpEngine);
    static void setAddressBarEnabled(QHelpEngineCore &helpEngine, bool enabled);

    static bool addressBarVisible(const QHelpEngineCore &helpEngine);
    static void setAddressBarVisible(QHelpEngineCore &helpEngine, bool visible);


    static bool documentationManagerEnabled(const QHelpEngineCore &helpEngine);
    static void setDocumentationManagerEnabled(QHelpEngineCore &helpEngine,
                                               bool enabled);

    static const QByteArray applicationIcon(const QHelpEngineCore &helpEngine);
    static void setApplicationIcon(QHelpEngineCore &helpEngine,
                                   const QByteArray &icon);

    // TODO: Encapsulate encoding from/to QByteArray here
    static const QByteArray aboutMenuTexts(const QHelpEngineCore &helpEngine);
    static void setAboutMenuTexts(QHelpEngineCore &helpEngine,
                                  const QByteArray &texts);

    static const QByteArray aboutIcon(const QHelpEngineCore &helpEngine);
    static void setAboutIcon(QHelpEngineCore &helpEngine,
                             const QByteArray &icon);

    // TODO: Encapsulate encoding from/to QByteArray here
    static const QByteArray aboutTexts(const QHelpEngineCore &helpEngine);
    static void setAboutTexts(QHelpEngineCore &helpEngine,
                              const QByteArray &texts);

    static const QByteArray aboutImages(const QHelpEngineCore &helpEngine);
    static void setAboutImages(QHelpEngineCore &helpEngine,
                               const QByteArray &images);

    static const QString defaultHomePage(const QHelpEngineCore &helpEngine);
    static void setDefaultHomePage(QHelpEngineCore &helpEngine,
                                   const QString &page);

    // TODO: Don't allow last pages and zoom factors to be set in isolation
    //       Perhaps also fill up missing elements automatically or assert.
    static const QStringList lastShownPages(const QHelpEngineCore &helpEngine);
    static void setLastShownPages(QHelpEngineCore &helpEngine,
                                  const QStringList &lastShownPages);
    static const QStringList lastZoomFactors(const QHelpEngineCore &helpEngine);
    static void setLastZoomFactors(QHelpEngineCore &helPEngine,
                                   const QStringList &lastZoomFactors);

    static int lastTabPage(const QHelpEngineCore &helpEngine);
    static void setLastTabPage(QHelpEngineCore &helpEngine, int lastPage);

    static bool isNewer(const QHelpEngineCore &newer,
                        const QHelpEngineCore &older);
    static void copyConfiguration(const QHelpEngineCore &source,
                                  QHelpEngineCore &target);

    /*
     * Note that this only reflects register actions caused by the
     * "-register" command line switch, not GUI or remote control actions.
     */
    static const QDateTime lastRegisterTime(const QHelpEngineCore &helpEngine);
    static void updateLastRegisterTime(QHelpEngineCore &helpEngine);

    static bool fullTextSearchFallbackEnabled(const QHelpEngineCore &helpEngine);
    static void setFullTextSearchFallbackEnabled(QHelpEngineCore &helpEngine,
        bool on);

    static const QString DefaultZoomFactor;
    static const QString ListSeparator;
};

QT_END_NAMESPACE

#endif // COLLECTIONCONFIGURATION_H
