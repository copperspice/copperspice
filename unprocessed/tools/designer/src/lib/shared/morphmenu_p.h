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

#ifndef MORPH_COMMAND_H
#define MORPH_COMMAND_H

#include "shared_global_p.h"
#include "qdesigner_formwindowcommand_p.h"

QT_BEGIN_NAMESPACE

class QAction;
class QSignalMapper;
class QMenu;

namespace qdesigner_internal {

/* Conveniene morph menu that acts on a single widget. */
class QDESIGNER_SHARED_EXPORT MorphMenu : public QObject {
    Q_DISABLE_COPY(MorphMenu)
    Q_OBJECT
public:
    typedef QList<QAction *> ActionList;

    explicit MorphMenu(QObject *parent = 0);

    void populate(QWidget *w, QDesignerFormWindowInterface *fw, ActionList& al);
    void populate(QWidget *w, QDesignerFormWindowInterface *fw, QMenu& m);

private slots:
    void slotMorph(const QString &newClassName);

private:
    bool populateMenu(QWidget *w, QDesignerFormWindowInterface *fw);

    QAction *m_subMenuAction;
    QMenu *m_menu;
    QSignalMapper *m_mapper;

    QWidget *m_widget;
    QDesignerFormWindowInterface *m_formWindow;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // MORPH_COMMAND_H
