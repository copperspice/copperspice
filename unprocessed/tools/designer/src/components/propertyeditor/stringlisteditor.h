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

#ifndef STRINGLISTEDITOR_H
#define STRINGLISTEDITOR_H

#include "ui_stringlisteditor.h"
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE
class QStringListModel;

namespace qdesigner_internal {

class StringListEditor : public QDialog, private Ui::Dialog
{
    Q_OBJECT
public:
    ~StringListEditor();
    void setStringList(const QStringList &stringList);
    QStringList stringList() const;

    static QStringList getStringList(
        QWidget *parent, const QStringList &init = QStringList(), int *result = 0);

private slots:
    void on_upButton_clicked();
    void on_downButton_clicked();
    void on_newButton_clicked();
    void on_deleteButton_clicked();
    void on_valueEdit_textEdited(const QString &text);
    void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
    void currentValueChanged();

private:
    StringListEditor(QWidget *parent = 0);
    void updateUi();
    int currentIndex() const;
    void setCurrentIndex(int index);
    int count() const;
    QString stringAt(int index) const;
    void setStringAt(int index, const QString &value);
    void removeString(int index);
    void insertString(int index, const QString &value);
    void editString(int index);

    QStringListModel *m_model;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // STRINGLISTEDITOR_H
