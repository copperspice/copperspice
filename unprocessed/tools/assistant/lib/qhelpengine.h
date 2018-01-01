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

#ifndef QHELPENGINE_H
#define QHELPENGINE_H

#include <QtHelp/qhelpenginecore.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Help)

class QHelpContentModel;
class QHelpContentWidget;
class QHelpIndexModel;
class QHelpIndexWidget;
class QHelpEnginePrivate;
class QHelpSearchEngine;

class QHELP_EXPORT QHelpEngine : public QHelpEngineCore
{
    Q_OBJECT

public:
    explicit QHelpEngine(const QString &collectionFile, QObject *parent = 0);
    ~QHelpEngine();

    QHelpContentModel *contentModel() const;
    QHelpIndexModel *indexModel() const;

    QHelpContentWidget *contentWidget();
    QHelpIndexWidget *indexWidget();

    QHelpSearchEngine *searchEngine();

private:
    QHelpEnginePrivate *d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
