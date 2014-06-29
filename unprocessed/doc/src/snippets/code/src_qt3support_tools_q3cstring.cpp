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
Q3CString str("helloworld", 6); // assigns "hello" to str
//! [0]


//! [1]
Q3CString a;                // a.data() == 0,  a.size() == 0, a.length() == 0
Q3CString b == "";        // b.data() == "", b.size() == 1, b.length() == 0
a.isNull();                // true  because a.data() == 0
a.isEmpty();        // true  because a.length() == 0
b.isNull();                // false because b.data() == ""
b.isEmpty();        // true  because b.length() == 0
//! [1]


//! [2]
Q3CString s = "truncate this string";
s.truncate(5);                      // s == "trunc"
//! [2]


//! [3]
Q3CString s;
s.sprintf("%d - %s", 1, "first");                // result < 256 chars

Q3CString big(25000);                        // very long string
big.sprintf("%d - %s", 2, longString);        // result < 25000 chars
//! [3]


//! [4]
Q3CString s("apple");
Q3CString t = s.leftJustify(8, '.');  // t == "apple..."
//! [4]


//! [5]
Q3CString s("pie");
Q3CString t = s.rightJustify(8, '.');  // t == ".....pie"
//! [5]
