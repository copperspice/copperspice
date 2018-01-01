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

#ifndef QHELPENGINE_P_H
#define QHELPENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QSqlQuery;

class QHelpEngineCore;
class QHelpDBReader;
class QHelpContentModel;
class QHelpContentWidget;
class QHelpIndexModel;
class QHelpIndexWidget;
class QHelpSearchEngine;
class QHelpCollectionHandler;

class QHelpEngineCorePrivate : public QObject
{
    Q_OBJECT

public:
    QHelpEngineCorePrivate();
    virtual ~QHelpEngineCorePrivate();

    virtual void init(const QString &collectionFile,
        QHelpEngineCore *helpEngineCore);

    void clearMaps();
    bool setup();

    QMap<QString, QHelpDBReader*> readerMap;
    QMap<QString, QHelpDBReader*> fileNameReaderMap;
    QMultiMap<QString, QHelpDBReader*> virtualFolderMap;
    QStringList orderedFileNameList;

    QHelpCollectionHandler *collectionHandler;
    QString currentFilter;
    QString error;
    bool needsSetup;
    bool autoSaveFilter;

protected:
    QHelpEngineCore *q;

private slots:
    void errorReceived(const QString &msg);
};


class QHelpEnginePrivate : public QHelpEngineCorePrivate
{
    Q_OBJECT

public:
    QHelpEnginePrivate();
    ~QHelpEnginePrivate();

    void init(const QString &collectionFile,
        QHelpEngineCore *helpEngineCore);

    QHelpContentModel *contentModel;
    QHelpContentWidget *contentWidget;

    QHelpIndexModel *indexModel;
    QHelpIndexWidget *indexWidget;

    QHelpSearchEngine *searchEngine;

    void stopDataCollection();

    friend class QHelpContentProvider;
    friend class QHelpContentModel;
    friend class QHelpIndexProvider;
    friend class QHelpIndexModel;

public slots:
    void setContentsWidgetBusy();
    void unsetContentsWidgetBusy();
    void setIndexWidgetBusy();
    void unsetIndexWidgetBusy();

private slots:
    void applyCurrentFilter();
};

QT_END_NAMESPACE

#endif
