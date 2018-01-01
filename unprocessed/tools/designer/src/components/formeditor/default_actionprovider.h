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

#ifndef DEFAULT_ACTIONPROVIDER_H
#define DEFAULT_ACTIONPROVIDER_H

#include "formeditor_global.h"
#include "actionprovider_p.h"
#include <extensionfactory_p.h>

#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QToolBar>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class FormWindow;

class QT_FORMEDITOR_EXPORT ActionProviderBase: public QDesignerActionProviderExtension
{
protected:
    explicit ActionProviderBase(QWidget *widget);

public:
    virtual void adjustIndicator(const QPoint &pos);
    virtual Qt::Orientation orientation() const = 0;

protected:
    virtual QRect indicatorGeometry(const QPoint &pos, Qt::LayoutDirection layoutDirection) const;

private:
    QWidget *m_indicator;
};

class QT_FORMEDITOR_EXPORT QToolBarActionProvider: public QObject, public ActionProviderBase
{
    Q_OBJECT
    Q_INTERFACES(QDesignerActionProviderExtension)
public:
    explicit QToolBarActionProvider(QToolBar *widget, QObject *parent = 0);

    virtual QRect actionGeometry(QAction *action) const;
    virtual QAction *actionAt(const QPoint &pos) const;
    Qt::Orientation orientation() const;

protected:
    virtual QRect indicatorGeometry(const QPoint &pos, Qt::LayoutDirection layoutDirection) const;

private:
    QToolBar *m_widget;
};

class QT_FORMEDITOR_EXPORT QMenuBarActionProvider: public QObject, public ActionProviderBase
{
    Q_OBJECT
    Q_INTERFACES(QDesignerActionProviderExtension)
public:
    explicit QMenuBarActionProvider(QMenuBar *widget, QObject *parent = 0);

    virtual QRect actionGeometry(QAction *action) const;
    virtual QAction *actionAt(const QPoint &pos) const;
    Qt::Orientation orientation() const;

private:
    QMenuBar *m_widget;
};

class QT_FORMEDITOR_EXPORT QMenuActionProvider: public QObject, public ActionProviderBase
{
    Q_OBJECT
    Q_INTERFACES(QDesignerActionProviderExtension)
public:
    explicit QMenuActionProvider(QMenu *widget, QObject *parent = 0);

    virtual QRect actionGeometry(QAction *action) const;
    virtual QAction *actionAt(const QPoint &pos) const;
    Qt::Orientation orientation() const;

private:
    QMenu *m_widget;
};

typedef ExtensionFactory<QDesignerActionProviderExtension, QToolBar, QToolBarActionProvider> QToolBarActionProviderFactory;
typedef ExtensionFactory<QDesignerActionProviderExtension, QMenuBar, QMenuBarActionProvider> QMenuBarActionProviderFactory;
typedef ExtensionFactory<QDesignerActionProviderExtension, QMenu, QMenuActionProvider> QMenuActionProviderFactory;

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // DEFAULT_ACTIONPROVIDER_H
