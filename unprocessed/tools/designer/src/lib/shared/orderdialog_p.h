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
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef ORDERDIALOG_P_H
#define ORDERDIALOG_P_H

#include "shared_global_p.h"

#include <QtGui/QDialog>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

namespace Ui {
    class OrderDialog;
}

class QDESIGNER_SHARED_EXPORT OrderDialog: public QDialog
{
    Q_OBJECT
public:
    OrderDialog(QWidget *parent);
    virtual ~OrderDialog();

    static QWidgetList pagesOfContainer(const QDesignerFormEditorInterface *core, QWidget *container);

    void setPageList(const QWidgetList &pages);
    QWidgetList pageList() const;

    void setDescription(const QString &d);

    enum Format {        // Display format
        PageOrderFormat, // Container pages, ranging 0..[n-1]
        TabOrderFormat   // List of widgets,  ranging 1..1
    };

    void setFormat(Format f)  { m_format = f; }
    Format format() const     { return m_format; }

private slots:
    void on_upButton_clicked();
    void on_downButton_clicked();
    void on_pageList_currentRowChanged(int row);
    void slotEnableButtonsAfterDnD();
    void slotReset();

private:
    void buildList();
    void enableButtons(int r);

    typedef QMap<int, QWidget*> OrderMap;
    OrderMap m_orderMap;
    Ui::OrderDialog* m_ui;
    Format m_format;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ORDERDIALOG_P_H
