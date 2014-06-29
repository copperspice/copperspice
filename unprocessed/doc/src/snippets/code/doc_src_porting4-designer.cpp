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

//! [0]
namespace Ui {

class HelloWorld
{
public:
    QVBoxLayout *vboxLayout;
    QPushButton *pushButton;

    void setupUi(QWidget *HelloWorld)
    {
        HelloWorld->setObjectName(QString::fromUtf8("HelloWorld"));

        vboxLayout = new QVBoxLayout(HelloWorld);
        vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));

        pushButton = new QPushButton(HelloWorld);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        vboxLayout->addWidget(pushButton);

        retranslateUi(HelloWorld);
    }
};

}
//! [0]


//! [1]
#include <QApplication>
#include <QWidget>

#include "ui_helloworld.h"   // defines Ui::HelloWorld

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget w;
    Ui::HelloWorld ui;
    ui.setupUi(&w);

    w.show();
    return app.exec();
}
//! [1]


//! [2]
#include <QApplication>
#include <QWidget>

#include "ui_helloworld.h"   // defines Ui::HelloWorld

class HelloWorldWidget : public QWidget, public Ui::HelloWorld
{
    Q_OBJECT

public:
    HelloWorldWidget(QWidget *parent = 0)
        : QWidget(parent)
    { setupUi(this); }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    HelloWorldWidget w;
    w.show();
    return app.exec();
}
//! [2]


//! [5]
class HelloWorldWidget : public QWidget, public Ui::HelloWorld
{
    Q_OBJECT

public:
    HelloWorldWidget(QWidget *parent = 0);

public slots:
    void mySlot();
};

HelloWorldWidget::HelloWorldWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    QObject::connect(pushButton, SIGNAL(clicked()),
                     this, SLOT(mySlot()));
}

void HelloWorldWidget::mySlot()
{
    ...
}
//! [5]


//! [6]
class HelloWorldWidget : public QWidget, public Ui::HelloWorld
{
    Q_OBJECT

public:
    HelloWorldWidget(QWidget *parent = 0);

public slots:
    void on_pushButton_clicked();
};

HelloWorldWidget::HelloWorldWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

void HelloWorldWidget::on_pushButton_clicked()
{
    ...
}
//! [6]


//! [9]
QFile file(":/icons/yes.png");
QIcon icon(":/icons/no.png");
QPixmap pixmap(":/icons/no.png");
//! [9]
