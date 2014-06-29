/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//! [3]
#include <QClassName>
//! [3]


//! [4]
#include <QString>
#include <QApplication>
#include <QSqlTableModel>
//! [4]


//! [5]
#include <qclassname.h>
//! [5]


//! [6]
#include <QtCore>
//! [6]


//! [7]
using namespace Qt;
//! [7]


//! [8]
QLabel *label1 = new QLabel("Hello", this);
QLabel *label2 = new QLabel(this, "Hello");
//! [8]


//! [9]
MyWidget::MyWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    ...
}
//! [9]


//! [10]
// DEPRECATED
if (obj->inherits("QPushButton")) {
    QPushButton *pushButton = (QPushButton *)obj;
    ...
}
//! [10]


//! [11]
QPushButton *pushButton = qobject_cast<QPushButton *>(obj);
if (pushButton) {
    ...
}
//! [11]


//! [12]
QLabel *label = new QLabel;
QPointer<QLabel> safeLabel = label;
safeLabel->setText("Hello world!");
delete label;
// safeLabel is now 0, whereas label is a dangling pointer
//! [12]
