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

#ifndef ACTIONPROVIDER_H
#define ACTIONPROVIDER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtDesigner/extension.h>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtGui/QApplication>

QT_BEGIN_NAMESPACE

class QAction;

class QDesignerActionProviderExtension
{
public:
    virtual ~QDesignerActionProviderExtension() {}

    virtual QRect actionGeometry(QAction *action) const = 0;
    virtual QAction *actionAt(const QPoint &pos) const = 0;

    virtual void adjustIndicator(const QPoint &pos) = 0;
};

// Find action at the given position for a widget that has actionGeometry() (QToolBar,
// QMenuBar, QMenu). They usually have actionAt(), but that fails since Designer usually sets
// WA_TransparentForMouseEvents on the widgets.
template <class Widget>
    int actionIndexAt(const Widget *w, const QPoint &pos, Qt::Orientation orientation)
{
    const QList<QAction*> actions = w->actions();
    const int actionCount = actions.count();
    if (actionCount == 0)
        return -1;
    // actionGeometry() can be wrong sometimes; it returns a geometry that
    // stretches to the end of the toolbar/menu bar. So, check from the beginning
    // in the case of a horizontal right-to-left orientation.
    const bool checkTopRight = orientation == Qt::Horizontal && w->layoutDirection() == Qt::RightToLeft;
    const QPoint topRight = QPoint(w->rect().width(), 0);
    for (int index = 0; index < actionCount; ++index) {
        QRect g = w->actionGeometry(actions.at(index));
        if (checkTopRight)
            g.setTopRight(topRight);
        else
            g.setTopLeft(QPoint(0, 0));

        if (g.contains(pos))
            return index;
    }
    return -1;
}

Q_DECLARE_EXTENSION_INTERFACE(QDesignerActionProviderExtension, "com.trolltech.Qt.Designer.ActionProvider")

QT_END_NAMESPACE

#endif // ACTIONPROVIDER_H
