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
QLinearGradient gradient(0, 0, 100, 100);
gradient.setColorAt(0, Qt::red);
gradient.setColorAt(0.5, Qt::green);
gradient.setColorAt(1, Qt::blue);
painter.setBrush(gradient);
painter.drawRect(0, 0, 100, 100);
//! [0]


//! [1]
QRadialGradient gradient(50, 50, 50, 30, 30);
gradient.setColorAt(0.2, Qt::white);
gradient.setColorAt(0.8, Qt::green);
gradient.setColorAt(1, Qt::black);
painter.setBrush(gradient);
painter.drawEllipse(0, 0, 100, 100);
//! [1]


//! [2]
QConicalGradient gradient(60, 40, 0);
gradient.setColorAt(0, Qt::black);
gradient.setColorAt(0.4, Qt::green);
gradient.setColorAt(0.6, Qt::white);
gradient.setColorAt(1, Qt::black);
painter.setBrush(gradient);
painter.drawEllipse(0, 0, 100, 100);
//! [2]


//! [3]
// Specfiy semi-transparent red
painter.setBrush(QColor(255, 0, 0, 127));
painter.drawRect(0, 0, width()/2, height());

// Specify semi-transparent blue
painter.setBrush(QColor(0, 0, 255, 127));
painter.drawRect(0, 0, width(), height()/2);
//! [3]


//! [4]
// One line without anti-aliasing
painter.drawLine(0, 0, width()/2, height());

// One line with anti-aliasing
painter.setRenderHint(QPainter::Antialiasing);
painter.drawLine(width()/2, 0, width(), height());
//! [4]


//! [5]
QPainterPath path;
path.addRect(20, 20, 60, 60);
path.moveTo(0, 0);
path.cubicTo(99, 0, 50, 50, 99, 99);
path.cubicTo(0, 99, 50, 50, 0, 0);
painter.drawPath(path);
//! [5]


//! [6]
QPixmap buffer(size());
QPainter painter(&buffer);

// Paint code here

painter.end();
bitBlt(this, 0, 0, &buffer);
//! [6]


//! [7]
QPainter painter(this);

// Paint code here

painter.end();
//! [7]


//! [8]
unbufferedWidget->setAttribute(Qt::WA_PaintOnScreen);
//! [8]


//! [9]
QLinearGradient gradient(0, 0, 100, 100);
gradient.setColorAt(0, Qt::blue);
gradient.setColorAt(1, Qt::red);
painter.setPen(QPen(gradient, 0));
for (int y=fontSize; y<100; y+=fontSize)
    drawText(0, y, text);
//! [9]


//! [10]
QImage image(100, 100, 32);
QPainter painter(&image);

// painter commands.

painter.end();
//! [10]
