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
#include <q3memarray.h>
#include <stdio.h>

Q3MemArray<int> fib( int num ) // returns fibonacci array
{
    Q_ASSERT( num > 2 );
    Q3MemArray<int> f( num ); // array of ints

    f[0] = f[1] = 1;
    for ( int i = 2; i < num; i++ )
	f[i] = f[i-1] + f[i-2];

    return f;
}

int main()
{
    Q3MemArray<int> a = fib( 6 ); // get first 6 fibonaccis
    for ( int i = 0; i < a.size(); i++ )
	qDebug( "%d: %d", i, a[i] );

    qDebug( "1 is found %d times", a.contains(1) );
    qDebug( "5 is found at index %d", a.find(5) );

    return 0;
}
//! [0]


//! [2]
// MyStruct may be padded to 4 or 8 bytes
struct MyStruct
{
    short i; // 2 bytes
    char c;  // 1 byte
};

Q3MemArray<MyStruct> a(1);
a[0].i = 5;
a[0].c = 't';

MyStruct x;
x.i = '5';
x.c = 't';
int i = a.find( x ); // may return -1 if the pad bytes differ
//! [2]


//! [3]
static char bindata[] = { 231, 1, 44, ... };
QByteArray	a;
a.setRawData( bindata, sizeof(bindata) );	// a points to bindata
QDataStream s( a, IO_ReadOnly );		// open on a's data
s >> <something>;				// read raw bindata
a.resetRawData( bindata, sizeof(bindata) ); // finished
//! [3]


//! [4]
static char bindata[] = { 231, 1, 44, ... };
QByteArray	a, b;
a.setRawData( bindata, sizeof(bindata) );	// a points to bindata
a.resize( 8 );				// will crash
b = a;					// will crash
a[2] = 123;					// might crash
// forget to resetRawData: will crash
//! [4]
