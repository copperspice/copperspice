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

#ifndef QHELPSEARCHRESULTWIDGET_H
#define QHELPSEARCHRESULTWIDGET_H

#include <QtHelp/qhelpsearchengine.h>
#include <QtHelp/qhelp_global.h>

#include <QtCore/QUrl>
#include <QtCore/QPoint>
#include <QtCore/QObject>

#include <QtGui/QWidget>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Help)

class QHelpSearchResultWidgetPrivate;

class QHELP_EXPORT QHelpSearchResultWidget : public QWidget
{
    Q_OBJECT

public:
    ~QHelpSearchResultWidget();
    QUrl linkAt(const QPoint &point);

Q_SIGNALS:
    void requestShowLink(const QUrl &url);

private:
    friend class QHelpSearchEngine;

    QHelpSearchResultWidgetPrivate *d;
    QHelpSearchResultWidget(QHelpSearchEngine *engine);
    virtual void changeEvent(QEvent *event);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif  // QHELPSEARCHRESULTWIDGET_H
