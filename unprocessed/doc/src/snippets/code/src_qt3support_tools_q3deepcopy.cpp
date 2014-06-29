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
// all 5 strings share the same data
QString s1 = "abcd";
QString s2 = s1;
QString s3 = s2;
QString s4 = s3;
QString s5 = s2;
//! [0]


//! [1]
// s1, s2 and s5 share the same data, neither s3 nor s4 are shared
QString s1 = "abcd";
QString s2 = s1;
Q3DeepCopy<QString> s3 = s2;  // s3 is a deep copy of s2
QString s4 = s3;             // s4 is a deep copy of s3
QString s5 = s2;
//! [1]


//! [2]
// s1, s2 and s5 share the same data, s3 and s4 share the same data
QString s1 = "abcd";
QString s2 = s1;
QString s3 = Q3DeepCopy<QString>( s2 );  // s3 is a deep copy of s2
QString s4 = s3;                        // s4 is a shallow copy of s3
QString s5 = s2;
//! [2]


//! [3]
Q3DeepCopy<QString> global_string;  // global string data
QMutex global_mutex;               // mutex to protext global_string

...

void setGlobalString( const QString &str )
{
    global_mutex.lock();
    global_string = str;           // global_string is a deep copy of str
    global_mutex.unlock();
}

...

void MyThread::run()
{
    global_mutex.lock();
    QString str = global_string;          // str is a deep copy of global_string
    global_mutex.unlock();

    // process the string data
    ...

    // update global_string
    setGlobalString( str );
}
//! [3]
