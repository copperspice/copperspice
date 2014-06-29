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
QPushButton p("&Exit", parent); // automatic shortcut Alt+E
Q3PopupMenu *fileMenu = new fileMenu(parent);
fileMenu->insertItem("Undo", parent, SLOT(undo()),
                     Qt::CTRL + Qt::Key_Z);
//! [0]


//! [1]
accelerator->insertItem(QKeySequence("M"));
//! [1]


//! [2]
Q3Accel *a = new Q3Accel(myWindow);
a->connectItem(a->insertItem(Qt::CTRL + Qt::Key_P),
               myWindow, SLOT(printDoc()));
//! [2]


//! [3]
Q3Accel *a = new Q3Accel(myWindow);	   // create accels for myWindow
a->insertItem(CTRL + Key_P, 200);	   // Ctrl+P, e.g. to print document
a->insertItem(ALT + Key_X, 201);	   // Alt+X, e.g. to quit
a->insertItem(UNICODE_ACCEL + 'q', 202);   // Unicode 'q', e.g. to quit
a->insertItem(Key_D);			   // gets a unique negative id < -1
a->insertItem(CTRL + SHIFT + Key_P);	   // gets a unique negative id < -1
//! [3]


//! [4]
a->connectItem(201, mainView, SLOT(quit()));
//! [4]


//! [5]
Q3PopupMenu *file = new Q3PopupMenu(this);
file->insertItem(p1, tr("&Open..."), this, SLOT(open()),
                  Q3Accel::stringToKey(tr("Ctrl+O", "File|Open")));
//! [5]
