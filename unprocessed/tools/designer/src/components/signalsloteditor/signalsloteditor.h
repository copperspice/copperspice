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

#ifndef SIGNALSLOTEDITOR_H
#define SIGNALSLOTEDITOR_H

#include "signalsloteditor_global.h"

#include <QtDesigner/private/connectionedit_p.h>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

QT_BEGIN_NAMESPACE

class DomConnections;

namespace qdesigner_internal {

class SignalSlotConnection;

class QT_SIGNALSLOTEDITOR_EXPORT SignalSlotEditor : public ConnectionEdit
{
    Q_OBJECT

public:
    SignalSlotEditor(QDesignerFormWindowInterface *form_window, QWidget *parent);

    virtual void setSignal(SignalSlotConnection *con, const QString &member);
    virtual void setSlot(SignalSlotConnection *con, const QString &member);
    virtual void setSource(Connection *con, const QString &obj_name);
    virtual void setTarget(Connection *con, const QString &obj_name);

    DomConnections *toUi() const;
    void fromUi(const DomConnections *connections, QWidget *parent);

    QDesignerFormWindowInterface *formWindow() const { return m_form_window; }

    QObject *objectByName(QWidget *topLevel, const QString &name) const;

    void addEmptyConnection();

protected:
    virtual QWidget *widgetAt(const QPoint &pos) const;

private:
    virtual Connection *createConnection(QWidget *source, QWidget *destination);
    virtual void modifyConnection(Connection *con);

    QDesignerFormWindowInterface *m_form_window;
    bool m_showAllSignalsSlots;

    friend class SetMemberCommand;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SIGNALSLOTEDITOR_H
