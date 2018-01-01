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

#include "default_layoutdecoration.h"
#include "qlayout_widget_p.h"

#include <layoutinfo_p.h>

#include <QtDesigner/QDesignerMetaDataBaseItemInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// ---- QDesignerLayoutDecorationFactory ----
QDesignerLayoutDecorationFactory::QDesignerLayoutDecorationFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QDesignerLayoutDecorationFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (!object->isWidgetType() || iid != Q_TYPEID(QDesignerLayoutDecorationExtension))
        return 0;

    QWidget *widget = qobject_cast<QWidget*>(object);

    if (const QLayoutWidget *layoutWidget = qobject_cast<const QLayoutWidget*>(widget))
        return QLayoutSupport::createLayoutSupport(layoutWidget->formWindow(), widget, parent);

    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(widget))
        if (LayoutInfo::managedLayout(fw->core(), widget))
            return QLayoutSupport::createLayoutSupport(fw, widget, parent);

    return 0;
}
}

QT_END_NAMESPACE
