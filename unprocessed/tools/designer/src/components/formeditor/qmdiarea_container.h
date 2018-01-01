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

#ifndef QMDIAREA_CONTAINER_H
#define QMDIAREA_CONTAINER_H

#include <QtDesigner/QDesignerContainerExtension>


#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>

#include <QtGui/QMdiArea>
#include <QtGui/QWorkspace>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// Container for QMdiArea
class QMdiAreaContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QMdiAreaContainer(QMdiArea *widget, QObject *parent = 0);

    virtual int count() const;
    virtual QWidget *widget(int index) const;
    virtual int currentIndex() const;
    virtual void setCurrentIndex(int index);
    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int index);

    // Semismart positioning of a new MDI child after cascading
    static void positionNewMdiChild(const QWidget *area, QWidget *mdiChild);

private:
    QMdiArea *m_mdiArea;
};

// PropertySheet for QMdiArea: Fakes window title and name.
// Also works for a QWorkspace as it relies on the container extension.

class QMdiAreaPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit QMdiAreaPropertySheet(QWidget *mdiArea, QObject *parent = 0);

    virtual void setProperty(int index, const QVariant &value);
    virtual bool reset(int index);
    virtual bool isEnabled(int index) const;
    virtual bool isChanged(int index) const;
    virtual QVariant property(int index) const;

    // Check whether the property is to be saved. Returns false for the page
    // properties (as the property sheet has no concept of 'stored')
    static bool checkProperty(const QString &propertyName);

private:
    const QString m_windowTitleProperty;
    QWidget *currentWindow() const;
    QDesignerPropertySheetExtension *currentWindowSheet() const;

    enum MdiAreaProperty { MdiAreaSubWindowName, MdiAreaSubWindowTitle, MdiAreaNone };
    static MdiAreaProperty mdiAreaProperty(const QString &name);
};

// Factories

typedef ExtensionFactory<QDesignerContainerExtension,  QMdiArea,  QMdiAreaContainer> QMdiAreaContainerFactory;
typedef QDesignerPropertySheetFactory<QMdiArea, QMdiAreaPropertySheet> QMdiAreaPropertySheetFactory;
typedef QDesignerPropertySheetFactory<QWorkspace, QMdiAreaPropertySheet> QWorkspacePropertySheetFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QMDIAREA_CONTAINER_H
