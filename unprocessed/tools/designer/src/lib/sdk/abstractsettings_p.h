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

#ifndef ABSTRACTSETTINGS_P_H
#define ABSTRACTSETTINGS_P_H

#include <QtDesigner/sdk_global.h>

#include <QVariant>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QString;

/*!
 To be implemented by IDEs that want to control the way designer retrieves/stores its settings.
 */
class QDESIGNER_SDK_EXPORT QDesignerSettingsInterface
{
public:
    virtual ~QDesignerSettingsInterface() {}

    virtual void beginGroup(const QString &prefix) = 0;
    virtual void endGroup() = 0;

    virtual bool contains(const QString &key) const = 0;
    virtual void setValue(const QString &key, const QVariant &value) = 0;
    virtual QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const = 0;
    virtual void remove(const QString &key) = 0;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // ABSTRACTSETTINGS_P_H
