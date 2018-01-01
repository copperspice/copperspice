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
// of the Qt tools.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef GRIDPANEL_H
#define GRIDPANEL_H

#include "shared_global_p.h"

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class Grid;

namespace Ui {
    class GridPanel;
}

class  QDESIGNER_SHARED_EXPORT GridPanel : public QWidget
{
    Q_OBJECT
public:
    GridPanel(QWidget *parent = 0);
    ~GridPanel();

    void setTitle(const QString &title);

    void setGrid(const Grid &g);
    Grid grid() const;

    void setCheckable (bool c);
    bool isCheckable () const;

    bool isChecked () const;
    void setChecked(bool c);

    void setResetButtonVisible(bool v);

private slots:
    void reset();

private:
    Ui::GridPanel *m_ui;
};

} // qdesigner_internal

QT_END_NAMESPACE

#endif // GRIDPANEL_H
