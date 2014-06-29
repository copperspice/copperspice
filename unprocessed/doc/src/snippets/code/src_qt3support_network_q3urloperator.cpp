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
Q3UrlOperator *op = new Q3UrlOperator();
op->copy( QString("ftp://ftp.qt.nokia.com/qt/source/qt-2.1.0.tar.gz"),
	 "file:///tmp" );
//! [0]


//! [1]
Q3UrlOperator op( "http://www.whatever.org/cgi-bin/search.pl?cmd=Hello" );
op.get();
//! [1]


//! [2]
Q3UrlOperator op( "ftp://ftp.whatever.org/pub" );
// do some other stuff like op.listChildren() or op.mkdir( "new_dir" )
op.get( "a_file.txt" );
//! [2]


//! [3]
Q3UrlOperator op( "http://www.whatever.org/cgi-bin" );
op.get( "search.pl?cmd=Hello" ); // WRONG!
//! [3]


//! [4]
Q3UrlOperator op( "ftp://ftp.whatever.com/home/me/filename.dat" );
op.put( data );
//! [4]


//! [5]
Q3UrlOperator op( "ftp://ftp.whatever.com/home/me" );
// do some other stuff like op.listChildren() or op.mkdir( "new_dir" )
op.put( data, "filename.dat" );
//! [5]
