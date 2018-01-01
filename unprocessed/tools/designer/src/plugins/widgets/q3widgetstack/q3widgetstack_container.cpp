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

#include "q3widgetstack_container.h"
#include "qdesigner_q3widgetstack_p.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

Q3WidgetStackContainer::Q3WidgetStackContainer(QDesignerQ3WidgetStack *widget, QObject *parent)
    : QObject(parent),
      m_widget(widget)
{}

int Q3WidgetStackContainer::count() const
{ return m_pages.count(); }

QWidget *Q3WidgetStackContainer::widget(int index) const
{
    if (index == -1)
        return 0;

    return m_pages.at(index);
}

int Q3WidgetStackContainer::currentIndex() const
{ return m_pages.indexOf(m_widget->visibleWidget()); }

void Q3WidgetStackContainer::setCurrentIndex(int index)
{ m_widget->raiseWidget(m_pages.at(index)); }

void Q3WidgetStackContainer::addWidget(QWidget *widget)
{
    m_pages.append(widget);
    m_widget->addWidget(widget);
}

void Q3WidgetStackContainer::insertWidget(int index, QWidget *widget)
{
    m_pages.insert(index, widget);
    m_widget->addWidget(widget);
    m_widget->setCurrentIndex(index);
}

void Q3WidgetStackContainer::remove(int index)
{
    int current = currentIndex();
    m_widget->removeWidget(m_pages.at(index));
    m_pages.removeAt(index);
    if (index == current) {
        if (count() > 0)
            m_widget->setCurrentIndex((index == count()) ? index-1 : index);
    } else if (index < current) {
        if (current > 0)
            m_widget->setCurrentIndex(current-1);
    }
}

Q3WidgetStackContainerFactory::Q3WidgetStackContainerFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *Q3WidgetStackContainerFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerContainerExtension))
        return 0;

    if (QDesignerQ3WidgetStack *w = qobject_cast<QDesignerQ3WidgetStack*>(object))
        return new Q3WidgetStackContainer(w, parent);

    return 0;
}


QT_END_NAMESPACE
