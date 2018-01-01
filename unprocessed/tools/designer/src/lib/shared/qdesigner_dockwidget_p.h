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

#ifndef QDESIGNER_DOCKWIDGET_H
#define QDESIGNER_DOCKWIDGET_H

#include "shared_global_p.h"
#include <QtGui/QDockWidget>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

class QDESIGNER_SHARED_EXPORT QDesignerDockWidget: public QDockWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::DockWidgetArea dockWidgetArea READ dockWidgetArea WRITE setDockWidgetArea DESIGNABLE docked STORED false)
    Q_PROPERTY(bool docked READ docked WRITE setDocked DESIGNABLE inMainWindow STORED false)
public:
    QDesignerDockWidget(QWidget *parent = 0);
    virtual ~QDesignerDockWidget();

    bool docked() const;
    void setDocked(bool b);

    Qt::DockWidgetArea dockWidgetArea() const;
    void setDockWidgetArea(Qt::DockWidgetArea dockWidgetArea);

    bool inMainWindow() const;

private:
    QDesignerFormWindowInterface *formWindow() const;
    QMainWindow *findMainWindow() const;
};

QT_END_NAMESPACE

#endif // QDESIGNER_DOCKWIDGET_H
