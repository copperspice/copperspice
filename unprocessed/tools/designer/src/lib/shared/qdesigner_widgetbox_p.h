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

#ifndef QDESIGNER_WIDGETBOX_H
#define QDESIGNER_WIDGETBOX_H

#include "shared_global_p.h"
#include <QtDesigner/QDesignerWidgetBoxInterface>

QT_BEGIN_NAMESPACE

class DomUI;

namespace qdesigner_internal {

// A widget box with a load mode that allows for updating custom widgets.

class QDESIGNER_SHARED_EXPORT QDesignerWidgetBox : public QDesignerWidgetBoxInterface
{
    Q_OBJECT
public:
    enum LoadMode { LoadMerge, LoadReplace, LoadCustomWidgetsOnly };

    explicit QDesignerWidgetBox(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    LoadMode loadMode() const;
    void setLoadMode(LoadMode lm);

    virtual bool loadContents(const QString &contents) = 0;

    // Convenience to access the widget box icon of a widget. Empty category
    // matches all
    virtual QIcon iconForWidget(const QString &className,
                                const QString &category = QString()) const = 0;

    // Convenience to find a widget by class name. Empty category matches all
    static bool findWidget(const QDesignerWidgetBoxInterface *wbox,
                           const QString &className,
                           const QString &category /* = QString()  */,
                           Widget *widgetData);
    // Convenience functions to create a DomWidget from widget box xml.
    static DomUI *xmlToUi(const QString &name, const QString &xml, bool insertFakeTopLevel, QString *errorMessage);
    static DomUI *xmlToUi(const QString &name, const QString &xml, bool insertFakeTopLevel);

private:
    LoadMode m_loadMode;
};
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_WIDGETBOX_H
