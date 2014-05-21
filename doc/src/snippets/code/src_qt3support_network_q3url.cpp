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
Q3Url url( "http://qt.nokia.com" );
// or
Q3Url url( "file:///home/myself/Mail", "Inbox" );
//! [0]


//! [1]
Q3Url url( "http://qt.nokia.com" );
QString s = url;
// or
QString s( "http://qt.nokia.com" );
Q3Url url( s );
//! [1]


//! [2]
Q3Url url( "ftp://ftp.qt.nokia.com/qt/source", "qt-2.1.0.tar.gz" );
//! [2]


//! [3]
Q3Url url( "ftp://ftp.qt.nokia.com/qt/source", "/usr/local" );
//! [3]


//! [4]
Q3Url url( "ftp://ftp.qt.nokia.com/qt/source", "file:///usr/local" );
//! [4]


//! [5]
QString url = http://qt.nokia.com
Q3Url::encode( url );
// url is now "http%3A//qt%20nokia%20com"
//! [5]


//! [6]
QString url = "http%3A//qt%20nokia%20com"
Q3Url::decode( url );
// url is now "http://qt.nokia.com"
//! [6]
