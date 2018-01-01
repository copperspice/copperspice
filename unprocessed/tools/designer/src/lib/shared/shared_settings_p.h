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

#ifndef SHARED_SETTINGS_H
#define SHARED_SETTINGS_H

#include "shared_global_p.h"
#include "deviceprofile_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerSettingsInterface;

class QStringList;
class QSize;

namespace qdesigner_internal {
class Grid;
class PreviewConfiguration;
}

/*!
  Auxiliary methods to store/retrieve settings
  */
namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT QDesignerSharedSettings {
public:
    typedef QList<DeviceProfile> DeviceProfileList;

    explicit QDesignerSharedSettings(QDesignerFormEditorInterface *core);

    Grid defaultGrid() const;
    void setDefaultGrid(const Grid &grid);

    QStringList formTemplatePaths() const;
    void setFormTemplatePaths(const QStringList &paths);

    void setAdditionalFormTemplatePaths(const QStringList &additionalPaths);
    QStringList additionalFormTemplatePaths() const;

    QString formTemplate() const;
    void setFormTemplate(const QString &t);

    QSize newFormSize() const;
    void setNewFormSize(const QSize &s);

    // Check with isCustomPreviewConfigurationEnabled if custom or default
    // configuration should be used.
    PreviewConfiguration customPreviewConfiguration() const;
    void setCustomPreviewConfiguration(const PreviewConfiguration &configuration);

    bool isCustomPreviewConfigurationEnabled() const;
    void setCustomPreviewConfigurationEnabled(bool enabled);

    QStringList userDeviceSkins() const;
    void setUserDeviceSkins(const QStringList &userDeviceSkins);

    bool zoomEnabled() const;
    void setZoomEnabled(bool v);

    // Zoom in percent
    int zoom() const;
    void setZoom(int z);

    // Embedded Design
    DeviceProfile currentDeviceProfile() const;
    void setCurrentDeviceProfileIndex(int i);
    int currentDeviceProfileIndex() const;

    DeviceProfile deviceProfileAt(int idx) const;
    DeviceProfileList deviceProfiles() const;
    void setDeviceProfiles(const DeviceProfileList &dp);

    static const QStringList &defaultFormTemplatePaths();

protected:
    QDesignerSettingsInterface *settings() const { return m_settings; }

private:
    QStringList deviceProfileXml() const;
    QDesignerSettingsInterface *m_settings;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // SHARED_SETTINGS_H
