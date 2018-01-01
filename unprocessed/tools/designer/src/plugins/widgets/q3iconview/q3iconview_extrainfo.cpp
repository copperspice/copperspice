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

#include "q3iconview_extrainfo.h"

#include <QtDesigner/QDesignerIconCacheInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/private/ui4_p.h>

#include <Qt3Support/Q3IconView>

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

Q3IconViewExtraInfo::Q3IconViewExtraInfo(Q3IconView *widget, QDesignerFormEditorInterface *core, QObject *parent)
    : QObject(parent), m_widget(widget), m_core(core)
{}

QWidget *Q3IconViewExtraInfo::widget() const
{ return m_widget; }

QDesignerFormEditorInterface *Q3IconViewExtraInfo::core() const
{ return m_core; }

bool Q3IconViewExtraInfo::saveUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }

bool Q3IconViewExtraInfo::loadUiExtraInfo(DomUI *ui)
{ Q_UNUSED(ui); return false; }


bool Q3IconViewExtraInfo::saveWidgetExtraInfo(DomWidget *ui_widget)
{
    // ### finish me
    Q3IconView *iconView = qobject_cast<Q3IconView*>(widget());
    Q_ASSERT(iconView != 0);

    QList<DomItem*> ui_items;

    Q3IconViewItem *__item = iconView->firstItem();
    while (__item != 0) {
        DomItem *ui_item = new DomItem();

        QList<DomProperty*> properties;

        // text property
        DomProperty *ptext = new DomProperty();
        DomString *str = new DomString();
        str->setText(__item->text());
        ptext->setAttributeName(QLatin1String("text"));
        ptext->setElementString(str);
        properties.append(ptext);

        ui_item->setElementProperty(properties);
        ui_items.append(ui_item);

        if (__item->pixmap() != 0 && core()->iconCache()) {
            QPixmap pix = *__item->pixmap();
            QString filePath = core()->iconCache()->pixmapToFilePath(pix);
            QString qrcPath = core()->iconCache()->pixmapToQrcPath(pix);
            DomResourcePixmap *ui_pix = new DomResourcePixmap();
            if (!qrcPath.isEmpty())
                ui_pix->setAttributeResource(qrcPath);
            ui_pix->setText(filePath);

            DomProperty *ppix = new DomProperty();
            ppix->setAttributeName(QLatin1String("pixmap"));
            ppix->setElementPixmap(ui_pix);
            properties.append(ppix);
        }

        __item = __item->nextItem();
    }

    ui_widget->setElementItem(ui_items);

    return true;
}

bool Q3IconViewExtraInfo::loadWidgetExtraInfo(DomWidget *ui_widget)
{
    Q3IconView *iconView = qobject_cast<Q3IconView*>(widget());
    Q_ASSERT(iconView != 0);
    Q_UNUSED(iconView);

    if (ui_widget->elementItem().size()) {
        initializeQ3IconViewItems(ui_widget->elementItem());
    }

    return true;
}

void Q3IconViewExtraInfo::initializeQ3IconViewItems(const QList<DomItem *> &items)
{
    Q3IconView *iconView = qobject_cast<Q3IconView*>(widget());
    Q_ASSERT(iconView != 0);

    for (int i=0; i<items.size(); ++i) {
        DomItem *item = items.at(i);

        Q3IconViewItem *__item = new Q3IconViewItem(iconView);

        QList<DomProperty*> properties = item->elementProperty();
        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            if (p->attributeName() == QLatin1String("text"))
                __item->setText(p->elementString()->text());

            if (p->attributeName() == QLatin1String("pixmap")) {
                DomResourcePixmap *pix = p->elementPixmap();
                QPixmap pixmap(core()->iconCache()->resolveQrcPath(pix->text(), pix->attributeResource(), workingDirectory()));
                __item->setPixmap(pixmap);
            }
        }
    }
}


Q3IconViewExtraInfoFactory::Q3IconViewExtraInfoFactory(QDesignerFormEditorInterface *core, QExtensionManager *parent)
    : QExtensionFactory(parent), m_core(core)
{}

QObject *Q3IconViewExtraInfoFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerExtraInfoExtension))
        return 0;

    if (Q3IconView *w = qobject_cast<Q3IconView*>(object))
        return new Q3IconViewExtraInfo(w, m_core, parent);

    return 0;
}

QT_END_NAMESPACE
