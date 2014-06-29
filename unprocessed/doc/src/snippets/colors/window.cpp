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

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    QFont font;
    font.setPixelSize(12);
    setFont(font);
}

void Window::closeEvent(QCloseEvent *event)
{
    QPixmap pixmap(size());
    render(&pixmap);
    pixmap.save("qt-colors.png");

    event->accept();
}

void Window::paintEvent(QPaintEvent *)
{
    QPainter painter;
    painter.begin(this);

    int h = 216 / 5;
    QRect r = QRect(0, 0, 160, h);
    painter.fillRect(r, Qt::white);
    painter.setPen(Qt::black);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("white"));
    r = QRect(0, h, 160, h);
    painter.fillRect(r, Qt::red);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("red"));
    r = QRect(0, h*2, 160, h);
    painter.fillRect(r, Qt::green);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("green"));
    r = QRect(0, h*3, 160, h);
    painter.fillRect(r, Qt::blue);
    painter.setPen(Qt::white);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("blue"));

    r = QRect(160, 0, 160, h);
    painter.fillRect(r, Qt::black);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("black"));
    r = QRect(160, h, 160, h);
    painter.fillRect(r, Qt::darkRed);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("darkRed"));
    r = QRect(160, h*2, 160, h);
    painter.fillRect(r, Qt::darkGreen);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("darkGreen"));
    r = QRect(160, h*3, 160, h);
    painter.fillRect(r, Qt::darkBlue);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("darkBlue"));

    r = QRect(320, 0, 160, h);
    painter.fillRect(r, Qt::cyan);
    painter.setPen(Qt::black);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("cyan"));
    r = QRect(320, h, 160, h);
    painter.fillRect(r, Qt::magenta);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("magenta"));
    r = QRect(320, h*2, 160, h);
    painter.fillRect(r, Qt::yellow);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("yellow"));
    r = QRect(320, h*3, 160, h);
    painter.fillRect(r, Qt::gray);
    painter.setPen(Qt::white);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("gray"));

    r = QRect(480, 0, 160, h);
    painter.fillRect(r, Qt::darkCyan);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("darkCyan"));
    r = QRect(480, h, 160, h);
    painter.fillRect(r, Qt::darkMagenta);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("darkMagenta"));
    r = QRect(480, h*2, 160, h);
    painter.fillRect(r, Qt::darkYellow);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("darkYellow"));
    r = QRect(480, h*3, 160, h);
    painter.fillRect(r, Qt::darkGray);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("darkGray"));

    r = QRect(0, h*4, 640, h);
    painter.fillRect(r, Qt::lightGray);
    painter.setPen(Qt::black);
    painter.drawText(r, Qt::AlignCenter, QLatin1String("lightGray"));

    painter.end();
}

