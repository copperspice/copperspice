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
Q3PtrDict<char> fields; // void* keys, char* values

QLineEdit *le1 = new QLineEdit( this );
le1->setText( "Simpson" );
QLineEdit *le2 = new QLineEdit( this );
le2->setText( "Homer" );
QLineEdit *le3 = new QLineEdit( this );
le3->setText( "45" );

fields.insert( le1, "Surname" );
fields.insert( le2, "Forename" );
fields.insert( le3, "Age" );

Q3PtrDictIterator<char> it( fields );
for( ; it.current(); ++it )
    cout << it.current() << endl;
cout << endl;

if ( fields[le1] ) // Prints "Surname: Simpson"
    cout << fields[le1] << ": " << le1->text() << endl; 
if ( fields[le2] ) // Prints "Forename: Homer"
    cout << fields[le2] << ": " << le2->text() << endl; 

fields.remove( le1 ); // Removes le1 from the dictionary
cout << le1->text() << endl; // Prints "Simpson"
//! [0]


//! [1]
Q3PtrDict<ItemType> dict;
    ...
if ( dict.find( key ) )
    dict.remove( key );
dict.insert( key, item );
//! [1]


//! [2]
Q3PtrDict<char> fields;

QLineEdit *le1 = new QLineEdit( this );
le1->setText( "Simpson" );
QLineEdit *le2 = new QLineEdit( this );
le2->setText( "Homer" );
QLineEdit *le3 = new QLineEdit( this );
le3->setText( "45" );

fields.insert( le1, "Surname" );
fields.insert( le2, "Forename" );
fields.insert( le3, "Age" );

Q3PtrDictIterator<char> it( fields );
for( ; it.current(); ++it ) {
    QLineEdit *le = (QLineEdit)it.currentKey();
    cout << it.current() << ": " << le->text() << endl;
}
cout << endl;

// Output (random order):
//  Forename: Homer
//  Age: 45
//  Surname: Simpson
//! [2]


