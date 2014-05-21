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
Q3SqlCursor cur( "staff" ); // Specify the table/view name
cur.select(); // We'll retrieve every record
while ( cur.next() ) {
    qDebug( cur.value( "id" ).toString() + ": " +
    cur.value( "surname" ).toString() + ' ' +
    cur.value( "salary" ).toString() );
}
//! [0]


//! [1]
Q3SqlCursor cur( "prices" );
cur.select( "id=202" );
if ( cur.next() ) {
    QSqlRecord *buffer = cur.primeUpdate();
    double price = buffer->value( "price" ).toDouble();
    double newprice = price * 1.05;
    buffer->setValue( "price", newprice );
    cur.update();
}
//! [1]


//! [2]
Q3SqlCursor cur("Employee"); // Use the Employee table or view
cur.select("deptno=10"); // select all records in department 10
while(cur.next()) {
    ... // process data
}
...
// select records in other departments, ordered by department number
cur.select("deptno>10", cur.index("deptno"));
...
//! [2]


//! [3]
Q3SqlCursor cur("Employee");
cur.select("deptno=10"); // select all records in department 10
while(cur.next()) {
    ... // process data
}
...
cur.select(); // re-selects all records in department 10
...
//! [3]


//! [4]
Q3SqlCursor cur("Employee");
QSqlIndex pk = cur.primaryIndex();
cur.setValue("id", 10);
cur.select(pk, pk); // generates "SELECT ... FROM Employee WHERE id=10 ORDER BY id"
...
//! [4]


//! [5]
Q3SqlCursor cur("Employee");
cur.setMode(Q3SqlCursor::Writable); // allow insert/update/delete
...
cur.setMode(Q3SqlCursor::Insert | Q3SqlCursor::Update); // allow inserts and updates only
...
cur.setMode(Q3SqlCursor::ReadOnly); // no inserts/updates/deletes allowed

//! [5]


//! [6]
Q3SqlCursor cur("prices");
QSqlRecord *buffer = cur.primeInsert();
buffer->setValue("id",    53981);
buffer->setValue("name",  "Thingy");
buffer->setValue("price", 105.75);
cur.insert();
//! [6]


//! [7]
Q3SqlCursor cur("prices");
cur.select("id=202");
if (cur.next()) {
    QSqlRecord *buffer = cur.primeUpdate();
    double price = buffer->value("price").toDouble();
    double newprice = price * 1.05;
    buffer->setValue("price", newprice);
    cur.update();
}
//! [7]


//! [8]
Q3SqlCursor cur("prices");
cur.select("id=999");
if (cur.next()) {
    cur.primeDelete();
    cur.del();
}
//! [8]
