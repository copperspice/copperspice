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

#ifndef QDESIGNER_SETTINGS_H
#define QDESIGNER_SETTINGS_H

#include "designer_enums.h"
#include <shared_settings_p.h>
#include <QtCore/QMap>
#include <QtCore/QRect>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerSettingsInterface;
struct ToolWindowFontSettings;

class QDesignerSettings : public qdesigner_internal::QDesignerSharedSettings
{
public:
    QDesignerSettings(QDesignerFormEditorInterface *core);

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    void restoreGeometry(QWidget *w, QRect fallBack = QRect()) const;
    void saveGeometryFor(const QWidget *w);

    QStringList recentFilesList() const;
    void setRecentFilesList(const QStringList &list);

    void setShowNewFormOnStartup(bool showIt);
    bool showNewFormOnStartup() const;

    void setUiMode(UIMode mode);
    UIMode uiMode() const;

    void setToolWindowFont(const ToolWindowFontSettings &fontSettings);
    ToolWindowFontSettings toolWindowFont() const;

    QByteArray mainWindowState(UIMode mode) const;
    void setMainWindowState(UIMode mode, const QByteArray &mainWindowState);

    QByteArray toolBarsState(UIMode mode) const;
    void setToolBarsState(UIMode mode, const QByteArray &mainWindowState);

    void clearBackup();
    void setBackup(const QMap<QString, QString> &map);
    QMap<QString, QString> backup() const;
};

QT_END_NAMESPACE

#endif // QDESIGNER_SETTINGS_H
