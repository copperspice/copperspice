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

#ifndef FORMWINDOWSETTINGS_H
#define FORMWINDOWSETTINGS_H

#include <QDialog>

QT_BEGIN_NAMESPACE
    
namespace Ui {
    class FormWindowSettings;
}

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

struct FormWindowData;
class FormWindowBase;

/* Dialog to edit the settings of a QDesignerFormWindowInterface.
 * It sets the dirty flag on the form window if something was changed. */

class FormWindowSettings: public QDialog
{
    Q_DISABLE_COPY(FormWindowSettings)
    Q_OBJECT
public:
    explicit FormWindowSettings(QDesignerFormWindowInterface *formWindow);
    virtual ~FormWindowSettings();

    virtual void accept();

private:
    FormWindowData data() const;
    void setData(const FormWindowData&);

    Ui::FormWindowSettings *m_ui;
    FormWindowBase *m_formWindow;
    FormWindowData *m_oldData;
};
}

QT_END_NAMESPACE

#endif // FORMWINDOWSETTINGS_H
