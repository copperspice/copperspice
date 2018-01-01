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

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QtCore/QUrl>
#include <QtCore/QPoint>

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QMouseEvent;
class QHelpSearchEngine;
class QHelpSearchResultWidget;

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QHelpSearchEngine *engine, QWidget *parent = 0);
    ~SearchWidget();

    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void requestShowLink(const QUrl &url);
    void requestShowLinkInNewTab(const QUrl &url);

private slots:
    void search() const;
    void searchingStarted();
    void searchingFinished(int hits);

private:
    bool eventFilter(QObject* o, QEvent *e);
    void keyPressEvent(QKeyEvent *keyEvent);
    void contextMenuEvent(QContextMenuEvent *contextMenuEvent);

private:
    int zoomCount;
    QHelpSearchEngine *searchEngine;
    QHelpSearchResultWidget *resultWidget;
};

QT_END_NAMESPACE

#endif  // SEARCHWIDGET_H
