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

#ifndef DEFAULT_CONTAINER_H
#define DEFAULT_CONTAINER_H

#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/extension.h>
#include <extensionfactory_p.h>

#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBox>
#include <QtGui/QScrollArea>
#include <QtGui/QDockWidget>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// ------------ QStackedWidgetContainer
class QStackedWidgetContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QStackedWidgetContainer(QStackedWidget *widget, QObject *parent = 0);

    virtual int count() const { return m_widget->count(); }
    virtual QWidget *widget(int index) const { return m_widget->widget(index); }

    virtual int currentIndex() const { return m_widget->currentIndex(); }
    virtual void setCurrentIndex(int index);

    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int index);

private:
    QStackedWidget *m_widget;
};

// ------------ QTabWidgetContainer
class QTabWidgetContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QTabWidgetContainer(QTabWidget *widget, QObject *parent = 0);

    virtual int count() const { return m_widget->count(); }
    virtual QWidget *widget(int index) const { return m_widget->widget(index); }

    virtual int currentIndex() const { return m_widget->currentIndex(); }
    virtual void setCurrentIndex(int index);

    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int index);

private:
    QTabWidget *m_widget;
};

// ------------  QToolBoxContainer
class QToolBoxContainer: public QObject, public QDesignerContainerExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QToolBoxContainer(QToolBox *widget, QObject *parent = 0);

    virtual int count() const { return m_widget->count(); }
    virtual QWidget *widget(int index) const { return m_widget->widget(index); }

    virtual int currentIndex() const { return m_widget->currentIndex(); }
    virtual void setCurrentIndex(int index);

    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int index);

private:
    QToolBox *m_widget;
};

// ------------ SingleChildContainer:
//  Template for containers that have a single child widget using widget()/setWidget().

template <class Container>
class SingleChildContainer: public QDesignerContainerExtension
{
protected:
    explicit SingleChildContainer(Container *widget, bool active = true);
public:
    virtual int count() const;
    virtual QWidget *widget(int index) const;
    virtual int currentIndex() const;
    virtual void setCurrentIndex(int /*index*/) {}
    virtual void addWidget(QWidget *widget);
    virtual void insertWidget(int index, QWidget *widget);
    virtual void remove(int /*index*/) {}

private:
    const bool m_active;
    Container *m_container;
};

template <class Container>
SingleChildContainer<Container>::SingleChildContainer(Container *widget, bool active) :
    m_active(active),
    m_container(widget)
{
}

template <class Container>
int SingleChildContainer<Container>::count() const
{
    return  m_active && m_container->widget() ? 1 : 0;
}

template <class Container>
QWidget *SingleChildContainer<Container>::widget(int /* index */) const
{
    return m_container->widget();
}

template <class Container>
int SingleChildContainer<Container>::currentIndex() const
{
    return m_active && m_container->widget() ? 0 : -1;
}

template <class Container>
void SingleChildContainer<Container>::addWidget(QWidget *widget)
{
    Q_ASSERT(m_container->widget() == 0);
    widget->setParent(m_container);
    m_container->setWidget(widget);
}

template <class Container>
void SingleChildContainer<Container>::insertWidget(int /* index */, QWidget *widget)
{
    addWidget(widget);
}

// ------------  QScrollAreaContainer
class QScrollAreaContainer: public QObject, public SingleChildContainer<QScrollArea>
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QScrollAreaContainer(QScrollArea *widget, QObject *parent = 0);
};

// --------------- QDockWidgetContainer
class QDockWidgetContainer: public QObject, public SingleChildContainer<QDockWidget>
{
    Q_OBJECT
    Q_INTERFACES(QDesignerContainerExtension)
public:
    explicit QDockWidgetContainer(QDockWidget *widget, QObject *parent = 0);
};

typedef ExtensionFactory<QDesignerContainerExtension, QStackedWidget, QStackedWidgetContainer> QDesignerStackedWidgetContainerFactory;
typedef ExtensionFactory<QDesignerContainerExtension, QTabWidget, QTabWidgetContainer> QDesignerTabWidgetContainerFactory;
typedef ExtensionFactory<QDesignerContainerExtension, QToolBox, QToolBoxContainer> QDesignerToolBoxContainerFactory;
typedef ExtensionFactory<QDesignerContainerExtension, QScrollArea, QScrollAreaContainer> QScrollAreaContainerFactory;
typedef ExtensionFactory<QDesignerContainerExtension,  QDockWidget, QDockWidgetContainer> QDockWidgetContainerFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // DEFAULT_CONTAINER_H
