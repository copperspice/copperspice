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
Q3ScrollView* sv = new Q3ScrollView(...);
QWidget *widget = new QWidget(sv->viewport());
QVBoxLayout *layout = new QVBoxLayout(widget);
addChild(widget);
//! [0]


//! [1]
QLabel* child1 = new QLabel("CHILD", widget);
QLabel* child2 = new QLabel("CHILD", widget);
QLabel* child3 = new QLabel("CHILD", widget);
layout->addWidget(child1);
layout->addWidget(child2);
layout->addWidget(child3);
...
//! [1]


//! [2]
Q3ScrollView* sv = new Q3ScrollView(...);
QLabel* child1 = new QLabel("CHILD", sv->viewport());
sv->addChild(child1);
QLabel* child2 = new QLabel("CHILD", sv->viewport());
sv->addChild(child2);
QLabel* child3 = new QLabel("CHILD", sv->viewport());
sv->addChild(child3);
//! [2]


//! [3]
Q3ScrollView* sv = new Q3ScrollView(...);
sv->enableClipper(true);
QLabel* child1 = new QLabel("CHILD", sv->viewport());
sv->addChild(child1);
QLabel* child2 = new QLabel("CHILD", sv->viewport());
sv->addChild(child2);
QLabel* child3 = new QLabel("CHILD", sv->viewport());
sv->addChild(child3);
//! [3]


//! [4]
{
    // Fill a 40000 by 50000 rectangle at (100000,150000)

    // Calculate the coordinates...
    int x1 = 100000, y1 = 150000;
    int x2 = x1+40000-1, y2 = y1+50000-1;

    // Clip the coordinates so X/Windows will not have problems...
    if (x1 < clipx) x1=clipx;
    if (y1 < clipy) y1=clipy;
    if (x2 > clipx+clipw-1) x2=clipx+clipw-1;
    if (y2 > clipy+cliph-1) y2=clipy+cliph-1;

    // Paint using the small coordinates...
    if (x2 >= x1 && y2 >= y1)
        p->fillRect(x1, y1, x2-x1+1, y2-y1+1, red);
}
//! [4]
