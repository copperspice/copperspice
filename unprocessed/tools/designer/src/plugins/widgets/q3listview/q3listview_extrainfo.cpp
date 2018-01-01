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

#include "q3listview_extrainfo.h"

#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/private/ui4_p.h>

#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3Header>

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

Q3ListViewExtraInfo::Q3ListViewExtraInfo(Q3ListView *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{}

QWidget *Q3ListViewExtraInfo::widget() const
{ return m_widget; }

QDesignerFormEditorInterface *Q3ListViewExtraInfo::core() const
{ return m_core; }

bool Q3ListViewExtraInfo::saveUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }

bool Q3ListViewExtraInfo::loadUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }


bool Q3ListViewExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    // ### finish me
    Q3ListView *listView = qobject_cast<Q3ListView*>(widget());
    Q_ASSERT(listView != 0);

    QList<DomColumn*> columns;
    Q3Header *header = listView->header();
    for (int i=0; i<header->count(); ++i) {
        DomColumn *c = new DomColumn();

        QList<DomProperty*> properties;

        DomString *str = new DomString();
        str->setText(header->label(i));

        DomProperty *ptext = new DomProperty();
        ptext->setAttributeName(QLatin1String("text"));
        ptext->setElementString(str);

        DomProperty *pclickable = new DomProperty();
        pclickable->setAttributeName(QLatin1String("clickable"));
        pclickable->setElementBool(header->isClickEnabled(i) ? QLatin1String("true") : QLatin1String("false"));

        DomProperty *presizable = new DomProperty();
        presizable->setAttributeName(QLatin1String("resizable"));
        presizable->setElementBool(header->isResizeEnabled(i) ? QLatin1String("true") : QLatin1String("false"));

        properties.append(ptext);
        properties.append(pclickable);
        properties.append(presizable);

        c->setElementProperty(properties);
        columns.append(c);
    }

    ui_widget->setElementColumn(columns);

    QList<DomItem *> items;
    Q3ListViewItem *child = listView->firstChild();
    while (child) {
        items.append(saveQ3ListViewItem(child));
        child = child->nextSibling();
    }
    ui_widget->setElementItem(items);


    return true;
}

bool Q3ListViewExtraInfo::loadWidgetExtraInfo(DomWidget *ui_widget)
{
    Q3ListView *listView = qobject_cast<Q3ListView*>(widget());
    Q_ASSERT(listView != 0);

    Q3Header *header = listView->header();

    QList<DomColumn*> columns = ui_widget->elementColumn();
    for (int i=0; i<columns.size(); ++i) {
        DomColumn *column = columns.at(i);

        QHash<QString, DomProperty*> properties = propertyMap(column->elementProperty());
        DomProperty *text = properties.value(QLatin1String("text"));
        DomProperty *pixmap = properties.value(QLatin1String("pixmap"));
        DomProperty *clickable = properties.value(QLatin1String("clickable"));
        DomProperty *resizable = properties.value(QLatin1String("resizable"));

        QString txt = text->elementString()->text();

        if (pixmap != 0) {
            DomResourcePixmap *pix = pixmap->elementPixmap();
            QIcon icon(core()->iconCache()->resolveQrcPath(pix->text(), pix->attributeResource(), workingDirectory()));
            listView->addColumn(icon, txt);
        } else {
            listView->addColumn(txt);
        }

        if (clickable != 0) {
            header->setClickEnabled(clickable->elementBool() == QLatin1String("true"), header->count() - 1);
        }

        if (resizable != 0) {
            header->setResizeEnabled(resizable->elementBool() == QLatin1String("true"), header->count() - 1);
        }
    }

    if (ui_widget->elementItem().size()) {
        initializeQ3ListViewItems(ui_widget->elementItem());
    }

    return true;
}

DomItem *Q3ListViewExtraInfo::saveQ3ListViewItem(Q3ListViewItem *item) const
{
    DomItem *pitem = new DomItem();
    QList<DomProperty *> properties;
    const int columnCount = static_cast<Q3ListView*>(widget())->columns();
    for (int i = 0; i < columnCount; ++i) {
        DomString *str = new DomString();
        str->setText(item->text(i));

        DomProperty *ptext = new DomProperty();
        ptext->setAttributeName(QLatin1String("text"));
        ptext->setElementString(str);

        properties.append(ptext);
    }
    pitem->setElementProperty(properties);
    QList<DomItem *> items;
    Q3ListViewItem *child = item->firstChild();
    while (child) {
        items.append(saveQ3ListViewItem(child));
        child = child->nextSibling();
    }
    pitem->setElementItem(items);
    return pitem;
}

void Q3ListViewExtraInfo::initializeQ3ListViewItems(const QList<DomItem *> &items, Q3ListViewItem *parentItem)
{
    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        Q3ListViewItem *__item = 0;
        if (parentItem != 0)
            __item = new Q3ListViewItem(parentItem);
        else
            __item = new Q3ListViewItem(static_cast<Q3ListView*>(widget()));

        int textCount = 0, pixCount = 0;
        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                __item->setText(textCount++, p->elementString()->text());

            if (p->attributeName() == QLatin1String("pixmap")) {
                DomResourcePixmap *pix = p->elementPixmap();
                QPixmap pixmap(core()->iconCache()->resolveQrcPath(pix->text(), pix->attributeResource(), workingDirectory()));
                __item->setPixmap(pixCount++, pixmap);
            }
        }

        if (item->elementItem().size()) {
            __item->setOpen(true);
            initializeQ3ListViewItems(item->elementItem(), __item);
        }
    }
}


Q3ListViewExtraInfoFactory::Q3ListViewExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent)
    : QExtensionFactory(parent), m_core(core)
{}

QObject *Q3ListViewExtraInfoFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
        return 0;

    if (Q3ListView *w = qobject_cast<Q3ListView*>(object))
        return new Q3ListViewExtraInfo(w, m_core, parent);

    return 0;
}

QT_END_NAMESPACE
