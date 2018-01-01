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

#ifndef QHELPPROJECTDATA_H
#define QHELPPROJECTDATA_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include "qhelp_global.h"
#include "qhelpdatainterface_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QHelpProjectDataPrivate;

class QHELP_EXPORT QHelpProjectData : public QHelpDataInterface
{
public:
    QHelpProjectData();
    ~QHelpProjectData();

    bool readData(const QString &fileName);
    QString errorMessage() const;

    QString namespaceName() const;
    QString virtualFolder() const;
    QList<QHelpDataCustomFilter> customFilters() const;
    QList<QHelpDataFilterSection> filterSections() const;
    QMap<QString, QVariant> metaData() const;
    QString rootPath() const;

private:
    QHelpProjectDataPrivate *d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
