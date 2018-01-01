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

#ifndef DPICHOOSER_H
#define DPICHOOSER_H

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QSpinBox;
class QComboBox;

namespace qdesigner_internal {

struct DPI_Entry;

/* Let the user choose a DPI settings */
class DPI_Chooser : public QWidget {
    Q_DISABLE_COPY(DPI_Chooser)
    Q_OBJECT

public:
    explicit DPI_Chooser(QWidget *parent = 0);
    ~DPI_Chooser();

    void getDPI(int *dpiX, int *dpiY) const;
    void setDPI(int dpiX, int dpiY);

private slots:
    void syncSpinBoxes();

private:
    void setUserDefinedValues(int dpiX, int dpiY);

    struct DPI_Entry *m_systemEntry;
    QComboBox *m_predefinedCombo;
    QSpinBox *m_dpiXSpinBox;
    QSpinBox *m_dpiYSpinBox;
};
}

QT_END_NAMESPACE

#endif // DPICHOOSER_H
