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

#ifndef QHELPSEARCHQUERYWIDGET_H
#define QHELPSEARCHQUERYWIDGET_H

#include <QtHelp/qhelp_global.h>
#include <QtHelp/qhelpsearchengine.h>

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtGui/QWidget>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Help)

class QFocusEvent;
class QHelpSearchQueryWidgetPrivate;

class QHELP_EXPORT QHelpSearchQueryWidget : public QWidget
{
    Q_OBJECT

public:
    QHelpSearchQueryWidget(QWidget *parent = 0);
    ~QHelpSearchQueryWidget();

    void expandExtendedSearch();
    void collapseExtendedSearch();

    QList<QHelpSearchQuery> query() const;
    void setQuery(const QList<QHelpSearchQuery> &queryList);

Q_SIGNALS:
    void search();

private:
    virtual void focusInEvent(QFocusEvent *focusEvent);
    virtual void changeEvent(QEvent *event);

private:
    QHelpSearchQueryWidgetPrivate *d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif  // QHELPSEARCHQUERYWIDGET_H
