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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef LAYOUTINFO_H
#define LAYOUTINFO_H

#include "shared_global_p.h"

QT_BEGIN_NAMESPACE

class QWidget;
class QLayout;
class QLayoutItem;
class QDesignerFormEditorInterface;
class QFormLayout;
class QRect;
class QString;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT LayoutInfo
{
public:
    enum Type
    {
        NoLayout,
        HSplitter,
        VSplitter,
        HBox,
        VBox,
        Grid,
        Form,
        UnknownLayout // QDockWindow inside QMainWindow is inside QMainWindowLayout - it doesn't mean there is no layout
    };

    static void deleteLayout(const QDesignerFormEditorInterface *core, QWidget *widget);

    // Examines the immediate layout of the widget (will fail for Q3Group Box).
    static Type layoutType(const QDesignerFormEditorInterface *core, const QWidget *w);
    // Examines the managed layout of the widget
    static Type managedLayoutType(const QDesignerFormEditorInterface *core, const QWidget *w, QLayout **layout = 0);
    static Type layoutType(const QDesignerFormEditorInterface *core, const QLayout *layout);
    static Type layoutType(const QString &typeName);
    static QString layoutName(Type t);

    static QWidget *layoutParent(const QDesignerFormEditorInterface *core, QLayout *layout);

    static Type laidoutWidgetType(const QDesignerFormEditorInterface *core, QWidget *widget, bool *isManaged = 0, QLayout **layout = 0);
    static bool inline isWidgetLaidout(const QDesignerFormEditorInterface *core, QWidget *widget) { return laidoutWidgetType(core, widget) != NoLayout; }

    static QLayout *managedLayout(const QDesignerFormEditorInterface *core, const QWidget *widget);
    static QLayout *managedLayout(const QDesignerFormEditorInterface *core, QLayout *layout);
    static QLayout *internalLayout(const QWidget *widget);

    // Is it a a dummy grid placeholder created by Designer?
    static bool isEmptyItem(QLayoutItem *item);
};

QDESIGNER_SHARED_EXPORT void getFormLayoutItemPosition(const QFormLayout *formLayout, int index, int *rowPtr, int *columnPtr = 0, int *rowspanPtr = 0, int *colspanPtr = 0);
QDESIGNER_SHARED_EXPORT void formLayoutAddWidget(QFormLayout *formLayout, QWidget *w, const QRect &r, bool insert);
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // LAYOUTINFO_H
