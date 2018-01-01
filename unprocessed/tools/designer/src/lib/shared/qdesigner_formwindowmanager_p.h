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

#ifndef QDESIGNER_FORMWINDOMANAGER_H
#define QDESIGNER_FORMWINDOMANAGER_H

#include "shared_global_p.h"
#include <QtDesigner/QDesignerFormWindowManagerInterface>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class PreviewManager;

//
// Convenience methods to manage form previews (ultimately forwarded to PreviewManager).
//
class QDESIGNER_SHARED_EXPORT QDesignerFormWindowManager
    : public QDesignerFormWindowManagerInterface
{
    Q_OBJECT
public:
    explicit QDesignerFormWindowManager(QObject *parent = 0);
    virtual ~QDesignerFormWindowManager();

    virtual QAction *actionDefaultPreview() const;
    virtual QActionGroup *actionGroupPreviewInStyle() const;
    virtual QAction *actionShowFormWindowSettingsDialog() const;

    virtual QPixmap createPreviewPixmap(QString *errorMessage) = 0;

    virtual PreviewManager *previewManager() const = 0;

Q_SIGNALS:
    void formWindowSettingsChanged(QDesignerFormWindowInterface *fw);

public Q_SLOTS:
    virtual void closeAllPreviews() = 0;
    void aboutPlugins();

private:
    void *m_unused;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_FORMWINDOMANAGER_H
