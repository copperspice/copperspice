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

#ifndef SHAREDENUMS_H
#define SHAREDENUMS_H

#include "shared_global_p.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

    // Validation mode of text property line edits
    enum TextPropertyValidationMode {
        // Allow for multiline editing using literal "\n".
        ValidationMultiLine,
        // Allow for HTML rich text including multiline editing using literal "\n".
        ValidationRichText,
        // Validate a stylesheet
        ValidationStyleSheet,
        // Single line mode, suppresses newlines
        ValidationSingleLine,
        // Allow only for identifier characters
        ValidationObjectName,
        // Allow only for identifier characters and colons
        ValidationObjectNameScope,
        // URL
        ValidationURL
        };

    // Container types
    enum ContainerType {
        // A container with pages, at least one of which one must always be present (for example, QTabWidget)
        PageContainer,
        // Mdi type container. All pages may be deleted, no concept of page order
        MdiContainer,
        // Wizard container
        WizardContainer
        };

    enum AuxiliaryItemDataRoles {
        // item->flags while being edited
        ItemFlagsShadowRole = 0x13370551
    };

}

QT_END_NAMESPACE

#endif // SHAREDENUMS_H
