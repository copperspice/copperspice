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


void wrapInFunction()
{

//! [0]
for (int row = 0; row < table->numRows(); row++) {
    for (int col = 0; col < table->numCols(); col++) {
        table->setItem(row, col,
            new Q3TableItem(table, Q3TableItem::WhenCurrent, QString::number(row * col)));
    }
}
//! [0]


//! [1]
QWidget* MyTableItem::createEditor() const
{
    QHBox* hbox = new QHBox(table()->viewport());
    hbox->setFocusProxy(new QLineEdit(hbox));
    new QLineEdit(hbox);
    return hbox;
}
//! [1]


//! [2]
p->setClipRect(table()->cellRect(row, col), QPainter::ClipPainter);
//... your drawing code
p->setClipping(false);
//! [2]


//! [3]
Q3Table *table = new Q3Table(100, 250, this);
table->setPixmap(3, 2, pix);
table->setText(3, 2, "A pixmap");
//! [3]


//! [4]
p->setClipRect(cellRect(row, col), QPainter::CoordPainter);
//... your drawing code
p->setClipping(false);
//! [4]


//! [5]
Q3TableItem *i = item(row, col);
if (initFromCell || (i && !i->isReplaceable()))
    // If we had a Q3TableItem ask the base class to create the editor
    return Q3Table::createEditor(row, col, initFromCell);
else
    return ...(create your own editor)
//! [5]

}

