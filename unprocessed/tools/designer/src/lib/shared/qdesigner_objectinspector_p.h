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

#ifndef DESIGNEROBJECTINSPECTOR_H
#define DESIGNEROBJECTINSPECTOR_H

#include "shared_global_p.h"
#include <QtDesigner/QDesignerObjectInspectorInterface>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class QDesignerDnDItemInterface;

namespace qdesigner_internal {

struct QDESIGNER_SHARED_EXPORT Selection {
    bool empty() const;
    void clear();

    // Merge all lists
    QObjectList selection() const;

    // Selection in cursor (managed widgets)
    QWidgetList managed;
    // Unmanaged widgets
    QWidgetList unmanaged;
    // Remaining selected objects (non-widgets)
    QObjectList objects;
};

// Extends the QDesignerObjectInspectorInterface by functionality
// to access the selection

class QDESIGNER_SHARED_EXPORT QDesignerObjectInspector: public QDesignerObjectInspectorInterface
{
    Q_OBJECT
public:
    explicit QDesignerObjectInspector(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    // Select a qobject unmanaged by form window
    virtual bool selectObject(QObject *o) = 0;
    virtual void getSelection(Selection &s) const = 0;
    virtual void clearSelection() = 0;

public slots:
    virtual void mainContainerChanged();
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // DESIGNEROBJECTINSPECTOR_H
