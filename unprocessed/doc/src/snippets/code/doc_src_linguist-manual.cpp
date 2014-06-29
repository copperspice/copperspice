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
label->setText(tr("F\374r \310lise"));
//! [3]


void wrapInFunction()
{
//! [6]
button = new QPushButton("&Quit", this);
//! [6]


//! [7]
button = new QPushButton(tr("&Quit"), this);
//! [7]


//! [8]
QPushButton::tr("&Quit")
//! [8]


//! [9]
QObject::tr("&Quit")
//! [9]


//! [10]
rbc = new QRadioButton(tr("Enabled", "Color frame"), this);
//! [10]


//! [11]
rbh = new QRadioButton(tr("Enabled", "Hue frame"), this);
//! [11]
}


//! [12]
/*
    TRANSLATOR FindDialog

    Choose Edit|Find from the menu bar or press Ctrl+F to pop up the
    Find dialog.

    ...
*/
//! [12]

//! [13]
/*
    TRANSLATOR MyNamespace::MyClass

    Necessary for lupdate.

    ...
*/
//! [13]

//! [14]
void some_global_function(LoginWidget *logwid)
{
    QLabel *label = new QLabel(
            LoginWidget::tr("Password:"), logwid);
}

void same_global_function(LoginWidget *logwid)
{
    QLabel *label = new QLabel(
            qApp->translate("LoginWidget", "Password:"),
            logwid);
}
//! [14]


//! [15]
QString FriendlyConversation::greeting(int greet_type)
{
    static const char* greeting_strings[] = {
        QT_TR_NOOP("Hello"),
        QT_TR_NOOP("Goodbye")
    };
    return tr(greeting_strings[greet_type]);
}
//! [15]


//! [16]
static const char* greeting_strings[] = {
    QT_TRANSLATE_NOOP("FriendlyConversation", "Hello"),
    QT_TRANSLATE_NOOP("FriendlyConversation", "Goodbye")
};

QString FriendlyConversation::greeting(int greet_type)
{
    return tr(greeting_strings[greet_type]);
}

QString global_greeting(int greet_type)
{
    return qApp->translate("FriendlyConversation",
                            greeting_strings[greet_type]);
}
//! [16]

void wrapInFunction()
{

//! [17]
QString tr(const char *text, const char *comment, int n);
//! [17]

//! [18]
tr("%n item(s) replaced", "", count);
//! [18]

}
