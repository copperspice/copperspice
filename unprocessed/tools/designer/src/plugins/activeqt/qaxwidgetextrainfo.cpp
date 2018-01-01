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

#include "qaxwidgetextrainfo.h"
#include "qdesigneraxwidget.h"
#include "qaxwidgetpropertysheet.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/private/ui4_p.h>

QT_BEGIN_NAMESPACE

QAxWidgetExtraInfo::QAxWidgetExtraInfo(QDesignerAxWidget *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{
}

QWidget *QAxWidgetExtraInfo::widget() const
{
    return m_widget;
}

QDesignerFormEditorInterface *QAxWidgetExtraInfo::core() const
{
    return m_core;
}

bool QAxWidgetExtraInfo::saveUiExtraInfo(DomUI *)
{
    return false;
}

bool QAxWidgetExtraInfo::loadUiExtraInfo(DomUI *)
{
    return false;
}

bool QAxWidgetExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    /* Turn off standard setters and make sure "control" is in front,
     * otherwise, previews will not work as the properties are not applied via
     * the caching property sheet, them. */
    typedef QList<DomProperty *> DomPropertyList;
    DomPropertyList props = ui_widget->elementProperty();
    const int size = props.size();
    const QString controlProperty = QLatin1String(QAxWidgetPropertySheet::controlPropertyName);
    for (int i = 0; i < size; i++) {
        props.at(i)->setAttributeStdset(false);
        if (i > 0 && props.at(i)->attributeName() == controlProperty) {
            qSwap(props[0], props[i]);
            ui_widget->setElementProperty(props);
        }

    }
    return true;
}

bool QAxWidgetExtraInfo::loadWidgetExtraInfo(DomWidget *)
{
    return false;
}

QAxWidgetExtraInfoFactory::QAxWidgetExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent)
    : QExtensionFactory(parent), m_core(core)
{
}

QObject *QAxWidgetExtraInfoFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
        return 0;

    if (QDesignerAxWidget *w = qobject_cast<QDesignerAxWidget*>(object))
        return new QAxWidgetExtraInfo(w, m_core, parent);

    return 0;
}

QT_END_NAMESPACE
