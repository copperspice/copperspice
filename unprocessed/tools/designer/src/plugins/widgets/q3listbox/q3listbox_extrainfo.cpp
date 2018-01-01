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

#include "q3listbox_extrainfo.h"

#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/private/ui4_p.h>

#include <Qt3Support/Q3ListBox>

QT_BEGIN_NAMESPACE

inline QHash<QString, DomProperty *> propertyMap(const QList<DomProperty *> &properties) // ### remove me
{
    QHash<QString, DomProperty *> map;

    for (int i=0; i<properties.size(); ++i) {
        DomProperty *p = properties.at(i);
        map.insert(p->attributeName(), p);
    }

    return map;
}

Q3ListBoxExtraInfo::Q3ListBoxExtraInfo(Q3ListBox *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{}

QWidget *Q3ListBoxExtraInfo::widget() const
{ return m_widget; }

QDesignerFormEditorInterface *Q3ListBoxExtraInfo::core() const
{ return m_core; }

bool Q3ListBoxExtraInfo::saveUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }

bool Q3ListBoxExtraInfo::loadUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }


bool Q3ListBoxExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    Q3ListBox *listBox = qobject_cast<Q3ListBox*>(widget());
    Q_ASSERT(listBox != 0);

    QList<DomItem *> items;
    const int childCount = listBox->count();
    for (int i = 0; i < childCount; ++i) {
        DomItem *item = new DomItem();

        QList<DomProperty*> properties;

        DomString *str = new DomString();
        str->setText(listBox->text(i));

        DomProperty *ptext = new DomProperty();
        ptext->setAttributeName(QLatin1String("text"));
        ptext->setElementString(str);

        properties.append(ptext);
        item->setElementProperty(properties);
        items.append(item);
    }
    ui_widget->setElementItem(items);

    return true;
}

bool Q3ListBoxExtraInfo::loadWidgetExtraInfo(DomWidget *ui_widget)
{
    Q3ListBox *listBox = qobject_cast<Q3ListBox*>(widget());
    Q_ASSERT(listBox != 0);

    QList<DomItem *> items = ui_widget->elementItem();
    for (int i = 0; i < items.size(); ++i) {
        DomItem *item = items.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(item->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));

        QString txt = text->elementString()->text();

        if (pixmap != 0) {
            DomResourcePixmap *pix = pixmap->elementPixmap();
            QPixmap pixmap(core()->iconCache()->resolveQrcPath(pix->text(), pix->attributeResource(), workingDirectory()));
            listBox->insertItem(pixmap, txt);
        } else {
            listBox->insertItem(txt);
        }
    }

    return true;
}

Q3ListBoxExtraInfoFactory::Q3ListBoxExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent)
    : QExtensionFactory(parent), m_core(core)
{}

QObject *Q3ListBoxExtraInfoFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
        return 0;

    if (Q3ListBox *w = qobject_cast<Q3ListBox*>(object))
        return new Q3ListBoxExtraInfo(w, m_core, parent);

    return 0;
}

QT_END_NAMESPACE
