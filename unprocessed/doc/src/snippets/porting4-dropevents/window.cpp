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

#include <QtGui>

#include "window.h"

MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
{
    QLabel *textLabel = new QLabel(tr("Data:"), this);
    dataLabel = new QLabel(this);
    dataLabel->setFixedSize(200, 200);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textLabel);
    layout->addWidget(dataLabel);

    setAcceptDrops(true);
    setWindowTitle(tr("Drop Events"));
}

//! [0]
void MyWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() || event->mimeData()->hasImage())
        event->acceptProposedAction();
}
//! [0]

//! [1]
void MyWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText())
        dataLabel->setText(event->mimeData()->text());
    else if (event->mimeData()->hasImage()) {
        QVariant imageData = event->mimeData()->imageData();
        dataLabel->setPixmap(qvariant_cast<QPixmap>(imageData));
    }
    event->acceptProposedAction();
}
//! [1]

//! [2]
void MyWidget::mousePressEvent(QMouseEvent *event)
{
//! [2]
    QString text = dataLabel->text();
    QPixmap iconPixmap(32, 32);
    iconPixmap.fill(qRgba(255, 0, 0, 127));
    QImage image(100, 100, QImage::Format_RGB32);
    image.fill(qRgb(0, 0, 255));

//! [3]
    if (event->button() == Qt::LeftButton) {

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(text);
        mimeData->setImageData(image);
        drag->setMimeData(mimeData);
        drag->setPixmap(iconPixmap);

        Qt::DropAction dropAction = drag->exec();
//! [3]
        // ...
//! [4]
        event->accept();
    }
//! [4]
    else if (event->button() == Qt::MidButton) {

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setImageData(image);
        drag->setMimeData(mimeData);
        drag->setPixmap(iconPixmap);

        Qt::DropAction dropAction = drag->exec();
        // ...
        event->accept();
    }
//! [5]
}
//! [5]
