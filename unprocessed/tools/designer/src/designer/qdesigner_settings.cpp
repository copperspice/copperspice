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

#include "qdesigner.h"
#include "qdesigner_settings.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_workbench.h"

#include <abstractformeditor.h>
#include <abstractsettings_p.h>
#include <qdesigner_utils_p.h>
#include <previewmanager_p.h>

#include <QtCore/QVariant>
#include <QtCore/QDir>

#include <QtGui/QDesktopWidget>
#include <QtGui/QStyle>
#include <QtGui/QListView>

#include <QtCore/qdebug.h>

enum { debugSettings = 0 };

QT_BEGIN_NAMESPACE

static const char *newFormShowKey = "newFormDialog/ShowOnStartup";

// Change the version whenever the arrangement changes significantly.
static const char *mainWindowStateKey = "MainWindowState45";
static const char *toolBarsStateKey = "ToolBarsState45";

static const char *backupOrgListKey = "backup/fileListOrg";
static const char *backupBakListKey = "backup/fileListBak";
static const char *recentFilesListKey = "recentFilesList";

QDesignerSettings::QDesignerSettings(QDesignerFormEditorInterface *core) :
    qdesigner_internal::QDesignerSharedSettings(core)
{
}

void QDesignerSettings::setValue(const QString &key, const QVariant &value)
{
    settings()->setValue(key, value);
}

QVariant QDesignerSettings::value(const QString &key, const QVariant &defaultValue) const
{
    return settings()->value(key, defaultValue);
}

static inline QChar modeChar(UIMode mode)
{
    return QLatin1Char(static_cast<char>(mode) + '0');
}

void QDesignerSettings::saveGeometryFor(const QWidget *w)
{
    Q_ASSERT(w && !w->objectName().isEmpty());
    QDesignerSettingsInterface *s = settings();
    const bool visible = w->isVisible();
    if (debugSettings)
        qDebug() << Q_FUNC_INFO << w << "visible=" << visible;
    s->beginGroup(w->objectName());
    s->setValue(QLatin1String("visible"), visible);
    s->setValue(QLatin1String("geometry"), w->saveGeometry());
    s->endGroup();
}

void QDesignerSettings::restoreGeometry(QWidget *w, QRect fallBack) const
{
    Q_ASSERT(w && !w->objectName().isEmpty());
    const QString key = w->objectName();
    const QByteArray ba(settings()->value(key + QLatin1String("/geometry")).toByteArray());
    const bool visible = settings()->value(key + QLatin1String("/visible"), true).toBool();

    if (debugSettings)
        qDebug() << Q_FUNC_INFO << w << fallBack << "visible=" << visible;
    if (ba.isEmpty()) {
        /// Apply default geometry, check for null and maximal size
        if (fallBack.isNull())
            fallBack = QRect(QPoint(0, 0), w->sizeHint());
        if (fallBack.size() == QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)) {
            w->setWindowState(w->windowState() | Qt::WindowMaximized);
        } else {
            w->move(fallBack.topLeft());
            w->resize(fallBack.size());
        }
    } else {
        w->restoreGeometry(ba);
    }

    if (visible)
        w->show();
}

QStringList QDesignerSettings::recentFilesList() const
{
    return settings()->value(QLatin1String(recentFilesListKey)).toStringList();
}

void QDesignerSettings::setRecentFilesList(const QStringList &sl)
{
    settings()->setValue(QLatin1String(recentFilesListKey), sl);
}

void QDesignerSettings::setShowNewFormOnStartup(bool showIt)
{
    settings()->setValue(QLatin1String(newFormShowKey), showIt);
}

bool QDesignerSettings::showNewFormOnStartup() const
{
    return settings()->value(QLatin1String(newFormShowKey), true).toBool();
}

QByteArray QDesignerSettings::mainWindowState(UIMode mode) const
{
    return settings()->value(QLatin1String(mainWindowStateKey) + modeChar(mode)).toByteArray();
}

void QDesignerSettings::setMainWindowState(UIMode mode, const QByteArray &mainWindowState)
{
    settings()->setValue(QLatin1String(mainWindowStateKey) + modeChar(mode), mainWindowState);
}

QByteArray QDesignerSettings::toolBarsState(UIMode mode) const
{
    QString key = QLatin1String(toolBarsStateKey);
    key += modeChar(mode);
    return settings()->value(key).toByteArray();
}

void QDesignerSettings::setToolBarsState(UIMode mode, const QByteArray &toolBarsState)
{
    QString key = QLatin1String(toolBarsStateKey);
    key += modeChar(mode);
    settings()->setValue(key, toolBarsState);
}

void QDesignerSettings::clearBackup()
{
    QDesignerSettingsInterface *s = settings();
    s->remove(QLatin1String(backupOrgListKey));
    s->remove(QLatin1String(backupBakListKey));
}

void QDesignerSettings::setBackup(const QMap<QString, QString> &map)
{
    const QStringList org = map.keys();
    const QStringList bak = map.values();

    QDesignerSettingsInterface *s = settings();
    s->setValue(QLatin1String(backupOrgListKey), org);
    s->setValue(QLatin1String(backupBakListKey), bak);
}

QMap<QString, QString> QDesignerSettings::backup() const
{
    const QStringList org = settings()->value(QLatin1String(backupOrgListKey), QStringList()).toStringList();
    const QStringList bak = settings()->value(QLatin1String(backupBakListKey), QStringList()).toStringList();

    QMap<QString, QString> map;
    const int orgCount = org.count();
    for (int i = 0; i < orgCount; ++i)
        map.insert(org.at(i), bak.at(i));

    return map;
}

void QDesignerSettings::setUiMode(UIMode mode)
{
    QDesignerSettingsInterface *s = settings();
    s->beginGroup(QLatin1String("UI"));
    s->setValue(QLatin1String("currentMode"), mode);
    s->endGroup();
}

UIMode QDesignerSettings::uiMode() const
{
#ifdef Q_WS_MAC
    const UIMode defaultMode = TopLevelMode;
#else
    const UIMode defaultMode = DockedMode;
#endif
    UIMode uiMode = static_cast<UIMode>(value(QLatin1String("UI/currentMode"), defaultMode).toInt());
    return uiMode;
}

void QDesignerSettings::setToolWindowFont(const ToolWindowFontSettings &fontSettings)
{
    QDesignerSettingsInterface *s = settings();
    s->beginGroup(QLatin1String("UI"));
    s->setValue(QLatin1String("font"), fontSettings.m_font);
    s->setValue(QLatin1String("useFont"), fontSettings.m_useFont);
    s->setValue(QLatin1String("writingSystem"), fontSettings.m_writingSystem);
    s->endGroup();
}

ToolWindowFontSettings QDesignerSettings::toolWindowFont() const
{
    ToolWindowFontSettings fontSettings;
    fontSettings.m_writingSystem =
            static_cast<QFontDatabase::WritingSystem>(value(QLatin1String("UI/writingSystem"),
                                                            QFontDatabase::Any).toInt());
    fontSettings.m_font = qvariant_cast<QFont>(value(QLatin1String("UI/font")));
    fontSettings.m_useFont =
            settings()->value(QLatin1String("UI/useFont"), QVariant(false)).toBool();
    return fontSettings;
}

QT_END_NAMESPACE
