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

#ifndef SYSTEMSETTINGSDIALOG_H
#define SYSTEMSETTINGSDIALOG_H

#include <QtGui/QDialog>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

namespace Ui {
    class DeviceProfileDialog;
}

class QDesignerDialogGuiInterface;

class QDialogButtonBox;

namespace qdesigner_internal {

class DeviceProfile;

/* DeviceProfileDialog: Widget to edit system settings for embedded design */

class DeviceProfileDialog : public QDialog
{
    Q_DISABLE_COPY(DeviceProfileDialog)
    Q_OBJECT
public:
    explicit DeviceProfileDialog(QDesignerDialogGuiInterface *dlgGui, QWidget *parent = 0);
    ~DeviceProfileDialog();

    DeviceProfile deviceProfile() const;
    void setDeviceProfile(const DeviceProfile &s);

    bool showDialog(const QStringList &existingNames);

private slots:
    void setOkButtonEnabled(bool);
    void nameChanged(const QString &name);
    void save();
    void open();

private:
    void critical(const QString &title, const QString &msg);
    Ui::DeviceProfileDialog *m_ui;
    QDesignerDialogGuiInterface *m_dlgGui;
    QStringList m_existingNames;
};
}

QT_END_NAMESPACE

#endif // SYSTEMSETTINGSDIALOG_H
