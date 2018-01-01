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

#ifndef FORMLAYOUTMENU
#define FORMLAYOUTMENU

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

#include "shared_global_p.h"
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

class QAction;
class QWidget;

namespace qdesigner_internal {

// Task menu to be used for form layouts. Offers an options "Add row" which
// pops up a dialog in which the user can specify label name, text and buddy.
class QDESIGNER_SHARED_EXPORT FormLayoutMenu : public QObject
{
    Q_DISABLE_COPY(FormLayoutMenu)
    Q_OBJECT
public:
    typedef QList<QAction *> ActionList;

    explicit FormLayoutMenu(QObject *parent);

    // Populate a list of actions with the form layout actions.
    void populate(QWidget *w, QDesignerFormWindowInterface *fw, ActionList &actions);
    // For implementing QDesignerTaskMenuExtension::preferredEditAction():
    // Return appropriate action for double clicking.
    QAction *preferredEditAction(QWidget *w, QDesignerFormWindowInterface *fw);

private slots:
    void slotAddRow();

private:
    QAction *m_separator1;
    QAction *m_populateFormAction;
    QAction *m_separator2;
    QPointer<QWidget> m_widget;
};
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMLAYOUTMENU
