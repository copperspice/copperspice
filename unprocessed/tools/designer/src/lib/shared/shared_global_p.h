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

#ifndef SHARED_GLOBAL_H
#define SHARED_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef QT_DESIGNER_STATIC
#define QDESIGNER_SHARED_EXTERN
#define QDESIGNER_SHARED_IMPORT
#else
#define QDESIGNER_SHARED_EXTERN Q_DECL_EXPORT
#define QDESIGNER_SHARED_IMPORT Q_DECL_IMPORT
#endif

#ifndef QT_NO_SHARED_EXPORT
#  ifdef QDESIGNER_SHARED_LIBRARY
#    define QDESIGNER_SHARED_EXPORT QDESIGNER_SHARED_EXTERN
#  else
#    define QDESIGNER_SHARED_EXPORT QDESIGNER_SHARED_IMPORT
#  endif
#else
#  define QDESIGNER_SHARED_EXPORT
#endif

#endif // SHARED_GLOBAL_H
