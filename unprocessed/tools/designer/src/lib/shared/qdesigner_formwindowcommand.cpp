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

#include "qdesigner_formwindowcommand_p.h"
#include "qdesigner_objectinspector_p.h"
#include "layout_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerObjectInspectorInterface>
#include <QtDesigner/QDesignerActionEditorInterface>
#include <QtDesigner/QDesignerMetaDataBaseInterface>
#include <QtDesigner/QDesignerPropertySheetExtension>
#include <QtDesigner/QDesignerPropertyEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtCore/QVariant>
#include <QtGui/QWidget>
#include <QtGui/QLabel>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

// ---- QDesignerFormWindowCommand ----
QDesignerFormWindowCommand::QDesignerFormWindowCommand(const QString &description,
                                                       QDesignerFormWindowInterface *formWindow,
                                                       QUndoCommand *parent)
    : QUndoCommand(description, parent),
      m_formWindow(formWindow)
{
}

QDesignerFormWindowInterface *QDesignerFormWindowCommand::formWindow() const
{
    return m_formWindow;
}

QDesignerFormEditorInterface *QDesignerFormWindowCommand::core() const
{
    if (QDesignerFormWindowInterface *fw = formWindow())
        return fw->core();

    return 0;
}

void QDesignerFormWindowCommand::undo()
{
    cheapUpdate();
}

void QDesignerFormWindowCommand::redo()
{
    cheapUpdate();
}

void QDesignerFormWindowCommand::cheapUpdate()
{
    if (core()->objectInspector())
        core()->objectInspector()->setFormWindow(formWindow());

    if (core()->actionEditor())
        core()->actionEditor()->setFormWindow(formWindow());
}

QDesignerPropertySheetExtension* QDesignerFormWindowCommand::propertySheet(QObject *object) const
{
    return  qt_extension<QDesignerPropertySheetExtension*>(formWindow()->core()->extensionManager(), object);
}

void QDesignerFormWindowCommand::updateBuddies(QDesignerFormWindowInterface *form,
                                               const QString &old_name,
                                               const QString &new_name)
{
    QExtensionManager* extensionManager = form->core()->extensionManager();

    typedef QList<QLabel*> LabelList;

    const LabelList label_list = form->findChildren<QLabel*>();
    if (label_list.empty())
        return;

    const QString buddyProperty = QLatin1String("buddy");
    const QByteArray oldNameU8 = old_name.toUtf8();
    const QByteArray newNameU8 = new_name.toUtf8();

    const LabelList::const_iterator cend = label_list.constEnd();
    for (LabelList::const_iterator it = label_list.constBegin(); it != cend; ++it ) {
        if (QDesignerPropertySheetExtension* sheet = qt_extension<QDesignerPropertySheetExtension*>(extensionManager, *it)) {
            const int idx = sheet->indexOf(buddyProperty);
            if (idx != -1) {
                const QByteArray oldBuddy = sheet->property(idx).toByteArray();
                if (oldBuddy == oldNameU8)
                    sheet->setProperty(idx, newNameU8);
            }
        }
    }
}

void QDesignerFormWindowCommand::selectUnmanagedObject(QObject *unmanagedObject)
{
    // Keep selection in sync
    if (QDesignerObjectInspector *oi = qobject_cast<QDesignerObjectInspector *>(core()->objectInspector())) {
        oi->clearSelection();
        oi->selectObject(unmanagedObject);
    }
    core()->propertyEditor()->setObject(unmanagedObject);
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
