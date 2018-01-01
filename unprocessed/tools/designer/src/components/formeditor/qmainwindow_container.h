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

#ifndef QMAINWINDOW_CONTAINER_H
#define QMAINWINDOW_CONTAINER_H

#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QExtensionFactory>

#include <extensionfactory_p.h>

#include <QtGui/QMainWindow>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QMainWindowContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QMainWindowContainer(QMainWindow *widget, QObject *parent = 0);

    virtual int count() const;
    virtual QWidget *widget(int index) const;
    virtual int currentIndex() const;
    virtual void setCurrentIndex(int index);
    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int index);

private:
    QMainWindow *m_mainWindow;
    QList<QWidget*> m_widgets;
};

typedef ExtensionFactory<QDesignerContainerExtension, QMainWindow, QMainWindowContainer> QMainWindowContainerFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QMAINWINDOW_CONTAINER_H
