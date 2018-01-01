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

#ifndef QDESIGNER_QSETTINGS_H
#define QDESIGNER_QSETTINGS_H

#include "abstractsettings_p.h"
#include "shared_global_p.h"

#include <QtCore/QSettings>

QT_BEGIN_NAMESPACE

//  Implements QDesignerSettingsInterface by calls to QSettings
class QDESIGNER_SHARED_EXPORT QDesignerQSettings : public QDesignerSettingsInterface
{
public:
    QDesignerQSettings();

    virtual void beginGroup(const QString &prefix);
    virtual void endGroup();

    virtual bool contains(const QString &key) const;
    virtual void setValue(const QString &key, const QVariant &value);
    virtual QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    virtual void remove(const QString &key);

    // The application name to be used for settings. Allows for including
    // the Qt version to prevent settings of different Qt versions from
    // interfering.
    static QString settingsApplicationName();

private:
    QSettings m_settings;
};

QT_END_NAMESPACE

#endif // QDESIGNER_QSETTINGS_H
