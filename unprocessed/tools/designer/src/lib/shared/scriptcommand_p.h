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

#ifndef QDESIGNER_SCRIPTCOMMAND_H
#define QDESIGNER_SCRIPTCOMMAND_H

#include "qdesigner_formwindowcommand_p.h"

#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT ScriptCommand: public QDesignerFormWindowCommand
{
    ScriptCommand(const ScriptCommand &);
    ScriptCommand& operator=(const ScriptCommand &);

public:
    explicit ScriptCommand(QDesignerFormWindowInterface *formWindow);

    typedef QList<QObject *> ObjectList;
    bool init(const ObjectList &list, const QString &script);

    virtual void redo();
    virtual void undo();

private:
    typedef QPair<QPointer<QObject>, QString> ObjectScriptPair;
    typedef QList<ObjectScriptPair> ObjectScriptList;
    ObjectScriptList m_oldValues;
    QString m_script;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_SCRIPTCOMMAND_H
